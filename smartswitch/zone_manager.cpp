/**
 * Zone Manager Implementation
 * 
 * Manages zone-based relay activation with overlap detection and timeouts.
 */

#include "zone_manager.h"

ZoneManager::ZoneManager() {
  config = nullptr;
  totalDetections = 0;
}

ZoneManager::~ZoneManager() {
}

void ZoneManager::begin(Config* cfg) {
  config = cfg;
  
  // Initialize relay states for all configured zones
  for (Zone& zone : config->zones) {
    for (int i = 0; i < zone.numRelays; i++) {
      initializeRelayState(zone.relayPins[i]);
    }
  }
  
  // Initialize zone detection counts
  zoneDetectionCounts.resize(config->zones.size(), 0);
  
  Serial.printf("Zone manager initialized with %d zones\n", config->zones.size());
}

void ZoneManager::update(const std::vector<Detection>& detections, int frameWidth, int frameHeight) {
  // Update each zone based on detections
  for (size_t i = 0; i < config->zones.size(); i++) {
    Zone& zone = config->zones[i];
    bool detected = false;
    
    // Check if any detection overlaps with this zone
    for (const Detection& detection : detections) {
      if (checkOverlap(detection, zone, frameWidth, frameHeight)) {
        detected = true;
        zoneDetectionCounts[i]++;
        totalDetections++;
        break;
      }
    }
    
    // Update zone state and relays
    updateZoneState(&zone, detected);
  }
}

bool ZoneManager::checkOverlap(const Detection& detection, const Zone& zone, int frameWidth, int frameHeight) {
  // Convert normalized detection coordinates to pixel coordinates
  int detX = (int)(detection.x * frameWidth);
  int detY = (int)(detection.y * frameHeight);
  int detWidth = (int)(detection.width * frameWidth);
  int detHeight = (int)(detection.height * frameHeight);
  
  // Check rectangle overlap
  bool overlap = (detX < zone.x + zone.width &&
                  detX + detWidth > zone.x &&
                  detY < zone.y + zone.height &&
                  detY + detHeight > zone.y);
  
  return overlap;
}

void ZoneManager::updateZoneState(Zone* zone, bool detected) {
  unsigned long currentTime = millis();
  
  if (detected) {
    // Person detected in zone
    if (!zone->active) {
      Serial.printf("✓ Zone %d (%s) ACTIVATED\n", zone->id, zone->name);
      zone->active = true;
    }
    zone->lastDetectionTime = currentTime;
    
    // Activate relays for this zone
    updateRelayForZone(*zone);
    
  } else {
    // No person detected
    if (zone->active) {
      // Check timeout
      unsigned long elapsed = currentTime - zone->lastDetectionTime;
      if (elapsed >= (unsigned long)(zone->timeout * 1000)) {
        // Timeout expired, deactivate zone
        Serial.printf("⊗ Zone %d (%s) DEACTIVATED (timeout)\n", zone->id, zone->name);
        zone->active = false;
        
        // Deactivate relays for this zone
        for (int i = 0; i < zone->numRelays; i++) {
          // Check if this relay is used by any other active zone
          bool usedByOtherZone = false;
          for (const Zone& otherZone : config->zones) {
            if (otherZone.id != zone->id && otherZone.active) {
              for (int j = 0; j < otherZone.numRelays; j++) {
                if (otherZone.relayPins[j] == zone->relayPins[i]) {
                  usedByOtherZone = true;
                  break;
                }
              }
            }
            if (usedByOtherZone) break;
          }
          
          if (!usedByOtherZone) {
            deactivateRelay(zone->relayPins[i]);
          }
        }
      }
    }
  }
}

void ZoneManager::updateRelayForZone(const Zone& zone) {
  for (int i = 0; i < zone.numRelays; i++) {
    activateRelay(zone.relayPins[i]);
  }
}

void ZoneManager::activateRelay(int pin) {
  RelayState* state = getRelayStatePtr(pin);
  if (state) {
    if (!state->active) {
      setRelayPinState(pin, true);
      state->active = true;
      state->lastActivationTime = millis();
      state->activationCount++;
      Serial.printf("  → Relay GPIO %d ON\n", pin);
    }
  }
}

void ZoneManager::deactivateRelay(int pin) {
  RelayState* state = getRelayStatePtr(pin);
  if (state) {
    if (state->active) {
      setRelayPinState(pin, false);
      state->active = false;
      Serial.printf("  → Relay GPIO %d OFF\n", pin);
    }
  }
}

void ZoneManager::toggleRelay(int pin) {
  RelayState* state = getRelayStatePtr(pin);
  if (state) {
    if (state->active) {
      deactivateRelay(pin);
    } else {
      activateRelay(pin);
    }
  }
}

void ZoneManager::disableAllRelays() {
  Serial.println("⚠ EMERGENCY STOP - Disabling all relays!");
  
  for (RelayState& state : relayStates) {
    setRelayPinState(state.pin, false);
    state.active = false;
  }
  
  // Deactivate all zones
  for (Zone& zone : config->zones) {
    zone.active = false;
  }
}

bool ZoneManager::addZone(const Zone& zone) {
  if (config->zones.size() >= MAX_ZONES) {
    Serial.println("ERROR: Maximum zones reached");
    return false;
  }
  
  config->zones.push_back(zone);
  zoneDetectionCounts.push_back(0);
  
  // Initialize relay states for new zone
  for (int i = 0; i < zone.numRelays; i++) {
    initializeRelayState(zone.relayPins[i]);
  }
  
  Serial.printf("✓ Zone %d added\n", zone.id);
  return true;
}

bool ZoneManager::removeZone(int zoneId) {
  for (size_t i = 0; i < config->zones.size(); i++) {
    if (config->zones[i].id == zoneId) {
      // Deactivate zone first
      Zone& zone = config->zones[i];
      if (zone.active) {
        zone.active = false;
        for (int j = 0; j < zone.numRelays; j++) {
          deactivateRelay(zone.relayPins[j]);
        }
      }
      
      config->zones.erase(config->zones.begin() + i);
      zoneDetectionCounts.erase(zoneDetectionCounts.begin() + i);
      Serial.printf("✓ Zone %d removed\n", zoneId);
      return true;
    }
  }
  
  Serial.printf("ERROR: Zone %d not found\n", zoneId);
  return false;
}

bool ZoneManager::updateZone(int zoneId, const Zone& zone) {
  for (size_t i = 0; i < config->zones.size(); i++) {
    if (config->zones[i].id == zoneId) {
      config->zones[i] = zone;
      Serial.printf("✓ Zone %d updated\n", zoneId);
      return true;
    }
  }
  
  Serial.printf("ERROR: Zone %d not found\n", zoneId);
  return false;
}

Zone* ZoneManager::getZone(int zoneId) {
  for (Zone& zone : config->zones) {
    if (zone.id == zoneId) {
      return &zone;
    }
  }
  return nullptr;
}

std::vector<Zone>* ZoneManager::getZones() {
  return &(config->zones);
}

bool ZoneManager::getRelayState(int pin) {
  RelayState* state = getRelayStatePtr(pin);
  return state ? state->active : false;
}

void ZoneManager::getActiveRelays(bool* states, int count) {
  for (int i = 0; i < count && i < (int)relayStates.size(); i++) {
    states[i] = relayStates[i].active;
  }
}

int ZoneManager::getZoneDetectionCount(int zoneId) {
  for (size_t i = 0; i < config->zones.size(); i++) {
    if (config->zones[i].id == zoneId) {
      return zoneDetectionCounts[i];
    }
  }
  return 0;
}

void ZoneManager::resetStatistics() {
  totalDetections = 0;
  for (size_t i = 0; i < zoneDetectionCounts.size(); i++) {
    zoneDetectionCounts[i] = 0;
  }
  for (RelayState& state : relayStates) {
    state.activationCount = 0;
  }
  Serial.println("Statistics reset");
}

void ZoneManager::initializeRelayState(int pin) {
  // Check if already initialized
  for (const RelayState& state : relayStates) {
    if (state.pin == pin) {
      return; // Already initialized
    }
  }
  
  RelayState state;
  state.pin = pin;
  state.active = false;
  state.lastActivationTime = 0;
  state.activationCount = 0;
  
  relayStates.push_back(state);
  
  // Initialize GPIO
  pinMode(pin, OUTPUT);
  digitalWrite(pin, config->relayActiveHigh ? LOW : HIGH);
}

ZoneManager::RelayState* ZoneManager::getRelayStatePtr(int pin) {
  for (RelayState& state : relayStates) {
    if (state.pin == pin) {
      return &state;
    }
  }
  return nullptr;
}

void ZoneManager::setRelayPinState(int pin, bool active) {
  int logicalState = active ? 1 : 0;
  int physicalState;
  
  if (config->relayActiveHigh) {
    physicalState = active ? HIGH : LOW;
    digitalWrite(pin, physicalState);
  } else {
    physicalState = active ? LOW : HIGH;
    digitalWrite(pin, physicalState);
  }
  
  Serial.printf("  [GPIO %d] Logical=%s Physical=%s (ActiveHigh=%s)\n", 
                pin, 
                logicalState ? "ON" : "OFF",
                physicalState == HIGH ? "HIGH" : "LOW",
                config->relayActiveHigh ? "true" : "false");
}
