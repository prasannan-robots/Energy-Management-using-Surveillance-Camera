/**
 * Web Server Manager Implementation
 * 
 * Async HTTP server with JSON API and WebSocket for real-time updates.
 */

#include "web_server.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <HTTPClient.h>
#include "config.h"

WebServerManager::WebServerManager() {
  server = nullptr;
  ws = nullptr;
  dnsServer = nullptr;
  config = nullptr;
  zoneManager = nullptr;
  detector = nullptr;
  lastBroadcast = 0;
  apMode = false;
}

WebServerManager::~WebServerManager() {
  if (server) {
    delete server;
  }
  if (ws) {
    delete ws;
  }
  if (dnsServer) {
    dnsServer->stop();
    delete dnsServer;
  }
}

void WebServerManager::begin(Config* cfg, ZoneManager* zoneMgr, TFLiteDetector* det, MJPEGStream* stream) {
  config = cfg;
  zoneManager = zoneMgr;
  detector = det;
  mjpegStream = stream;
  apMode = false;
  
  Serial.println("Creating web server...");
  yield();
  
  // Create server
  server = new AsyncWebServer(80);
  
  Serial.println("Creating WebSocket...");
  yield();
  
  // Create WebSocket
  ws = new AsyncWebSocket("/ws");
  ws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                    AwsEventType type, void* arg, uint8_t* data, size_t len) {
    this->onWebSocketEvent(server, client, type, arg, data, len);
  });
  server->addHandler(ws);
  
  Serial.println("Setting up routes...");
  yield();
  
  // Setup routes
  setupRoutes();
  
  Serial.println("Starting server...");
  yield();
  
  // Start server
  server->begin();
  Serial.println("✓ Web server started on port 80");
}

void WebServerManager::setupRoutes() {
  // Simple test endpoint
  server->on("/test", HTTP_GET, [](AsyncWebServerRequest* request) {
    String msg = "HI - Web server is working!\n";
    msg += "LittleFS mounted: " + String(LittleFS.begin() ? "YES" : "NO") + "\n";
    msg += "Files in root:\n";
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while(file) {
      msg += "  " + String(file.name()) + " (" + String(file.size()) + " bytes)\n";
      file = root.openNextFile();
    }
    request->send(200, "text/plain", msg);
  });
  
  // Root endpoint - try both LittleFS and fallback
  server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (LittleFS.exists("/index.html")) {
      request->send(LittleFS, "/index.html", "text/html");
    } else {
      request->send(200, "text/html", 
        "<h1>ESP32 Smart Zone Switch</h1>"
        "<p>Web server is running!</p>"
        "<p>But LittleFS files not found. Please upload data folder using LittleFS Upload.</p>"
        "<p><a href='/test'>Test endpoint - Check filesystem</a></p>");
    }
  });
  
  // 404 handler
  server->onNotFound([](AsyncWebServerRequest* request) {
    String message = "File Not Found\n\n";
    message += "URI: " + request->url() + "\n";
    message += "Method: " + String((request->method() == HTTP_GET) ? "GET" : "POST") + "\n";
    message += "\nTry:\n";
    message += "  /test - Simple test page\n";
    message += "  / - Main interface\n";
    request->send(404, "text/plain", message);
  });
  
  // Serve static files
  server->serveStatic("/", LittleFS, "/");
  
  // API: Get configuration
  server->on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleGetConfig(request);
  });
  
  // API: Save configuration
  server->on("/api/config", HTTP_POST, [](AsyncWebServerRequest* request) {},
    nullptr,
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
      handleSaveConfig(request, data, len);
    });
  
  // API: Get zones
  server->on("/api/zones", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleGetZones(request);
  });
  
  // API: Add zone
  server->on("/api/zones/add", HTTP_POST, [](AsyncWebServerRequest* request) {},
    nullptr,
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
      handleAddZone(request, data, len);
    });
  
  // API: Update zone
  server->on("/api/zones/update", HTTP_POST, [](AsyncWebServerRequest* request) {},
    nullptr,
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
      handleUpdateZone(request, data, len);
    });
  
  // API: Delete zone
  server->on("/api/zones/delete", HTTP_DELETE, [this](AsyncWebServerRequest* request) {
    handleDeleteZone(request);
  });
  
  // API: Get relay states
  server->on("/api/relays", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleGetRelayStates(request);
  });
  
  // API: Set relay
  server->on("/api/relays/set", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleSetRelay(request);
  });
  
  // API: Emergency stop
  server->on("/api/emergency-stop", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleEmergencyStop(request);
  });
  
  // API: Get statistics
  server->on("/api/statistics", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleGetStatistics(request);
  });
  
  // API: Reset statistics
  server->on("/api/statistics/reset", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleResetStatistics(request);
  });
  
  // API: Get system info
  server->on("/api/system", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleGetSystemInfo(request);
  });
  
  // API: Test CCTV connection
  server->on("/api/test-connection", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleTestConnection(request);
  });
  
  // API: Start camera stream
  server->on("/api/camera/start", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleStartCamera(request);
  });
  
  // API: Stop camera stream
  server->on("/api/camera/stop", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleStopCamera(request);
  });
  
  // API: Get camera status
  server->on("/api/camera/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleCameraStatus(request);
  });
  
  // 404 handler
  server->onNotFound([](AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not found");
  });
}

void WebServerManager::handleClient() {
  // Cleanup WebSocket clients
  ws->cleanupClients();
}

void WebServerManager::handleGetConfig(AsyncWebServerRequest* request) {
  String json = serializeConfig();
  request->send(200, "application/json", json);
}

void WebServerManager::handleSaveConfig(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
  // Parse JSON
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, data, len);
  
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  // Update config
  if (doc.containsKey("cctvIP")) {
    strlcpy(config->cctvIP, doc["cctvIP"], MAX_IP_LENGTH);
  }
  if (doc.containsKey("cctvPort")) {
    config->cctvPort = doc["cctvPort"];
  }
  if (doc.containsKey("detectionThreshold")) {
    config->detectionThreshold = doc["detectionThreshold"];
  }
  if (doc.containsKey("globalTimeout")) {
    config->globalTimeout = doc["globalTimeout"];
  }
  
  // Save to SPIFFS
  if (saveConfigToSPIFFS(config)) {
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(500, "application/json", "{\"error\":\"Failed to save\"}");
  }
}

void WebServerManager::handleGetZones(AsyncWebServerRequest* request) {
  String json = serializeZones();
  request->send(200, "application/json", json);
}

void WebServerManager::handleAddZone(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
  Zone zone;
  if (!parseZoneJSON((char*)data, &zone)) {
    request->send(400, "application/json", "{\"error\":\"Invalid zone data\"}");
    return;
  }
  
  if (zoneManager->addZone(zone)) {
    saveConfigToSPIFFS(config);
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(500, "application/json", "{\"error\":\"Failed to add zone\"}");
  }
}

void WebServerManager::handleUpdateZone(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
  Zone zone;
  if (!parseZoneJSON((char*)data, &zone)) {
    request->send(400, "application/json", "{\"error\":\"Invalid zone data\"}");
    return;
  }
  
  if (zoneManager->updateZone(zone.id, zone)) {
    saveConfigToSPIFFS(config);
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(404, "application/json", "{\"error\":\"Zone not found\"}");
  }
}

void WebServerManager::handleDeleteZone(AsyncWebServerRequest* request) {
  if (!request->hasParam("id")) {
    request->send(400, "application/json", "{\"error\":\"Missing zone ID\"}");
    return;
  }
  
  int zoneId = request->getParam("id")->value().toInt();
  
  if (zoneManager->removeZone(zoneId)) {
    saveConfigToSPIFFS(config);
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(404, "application/json", "{\"error\":\"Zone not found\"}");
  }
}

void WebServerManager::handleGetRelayStates(AsyncWebServerRequest* request) {
  String json = serializeRelayStates();
  request->send(200, "application/json", json);
}

void WebServerManager::handleSetRelay(AsyncWebServerRequest* request) {
  if (!request->hasParam("pin") || !request->hasParam("state")) {
    request->send(400, "application/json", "{\"error\":\"Missing parameters\"}");
    return;
  }
  
  int pin = request->getParam("pin")->value().toInt();
  bool state = request->getParam("state")->value() == "true";
  
  if (state) {
    zoneManager->activateRelay(pin);
  } else {
    zoneManager->deactivateRelay(pin);
  }
  
  request->send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleEmergencyStop(AsyncWebServerRequest* request) {
  zoneManager->disableAllRelays();
  request->send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleGetStatistics(AsyncWebServerRequest* request) {
  String json = serializeStatistics();
  request->send(200, "application/json", json);
}

void WebServerManager::handleResetStatistics(AsyncWebServerRequest* request) {
  zoneManager->resetStatistics();
  request->send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleGetSystemInfo(AsyncWebServerRequest* request) {
  String json = serializeSystemInfo();
  request->send(200, "application/json", json);
}

void WebServerManager::handleTestConnection(AsyncWebServerRequest* request) {
  // Test connection to CCTV stream
  
  // Yield to prevent watchdog timeout
  yield();
  
  String url = "http://";
  url += config->cctvIP;
  url += ":";
  url += String(config->cctvPort);
  url += config->streamPath;
  
  Serial.printf("Testing connection to: %s\n", url.c_str());
  
  HTTPClient http;
  WiFiClient client;
  
  // Use shorter timeout to prevent watchdog
  http.setTimeout(3000); // 3 seconds max
  http.setConnectTimeout(2000); // 2 seconds to connect
  
  bool success = false;
  int httpCode = -1;
  
  if (http.begin(client, url)) {
    // Yield during request
    yield();
    
    httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      success = true;
      Serial.println("✓ CCTV connection test successful");
    } else {
      Serial.printf("✗ CCTV connection test failed: %d\n", httpCode);
    }
    
    http.end();
  } else {
    Serial.println("✗ Failed to initialize HTTP client");
  }
  
  client.stop();
  
  // Yield after operation
  yield();
  
  if (success) {
    request->send(200, "application/json", "{\"success\":true,\"message\":\"Connection successful\"}");
  } else {
    String json = "{\"success\":false,\"message\":\"Connection failed";
    if (httpCode == -1) {
      json += " - Camera unreachable";
    } else {
      json += " - HTTP code: " + String(httpCode);
    }
    json += "\",\"code\":" + String(httpCode) + "}";
    request->send(200, "application/json", json);
  }
}

void WebServerManager::broadcastFrame(uint8_t* jpegData, size_t jpegSize, const std::vector<Detection>& detections) {
  // Rate limit broadcasts to avoid overwhelming clients
  if (millis() - lastBroadcast < 100) { // Max 10 FPS
    return;
  }
  lastBroadcast = millis();
  
  // Send via WebSocket (binary frame)
  // Format: [type:1byte][jpegSize:4bytes][jpeg][numDetections:2bytes][detections]
  // This is a simplified version - for production, use proper framing
  
  if (ws->count() > 0) {
    // For now, just send detection count
    StaticJsonDocument<256> doc;
    doc["type"] = "detections";
    doc["count"] = detections.size();
    
    JsonArray dets = doc.createNestedArray("detections");
    for (const Detection& det : detections) {
      JsonObject obj = dets.createNestedObject();
      obj["x"] = det.x;
      obj["y"] = det.y;
      obj["width"] = det.width;
      obj["height"] = det.height;
      obj["confidence"] = det.confidence;
    }
    
    String json;
    serializeJson(doc, json);
    ws->textAll(json);
  }
}

void WebServerManager::broadcastRelayStates() {
  if (ws->count() > 0) {
    String json = serializeRelayStates();
    ws->textAll(json);
  }
}

void WebServerManager::broadcastStatistics() {
  if (ws->count() > 0) {
    String json = serializeStatistics();
    ws->textAll(json);
  }
}

void WebServerManager::onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                       AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("WebSocket client #%u connected\n", client->id());
    
    // Send initial state
    client->text(serializeZones());
    
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    
  } else if (type == WS_EVT_DATA) {
    // Handle incoming WebSocket messages if needed
    Serial.printf("WebSocket data from client #%u\n", client->id());
  }
}

String WebServerManager::serializeConfig() {
  StaticJsonDocument<1024> doc;
  
  doc["cctvIP"] = config->cctvIP;
  doc["cctvPort"] = config->cctvPort;
  doc["streamPath"] = config->streamPath;
  doc["detectionThreshold"] = config->detectionThreshold;
  doc["globalTimeout"] = config->globalTimeout;
  doc["relayActiveHigh"] = config->relayActiveHigh;
  
  String json;
  serializeJson(doc, json);
  return json;
}

String WebServerManager::serializeZones() {
  DynamicJsonDocument doc(4096);
  JsonArray array = doc.createNestedArray("zones");
  
  for (const Zone& zone : config->zones) {
    JsonObject obj = array.createNestedObject();
    obj["id"] = zone.id;
    obj["name"] = zone.name;
    obj["x"] = zone.x;
    obj["y"] = zone.y;
    obj["width"] = zone.width;
    obj["height"] = zone.height;
    obj["timeout"] = zone.timeout;
    obj["active"] = zone.active;
    
    JsonArray pins = obj.createNestedArray("relayPins");
    for (int i = 0; i < zone.numRelays; i++) {
      pins.add(zone.relayPins[i]);
    }
  }
  
  String json;
  serializeJson(doc, json);
  return json;
}

String WebServerManager::serializeRelayStates() {
  StaticJsonDocument<512> doc;
  JsonArray array = doc.createNestedArray("relays");
  
  // Get all unique relay pins
  std::vector<int> pins;
  for (const Zone& zone : config->zones) {
    for (int i = 0; i < zone.numRelays; i++) {
      bool found = false;
      for (int pin : pins) {
        if (pin == zone.relayPins[i]) {
          found = true;
          break;
        }
      }
      if (!found) {
        pins.push_back(zone.relayPins[i]);
      }
    }
  }
  
  for (int pin : pins) {
    JsonObject obj = array.createNestedObject();
    obj["pin"] = pin;
    obj["active"] = zoneManager->getRelayState(pin);
  }
  
  String json;
  serializeJson(doc, json);
  return json;
}

String WebServerManager::serializeStatistics() {
  StaticJsonDocument<1024> doc;
  
  doc["totalDetections"] = zoneManager->getTotalDetections();
  
  JsonArray zones = doc.createNestedArray("zones");
  for (const Zone& zone : config->zones) {
    JsonObject obj = zones.createNestedObject();
    obj["id"] = zone.id;
    obj["name"] = zone.name;
    obj["detections"] = zoneManager->getZoneDetectionCount(zone.id);
  }
  
  String json;
  serializeJson(doc, json);
  return json;
}

String WebServerManager::serializeSystemInfo() {
  StaticJsonDocument<1024> doc;
  
  doc["chipModel"] = ESP.getChipModel();
  doc["cpuFreq"] = ESP.getCpuFreqMHz();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["freePsram"] = ESP.getFreePsram();
  doc["wifiRSSI"] = WiFi.RSSI();
  doc["ipAddress"] = WiFi.localIP().toString();
  
  // Motion detector is used instead of TFLite
  if (detector != nullptr) {
    char modelInfo[128];
    detector->getModelInfo(modelInfo, sizeof(modelInfo));
    doc["modelInfo"] = modelInfo;
    doc["lastInferenceTime"] = detector->getLastInferenceTime();
  } else {
    doc["modelInfo"] = "Simple Motion Detection";
    doc["lastInferenceTime"] = 0;
  }
  
  String json;
  serializeJson(doc, json);
  return json;
}

bool WebServerManager::parseZoneJSON(const char* json, Zone* zone) {
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    return false;
  }
  
  zone->id = doc["id"] | 0;
  strlcpy(zone->name, doc["name"] | "Unnamed", 32);
  zone->x = doc["x"] | 0;
  zone->y = doc["y"] | 0;
  zone->width = doc["width"] | 100;
  zone->height = doc["height"] | 100;
  zone->timeout = doc["timeout"] | 5;
  zone->active = false;
  zone->lastDetectionTime = 0;
  
  JsonArray pins = doc["relayPins"];
  zone->numRelays = 0;
  for (JsonVariant pin : pins) {
    if (zone->numRelays < MAX_RELAYS_PER_ZONE) {
      zone->relayPins[zone->numRelays++] = pin.as<int>();
    }
  }
  
  return true;
}

void WebServerManager::handleStartCamera(AsyncWebServerRequest* request) {
  // Yield before starting
  yield();
  
  Serial.println("Starting camera connection from web UI...");
  
  if (mjpegStream->isConnected()) {
    request->send(200, "application/json", "{\"success\":true,\"message\":\"Camera already connected\"}");
    return;
  }
  
  // Attempt to connect
  bool success = mjpegStream->begin(config->cctvIP, config->cctvPort, config->streamPath);
  
  yield();
  
  if (success) {
    Serial.println("✓ Camera connected successfully");
    request->send(200, "application/json", "{\"success\":true,\"message\":\"Camera connected successfully\"}");
  } else {
    Serial.println("✗ Failed to connect to camera");
    request->send(200, "application/json", "{\"success\":false,\"message\":\"Failed to connect to camera\"}");
  }
}

void WebServerManager::handleStopCamera(AsyncWebServerRequest* request) {
  Serial.println("Stopping camera connection from web UI...");
  
  mjpegStream->disconnect();
  
  request->send(200, "application/json", "{\"success\":true,\"message\":\"Camera disconnected\"}");
}

void WebServerManager::handleCameraStatus(AsyncWebServerRequest* request) {
  StaticJsonDocument<256> doc;
  
  doc["connected"] = mjpegStream->isConnected();
  doc["frameCount"] = mjpegStream->getFrameCount();
  doc["avgFPS"] = mjpegStream->getAverageFPS();
  
  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}
