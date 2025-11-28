/**
 * Zone Manager Header
 * 
 * Manages zone-based relay activation logic.
 * Checks for detection/zone overlaps and controls relay GPIOs.
 */

#ifndef ZONE_MANAGER_H
#define ZONE_MANAGER_H

#include <Arduino.h>
#include <vector>
#include "config.h"
#include "tflite_detector.h"

/**
 * Zone Manager Class
 */
class ZoneManager {
public:
  ZoneManager();
  ~ZoneManager();
  
  // Initialize with configuration
  void begin(Config* config);
  
  // Update relay states based on detections
  void update(const std::vector<Detection>& detections, int frameWidth, int frameHeight);
  
  // Manual relay control
  void activateRelay(int pin);
  void deactivateRelay(int pin);
  void toggleRelay(int pin);
  
  // Emergency stop - disable all relays
  void disableAllRelays();
  
  // Zone management
  bool addZone(const Zone& zone);
  bool removeZone(int zoneId);
  bool updateZone(int zoneId, const Zone& zone);
  Zone* getZone(int zoneId);
  std::vector<Zone>* getZones();
  
  // Get relay states
  bool getRelayState(int pin);
  void getActiveRelays(bool* states, int count);
  
  // Statistics
  int getTotalDetections() { return totalDetections; }
  int getZoneDetectionCount(int zoneId);
  void resetStatistics();
  
private:
  Config* config;
  
  // Relay state tracking
  struct RelayState {
    int pin;
    bool active;
    unsigned long lastActivationTime;
    int activationCount;
  };
  std::vector<RelayState> relayStates;
  
  // Statistics
  int totalDetections;
  std::vector<int> zoneDetectionCounts;
  
  // Private methods
  bool checkOverlap(const Detection& detection, const Zone& zone, int frameWidth, int frameHeight);
  void updateZoneState(Zone* zone, bool detected);
  void updateRelayForZone(const Zone& zone);
  void initializeRelayState(int pin);
  RelayState* getRelayStatePtr(int pin);
  void setRelayPinState(int pin, bool active);
};

#endif // ZONE_MANAGER_H
