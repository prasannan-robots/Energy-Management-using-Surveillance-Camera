/**
 * Configuration Manager Implementation
 * 
 * Handles SPIFFS storage, JSON parsing, and WiFi setup.
 */

#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// File paths
#define CONFIG_FILE "/config.json"
#define ZONES_FILE "/zones.json"

/**
 * Load configuration from SPIFFS
 */
bool loadConfigFromSPIFFS(Config* config) {
  if (!LittleFS.exists(CONFIG_FILE)) {
    Serial.println("Config file not found, using defaults");
    return false;
  }
  
  File file = LittleFS.open(CONFIG_FILE, "r");
  if (!file) {
    Serial.println("ERROR: Failed to open config file");
    return false;
  }
  
  // Parse JSON
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.printf("ERROR: Failed to parse config JSON: %s\n", error.c_str());
    return false;
  }
  
  // Load WiFi settings
  strlcpy(config->wifiSSID, doc["wifi"]["ssid"] | "ESP32_SmartSwitch", MAX_SSID_LENGTH);
  strlcpy(config->wifiPassword, doc["wifi"]["password"] | "", MAX_PASSWORD_LENGTH);
  
  // Load MJPEG stream settings
  strlcpy(config->cctvIP, doc["cctv"]["ip"] | "192.168.4.100", MAX_IP_LENGTH);
  config->cctvPort = doc["cctv"]["port"] | 81;
  strlcpy(config->streamPath, doc["cctv"]["path"] | "/stream", 64);
  
  // Load detection settings
  config->detectionThreshold = doc["detection"]["threshold"] | 0.5;
  config->globalTimeout = doc["detection"]["globalTimeout"] | 5;
  
  // Load system settings
  config->relayActiveHigh = doc["system"]["relayActiveHigh"] | true;
  config->enableWatchdog = doc["system"]["enableWatchdog"] | true;
  config->watchdogTimeout = doc["system"]["watchdogTimeout"] | 60;
  
  Serial.println("✓ Configuration loaded from SPIFFS");
  
  // Load zones
  return loadZonesFromJSON(config, ZONES_FILE);
}

/**
 * Save configuration to SPIFFS
 */
bool saveConfigToSPIFFS(const Config* config) {
  // Create JSON document
  StaticJsonDocument<1024> doc;
  
  // WiFi settings
  doc["wifi"]["ssid"] = config->wifiSSID;
  doc["wifi"]["password"] = config->wifiPassword;
  
  // MJPEG stream settings
  doc["cctv"]["ip"] = config->cctvIP;
  doc["cctv"]["port"] = config->cctvPort;
  doc["cctv"]["path"] = config->streamPath;
  
  // Detection settings
  doc["detection"]["threshold"] = config->detectionThreshold;
  doc["detection"]["globalTimeout"] = config->globalTimeout;
  
  // System settings
  doc["system"]["relayActiveHigh"] = config->relayActiveHigh;
  doc["system"]["enableWatchdog"] = config->enableWatchdog;
  doc["system"]["watchdogTimeout"] = config->watchdogTimeout;
  
  // Write to file
  File file = LittleFS.open(CONFIG_FILE, "w");
  if (!file) {
    Serial.println("ERROR: Failed to open config file for writing");
    return false;
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("ERROR: Failed to write config JSON");
    file.close();
    return false;
  }
  
  file.close();
  Serial.println("✓ Configuration saved to SPIFFS");
  
  // Save zones
  return saveZonesToJSON(config, ZONES_FILE);
}

/**
 * Load zones from JSON file
 */
bool loadZonesFromJSON(Config* config, const char* jsonPath) {
  if (!LittleFS.exists(jsonPath)) {
    Serial.println("Zones file not found");
    return false;
  }
  
  File file = LittleFS.open(jsonPath, "r");
  if (!file) {
    Serial.println("ERROR: Failed to open zones file");
    return false;
  }
  
  // Parse JSON
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.printf("ERROR: Failed to parse zones JSON: %s\n", error.c_str());
    return false;
  }
  
  // Clear existing zones
  config->zones.clear();
  
  // Load zones array
  JsonArray zonesArray = doc["zones"].as<JsonArray>();
  for (JsonObject zoneObj : zonesArray) {
    Zone zone;
    zone.id = zoneObj["id"] | 0;
    strlcpy(zone.name, zoneObj["name"] | "Unnamed", 32);
    zone.x = zoneObj["x"] | 0;
    zone.y = zoneObj["y"] | 0;
    zone.width = zoneObj["width"] | 100;
    zone.height = zoneObj["height"] | 100;
    zone.timeout = zoneObj["timeout"] | config->globalTimeout;
    zone.active = false;
    zone.lastDetectionTime = 0;
    
    // Load relay pins
    JsonArray relaysArray = zoneObj["relayPins"].as<JsonArray>();
    zone.numRelays = 0;
    for (JsonVariant relay : relaysArray) {
      if (zone.numRelays < MAX_RELAYS_PER_ZONE) {
        zone.relayPins[zone.numRelays++] = relay.as<int>();
      }
    }
    
    config->zones.push_back(zone);
  }
  
  Serial.printf("✓ Loaded %d zones from SPIFFS\n", config->zones.size());
  return true;
}

/**
 * Save zones to JSON file
 */
bool saveZonesToJSON(const Config* config, const char* jsonPath) {
  // Create JSON document
  DynamicJsonDocument doc(4096);
  
  // Create zones array
  JsonArray zonesArray = doc.createNestedArray("zones");
  
  for (const Zone& zone : config->zones) {
    JsonObject zoneObj = zonesArray.createNestedObject();
    zoneObj["id"] = zone.id;
    zoneObj["name"] = zone.name;
    zoneObj["x"] = zone.x;
    zoneObj["y"] = zone.y;
    zoneObj["width"] = zone.width;
    zoneObj["height"] = zone.height;
    zoneObj["timeout"] = zone.timeout;
    
    JsonArray relaysArray = zoneObj.createNestedArray("relayPins");
    for (int i = 0; i < zone.numRelays; i++) {
      relaysArray.add(zone.relayPins[i]);
    }
  }
  
  // Write to file
  File file = LittleFS.open(jsonPath, "w");
  if (!file) {
    Serial.println("ERROR: Failed to open zones file for writing");
    return false;
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("ERROR: Failed to write zones JSON");
    file.close();
    return false;
  }
  
  file.close();
  Serial.printf("✓ Saved %d zones to SPIFFS\n", config->zones.size());
  return true;
}

/**
 * Set default configuration
 */
void setDefaultConfig(Config* config) {
  // WiFi credentials - hardcoded for simplicity
  strlcpy(config->wifiSSID, "EnergyManagement", MAX_SSID_LENGTH);
  strlcpy(config->wifiPassword, "123456789@E", MAX_PASSWORD_LENGTH);
  
  // MJPEG stream defaults (ESP32-CAM or IP camera)
  strlcpy(config->cctvIP, "192.168.137.206", MAX_IP_LENGTH);
  config->cctvPort = 8080;
  strlcpy(config->streamPath, "", 64);  // Empty for most IP cameras on port 8080
  
  // Detection defaults
  config->detectionThreshold = 0.5;
  config->globalTimeout = 5;
  
  // System defaults
  config->relayActiveHigh = true;  // GPIOs will be HIGH when person detected
  config->enableWatchdog = true;
  config->watchdogTimeout = 60;
  
  // Clear zones
  config->zones.clear();
  
  // Add two example zones
  Zone zone1;
  zone1.id = 1;
  strlcpy(zone1.name, "Zone 1", 32);
  zone1.x = 10;
  zone1.y = 10;
  zone1.width = 140;
  zone1.height = 220;
  zone1.relayPins[0] = 12;
  zone1.numRelays = 1;
  zone1.timeout = 5;
  zone1.active = false;
  zone1.lastDetectionTime = 0;
  
  Zone zone2;
  zone2.id = 2;
  strlcpy(zone2.name, "Zone 2", 32);
  zone2.x = 170;
  zone2.y = 10;
  zone2.width = 140;
  zone2.height = 220;
  zone2.relayPins[0] = 13;
  zone2.numRelays = 1;
  zone2.timeout = 5;
  zone2.active = false;
  zone2.lastDetectionTime = 0;
  
  config->zones.push_back(zone1);
  config->zones.push_back(zone2);
  
  Serial.println("✓ Default configuration set");
}

/**
 * Print configuration for debugging
 */
void printConfig(const Config* config) {
  Serial.println("\n=== Configuration ===");
  Serial.printf("WiFi SSID: %s\n", config->wifiSSID);
  Serial.printf("CCTV IP: %s:%d%s\n", config->cctvIP, config->cctvPort, config->streamPath);
  Serial.printf("Detection Threshold: %.2f\n", config->detectionThreshold);
  Serial.printf("Global Timeout: %d seconds\n", config->globalTimeout);
  Serial.printf("Relay Active High: %s\n", config->relayActiveHigh ? "Yes" : "No");
  Serial.printf("Watchdog Enabled: %s\n", config->enableWatchdog ? "Yes" : "No");
  Serial.printf("Zones: %d\n", config->zones.size());
  
  for (size_t i = 0; i < config->zones.size(); i++) {
    const Zone& zone = config->zones[i];
    Serial.printf("  Zone %d: %s [%d,%d,%dx%d] Relays:", 
                 zone.id, zone.name, zone.x, zone.y, zone.width, zone.height);
    for (int j = 0; j < zone.numRelays; j++) {
      Serial.printf(" %d", zone.relayPins[j]);
    }
    Serial.printf(" Timeout: %ds\n", zone.timeout);
  }
  Serial.println("====================\n");
}

/**
 * Setup WiFi connection
 */
void setupWiFi(Config* config) {
  Serial.println("\n=== WiFi Setup ===");
  
  // Connect to WiFi
  Serial.printf("Connecting to WiFi: %s\n", config->wifiSSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(config->wifiSSID, config->wifiPassword);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {  // Reduced to 20 seconds
    delay(500);  // Shorter delay
    yield();  // Feed watchdog
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✓ WiFi connected!");
    Serial.printf("\n███████████████████████████\n");
    Serial.printf("█ Web Interface URL:      █\n");
    Serial.printf("█ http://%-18s █\n", WiFi.localIP().toString().c_str());
    Serial.printf("███████████████████████████\n\n");
    Serial.println("1. Open browser and navigate to above URL");
    Serial.println("2. Go to Settings tab to configure camera IP");
    Serial.println("3. Go to Zones tab to draw detection zones");
    Serial.printf("\nSignal Strength: %d dBm\n", WiFi.RSSI());
  } else {
    Serial.println("✗ WiFi connection FAILED!");
    Serial.println("\nCheck:");
    Serial.println("  - WiFi SSID and password are correct");
    Serial.println("  - Router is powered on and in range");
    Serial.println("  - ESP32 is within WiFi coverage");
    Serial.println("\nRebooting in 10 seconds...");
    delay(10000);
    ESP.restart();
  }
  
  Serial.println("==================\n");
}
