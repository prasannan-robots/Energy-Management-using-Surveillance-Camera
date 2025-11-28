/**
 * Configuration Manager Header
 * 
 * Handles loading/saving configuration from SPIFFS,
 * zone definitions, CCTV settings, and system parameters.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <vector>

// Maximum limits
#define MAX_ZONES 10
#define MAX_RELAYS_PER_ZONE 4
#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 64
#define MAX_IP_LENGTH 16

/**
 * Zone definition structure
 */
struct Zone {
  int id;
  char name[32];
  int x;          // Top-left X coordinate (0-320 for QVGA)
  int y;          // Top-left Y coordinate (0-240 for QVGA)
  int width;      // Zone width
  int height;     // Zone height
  int relayPins[MAX_RELAYS_PER_ZONE]; // GPIO pins to activate
  int numRelays;  // Number of relays assigned
  int timeout;    // Timeout in seconds
  bool active;    // Current activation state
  unsigned long lastDetectionTime; // For timeout tracking
};

/**
 * Global configuration structure
 */
struct Config {
  // WiFi settings
  char wifiSSID[MAX_SSID_LENGTH];
  char wifiPassword[MAX_PASSWORD_LENGTH];
  
  // MJPEG stream settings
  char cctvIP[MAX_IP_LENGTH];
  int cctvPort;
  char streamPath[64];
  
  // Detection settings
  float detectionThreshold;  // Confidence threshold (0.0-1.0)
  int globalTimeout;         // Default timeout in seconds
  
  // Zones
  std::vector<Zone> zones;
  
  // System settings
  bool relayActiveHigh;      // true = HIGH activates relay, false = LOW activates
  bool enableWatchdog;       // Enable watchdog timer
  int watchdogTimeout;       // Watchdog timeout in seconds
  bool autoRelayControl;     // Enable automatic relay control on motion
};

// Function declarations
bool loadConfigFromSPIFFS(Config* config);
bool saveConfigToSPIFFS(const Config* config);
void setDefaultConfig(Config* config);
bool loadZonesFromJSON(Config* config, const char* jsonPath);
bool saveZonesToJSON(const Config* config, const char* jsonPath);
void printConfig(const Config* config);
void setupWiFi(Config* config);

#endif // CONFIG_H
