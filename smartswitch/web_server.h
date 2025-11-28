/**
 * Web Server Manager Header
 * 
 * Async HTTP server for web UI, JSON API endpoints,
 * and WebSocket for real-time updates.
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <DNSServer.h>
#include "config.h"
#include "zone_manager.h"
#include "tflite_detector.h"
#include "mjpeg_stream.h"

/**
 * Web Server Manager Class
 */
class WebServerManager {
public:
  WebServerManager();
  ~WebServerManager();
  
  // Initialize server with configuration
  void begin(Config* config, ZoneManager* zoneMgr, TFLiteDetector* detector, MJPEGStream* stream);
  
  // Handle client requests (call in loop)
  void handleClient();
  
  // Broadcast frame to WebSocket clients
  void broadcastFrame(uint8_t* jpegData, size_t jpegSize, const std::vector<Detection>& detections);
  
  // Broadcast relay states
  void broadcastRelayStates();
  
  // Broadcast statistics
  void broadcastStatistics();
  
  // Camera control
  void handleStartCamera(AsyncWebServerRequest* request);
  void handleStopCamera(AsyncWebServerRequest* request);
  void handleCameraStatus(AsyncWebServerRequest* request);
  
private:
  AsyncWebServer* server;
  AsyncWebSocket* ws;
  DNSServer* dnsServer;
  
  Config* config;
  ZoneManager* zoneManager;
  TFLiteDetector* detector;
  MJPEGStream* mjpegStream;
  
  unsigned long lastBroadcast;
  bool apMode;
  
  // Setup routes
  void setupRoutes();
  void setupWebSocketHandlers();
  
  // API endpoint handlers
  void handleGetConfig(AsyncWebServerRequest* request);
  void handleSaveConfig(AsyncWebServerRequest* request, uint8_t* data, size_t len);
  void handleGetZones(AsyncWebServerRequest* request);
  void handleAddZone(AsyncWebServerRequest* request, uint8_t* data, size_t len);
  void handleUpdateZone(AsyncWebServerRequest* request, uint8_t* data, size_t len);
  void handleDeleteZone(AsyncWebServerRequest* request);
  void handleGetRelayStates(AsyncWebServerRequest* request);
  void handleSetRelay(AsyncWebServerRequest* request);
  void handleEmergencyStop(AsyncWebServerRequest* request);
  void handleGetStatistics(AsyncWebServerRequest* request);
  void handleResetStatistics(AsyncWebServerRequest* request);
  void handleGetSystemInfo(AsyncWebServerRequest* request);
  void handleTestConnection(AsyncWebServerRequest* request);
  
  // WebSocket handlers
  void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                       AwsEventType type, void* arg, uint8_t* data, size_t len);
  
  // Helper methods
  String serializeConfig();
  String serializeZones();
  String serializeRelayStates();
  String serializeStatistics();
  String serializeSystemInfo();
  bool parseZoneJSON(const char* json, Zone* zone);
};

#endif // WEB_SERVER_H
