/**
 * MJPEG Stream Consumer Header
 * 
 * Connects to HTTP MJPEG stream (multipart/x-mixed-replace)
 * Commonly used by ESP32-CAM and IP cameras.
 */

#ifndef MJPEG_STREAM_H
#define MJPEG_STREAM_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

#define MJPEG_BUFFER_SIZE (100 * 1024)  // 100KB buffer for MJPEG data
#define BOUNDARY_MAX_LENGTH 128

/**
 * MJPEG Stream Client Class
 */
class MJPEGStream {
public:
  MJPEGStream();
  ~MJPEGStream();
  
  // Initialize with stream URL
  bool begin(const char* streamURL);
  
  // Initialize with IP and port (convenience method)
  bool begin(const char* ip, int port, const char* path = "/stream");
  
  // Fetch next frame from stream
  bool fetchFrame(uint8_t** frameData, size_t* frameSize);
  
  // Reconnect to stream
  bool reconnect();
  
  // Disconnect from stream
  void disconnect();
  
  // Check if connected
  bool isConnected();
  
  // Get stream statistics
  int getFrameCount() { return frameCount; }
  float getAverageFPS();
  
private:
  // Connection
  WiFiClient client;
  HTTPClient http;
  String streamURL;
  bool connected;
  
  // Stream parsing
  uint8_t* buffer;
  size_t bufferSize;
  size_t bufferPos;
  char boundary[BOUNDARY_MAX_LENGTH];
  bool boundaryFound;
  
  // Statistics
  int frameCount;
  unsigned long lastFrameTime;
  unsigned long firstFrameTime;
  
  // Private methods
  bool connectToStream();
  bool findBoundary();
  bool extractFrame(uint8_t** frameData, size_t* frameSize);
  int findBoundaryInBuffer();
  bool readMoreData();
};

#endif // MJPEG_STREAM_H
