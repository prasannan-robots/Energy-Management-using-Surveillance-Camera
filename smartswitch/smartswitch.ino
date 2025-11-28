/**
 * ESP32-CAM Smart Zone Switch
 * 
 * Consumes MJPEG stream from CCTV node, performs local person detection,
 * and activates relays based on zone occupancy.
 * 
 * Hardware:
 * - ESP32-CAM board
 * - GPIO 12, 13, 14: Relay control pins
 * - 5V dual-channel relay module
 * 
 * Features:
 * - MJPEG stream consumer from remote CCTV camera
 * - TensorFlow Lite person detection
 * - Web-based zone drawing interface
 * - Zone-based relay activation with timeouts
 * - Configuration persistence via SPIFFS
 */

#include <WiFi.h>
#include <LittleFS.h>
#include "config.h"
#include "mjpeg_stream.h"
#include "motion_detector.h"
#include "zone_manager.h"
#include "web_server.h"
#include "utils.h"

#define SPIFFS LittleFS  // Use LittleFS instead of SPIFFS

// Global objects
Config globalConfig;
MJPEGStream mjpegStream;
MotionDetector motionDetector;
ZoneManager zoneManager;
WebServerManager webServer;

// Frame buffer
uint8_t* frameBuffer = nullptr;
size_t frameSize = 0;

// Watchdog timer variables
unsigned long lastFrameTime = 0;
const unsigned long WATCHDOG_TIMEOUT = 60000; // 60 seconds

// Performance tracking
unsigned long lastStatsTime = 0;
int frameCount = 0;
float avgFPS = 0.0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32-CAM Smart Zone Switch ===");
  Serial.println("Initializing...\n");
  
  // Initialize relay GPIOs (default to LOW/OFF)
  initializeRelays();
  
  // Initialize LittleFS (replaces SPIFFS)
  if (!LittleFS.begin(true)) {
    Serial.println("ERROR: LittleFS mount failed!");
    Serial.println("Continuing with default configuration...");
  } else {
    Serial.println("✓ LittleFS initialized");
  }
  
  // Load configuration from SPIFFS
  if (loadConfigFromSPIFFS(&globalConfig)) {
    Serial.println("✓ Configuration loaded from SPIFFS");
  } else {
    Serial.println("⚠ Using default configuration");
    setDefaultConfig(&globalConfig);
  }
  
  // Initialize WiFi
  setupWiFi(&globalConfig);
  
  // Set hostname
  WiFi.setHostname("smartswitch");
  Serial.println("✓ Hostname set to: smartswitch");
  Serial.println("  Access via: http://smartswitch.local/ (if mDNS supported)");
  
  // Initialize zone manager
  zoneManager.begin(&globalConfig);
  Serial.println("✓ Zone manager initialized");
  
  // Initialize simple motion detector (lightweight, no AI)
  if (motionDetector.begin(640, 480)) {
    Serial.println("✓ Motion detector initialized");
    Serial.println("  Motion in zones will trigger relays");
    motionDetector.setSensitivity(0.15);  // Adjust 0.1-0.5 (lower = more sensitive)
  } else {
    Serial.println("⚠ Motion detector failed to initialize");
  }
  
  // Don't auto-connect to camera - let user test/start from web UI
  Serial.println("\n⚠ Camera not connected - configure and test via web interface");
  Serial.println("  Current camera setting: http://" + String(globalConfig.cctvIP) + ":" + String(globalConfig.cctvPort) + globalConfig.streamPath);
  Serial.println("  Click 'Test Connection' in Settings tab to verify camera access");
  
  // Start web server (pass NULL for TFLite detector - we're using motion)
  yield(); // Prevent watchdog
  delay(100);
  webServer.begin(&globalConfig, &zoneManager, nullptr, &mjpegStream);
  Serial.println("✓ Web server started");
  yield();
  
  // Print system info
  printSystemInfo();
  
  Serial.println("\n╔══════════════════════════════════════════╗");
  Serial.println("║     🎉 SYSTEM READY - WEB UI ACTIVE     ║");
  Serial.println("╚══════════════════════════════════════════╝");
  Serial.println("\n📱 STEP 1: Open your web browser");
  Serial.printf("   → http://%s/\n", WiFi.localIP().toString().c_str());
  Serial.println("   → http://smartswitch.local/ (if supported)\n");
  Serial.println("⚙️  STEP 2: Configure camera (Settings tab)");
  Serial.println("   → Enter your camera's IP address and port");
  Serial.printf("   → Current: http://%s:%d%s\n\n", 
                globalConfig.cctvIP, globalConfig.cctvPort, globalConfig.streamPath);
  Serial.println("🎯 STEP 3: Draw zones (Zones tab)");
  Serial.println("   → Click 'Draw Zone' button");
  Serial.println("   → Draw rectangles on video feed");
  Serial.println("   → Assign relay pins to each zone\n");
  
  lastFrameTime = millis();
  lastStatsTime = millis();
}

void loop() {
  // Handle web server requests
  webServer.handleClient();
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠ WiFi disconnected! Attempting reconnection...");
    WiFi.reconnect();
    delay(5000);
    return;
  }
  
  // Track camera connection state
  static bool wasConnected = false;
  bool isConnected = mjpegStream.isConnected();
  
  // Only process frames if camera is connected
  if (!isConnected) {
    // Camera not connected - just wait
    wasConnected = false;
    delay(500);
    return;
  }
  
  // Update lastFrameTime when camera first connects to prevent immediate timeout
  if (isConnected && !wasConnected) {
    Serial.println("Camera connection established - resetting watchdog timer");
    lastFrameTime = millis();
    wasConnected = true;
  }
  
  // Watchdog timer: disable all relays if no frame received for too long
  if (millis() - lastFrameTime > WATCHDOG_TIMEOUT) {
    Serial.println("⚠ WATCHDOG TIMEOUT - No frames for 60s, disabling all relays!");
    zoneManager.disableAllRelays();
    mjpegStream.disconnect(); // Disconnect stuck stream
    delay(1000); // Prevent log spam
    return;
  }
  
  // Fetch next MJPEG frame
  if (mjpegStream.fetchFrame(&frameBuffer, &frameSize)) {
    lastFrameTime = millis();
    frameCount++;
    
    // Use motion detection on raw JPEG frames (no decoding needed)
    int width = 640;  // Assume default resolution
    int height = 480;
    
    // Detect motion in current frame
    std::vector<Detection> detections;
    std::vector<MotionBlob> motionBlobs = motionDetector.detectMotion(frameBuffer, frameSize);
    
    // Convert motion blobs to detections for zone manager
    for (const MotionBlob& blob : motionBlobs) {
      Detection det;
      det.x = (float)blob.x / width;
      det.y = (float)blob.y / height;
      det.width = (float)blob.width / width;
      det.height = (float)blob.height / height;
      det.confidence = blob.intensity;
      detections.push_back(det);
    }
    
    if (true) {  // Always process frame
      
      // Update relay states based on detections and zones (if auto control enabled)
      if (globalConfig.autoRelayControl) {
        zoneManager.update(detections, width, height);
      } else {
        // Just log detections without controlling relays
        if (detections.size() > 0) {
          Serial.printf("👁 Motion detected but auto-relay control DISABLED\n");
        }
      }
      
      // Send frame to web UI clients (via WebSocket)
      webServer.broadcastFrame(frameBuffer, frameSize, detections);
      
    } else {
      Serial.println("⚠ Failed to decode JPEG frame");
    }
    
    // Calculate and display FPS stats every 10 seconds
    if (millis() - lastStatsTime > 10000) {
      avgFPS = frameCount / ((millis() - lastStatsTime) / 1000.0);
      Serial.printf("📊 Performance: %.1f FPS, Free heap: %d bytes, PSRAM: %d bytes\n",
                   avgFPS, ESP.getFreeHeap(), ESP.getFreePsram());
      frameCount = 0;
      lastStatsTime = millis();
    }
    
  } else {
    // Failed to fetch frame - stream might be down
    static unsigned long lastErrorLog = 0;
    if (millis() - lastErrorLog > 5000) {
      Serial.println("⚠ Failed to fetch MJPEG frame");
      Serial.println("  Check camera connection in web interface");
      lastErrorLog = millis();
      
      // Disconnect to prevent watchdog
      mjpegStream.disconnect();
    }
    delay(100);
  }
  
  // Small delay to prevent tight loop
  delay(50); // Target ~20 FPS max (combined with processing time)
}

/**
 * Initialize relay GPIO pins to safe OFF state
 */
void initializeRelays() {
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
  
  Serial.println("✓ Relay GPIOs initialized (12, 13, 14) - all OFF");
}

/**
 * Print system information
 */
void printSystemInfo() {
  Serial.println("\n--- System Information ---");
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
  Serial.printf("WiFi MAC: %s\n", WiFi.macAddress().c_str());
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  Serial.println("-------------------------\n");
}
