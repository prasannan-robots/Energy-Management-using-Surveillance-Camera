/**
 * MJPEG Stream Consumer Implementation
 * 
 * Connects to HTTP MJPEG stream and extracts JPEG frames.
 * Works with ESP32-CAM and most IP cameras that support MJPEG.
 */

#include "mjpeg_stream.h"

MJPEGStream::MJPEGStream() {
  buffer = nullptr;
  bufferSize = MJPEG_BUFFER_SIZE;
  bufferPos = 0;
  connected = false;
  boundaryFound = false;
  frameCount = 0;
  lastFrameTime = 0;
  firstFrameTime = 0;
  memset(boundary, 0, BOUNDARY_MAX_LENGTH);
}

MJPEGStream::~MJPEGStream() {
  disconnect();
  if (buffer) {
    free(buffer);
  }
}

bool MJPEGStream::begin(const char* url) {
  streamURL = String(url);
  
  // Allocate buffer
  if (!buffer) {
    buffer = (uint8_t*)malloc(bufferSize);
    if (!buffer) {
      Serial.println("ERROR: Failed to allocate MJPEG buffer");
      return false;
    }
  }
  
  Serial.printf("MJPEG Stream URL: %s\n", streamURL.c_str());
  
  // Connect to stream
  return connectToStream();
}

bool MJPEGStream::begin(const char* ip, int port, const char* path) {
  // Build stream URL
  streamURL = "http://";
  streamURL += ip;
  streamURL += ":";
  streamURL += String(port);
  streamURL += path;
  
  return begin(streamURL.c_str());
}

bool MJPEGStream::connectToStream() {
  Serial.println("Connecting to MJPEG stream...");
  
  // Yield before connecting
  yield();
  
  // Use shorter timeouts to prevent watchdog
  http.setTimeout(5000); // 5 second timeout
  http.setConnectTimeout(3000); // 3 second connect timeout
  
  if (!http.begin(client, streamURL)) {
    Serial.println("ERROR: Failed to initialize HTTP client");
    return false;
  }
  
  // Yield during request
  yield();
  
  int httpCode = http.GET();
  
  // Yield after request
  yield();
  
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("ERROR: HTTP GET failed, code: %d\n", httpCode);
    if (httpCode == -1) {
      Serial.println("⚠ Camera unreachable - check IP address and network");
    }
    http.end();
    client.stop();
    return false;
  }
  
  // Get content type to find boundary
  String contentType = http.header("Content-Type");
  Serial.printf("Content-Type: %s\n", contentType.c_str());
  
  // Check if it's a multipart stream or simple JPEG
  if (contentType.indexOf("multipart") >= 0) {
    // Standard MJPEG stream with boundaries
    int boundaryIndex = contentType.indexOf("boundary=");
    if (boundaryIndex == -1) {
      Serial.println("⚠ Multipart stream but no boundary found");
      Serial.println("  Trying fallback mode...");
      strcpy(boundary, "--myboundary");  // Try common default
    } else {
      String boundaryStr = contentType.substring(boundaryIndex + 9);
      boundaryStr.trim();
      snprintf(boundary, BOUNDARY_MAX_LENGTH, "--%s", boundaryStr.c_str());
      Serial.printf("Boundary: %s\n", boundary);
    }
    boundaryFound = true;
  } else {
    // Non-standard stream (might be single JPEG or streaming without boundaries)
    Serial.println("⚠ Not a multipart stream - using single-frame mode");
    Serial.println("  This camera serves individual JPEG frames");
    boundaryFound = false;  // Will read entire response as one frame
    strcpy(boundary, "");
  }
  
  connected = true;
  bufferPos = 0;
  firstFrameTime = millis();
  
  Serial.println("✓ Connected to camera stream");
  return true;
}

bool MJPEGStream::fetchFrame(uint8_t** frameData, size_t* frameSize) {
  if (!connected) {
    return false;
  }
  
  // Read data from stream and extract frame
  return extractFrame(frameData, frameSize);
}

bool MJPEGStream::extractFrame(uint8_t** frameData, size_t* frameSize) {
  // Handle non-multipart streams (single JPEG per request)
  if (!boundaryFound) {
    // Read all available data as a single JPEG frame
    int readAttempts = 0;
    while (readMoreData() && readAttempts < 50) {
      // Keep reading until no more data (max 50 attempts to prevent overflow)
      delay(10);
      readAttempts++;
    }
    
    // Check if we have a complete JPEG (starts with FFD8, ends with FFD9)
    if (bufferPos > 100) {
      // Find JPEG start marker (0xFF 0xD8)
      int jpegStart = -1;
      for (size_t i = 0; i < bufferPos - 1; i++) {
        if (buffer[i] == 0xFF && buffer[i+1] == 0xD8) {
          jpegStart = i;
          break;
        }
      }
      
      // Find JPEG end marker (0xFF 0xD9)
      int jpegEnd = -1;
      for (size_t i = bufferPos - 2; i > 0; i--) {
        if (buffer[i] == 0xFF && buffer[i+1] == 0xD9) {
          jpegEnd = i + 2;
          break;
        }
      }
      
      if (jpegStart >= 0 && jpegEnd > jpegStart) {
        int jpegSize = jpegEnd - jpegStart;
        
        *frameData = (uint8_t*)malloc(jpegSize);
        if (*frameData) {
          memcpy(*frameData, &buffer[jpegStart], jpegSize);
          *frameSize = jpegSize;
          
          bufferPos = 0;  // Reset buffer for next frame
          frameCount++;
          lastFrameTime = millis();
          
          // Reconnect for next frame (single-shot mode)
          disconnect();
          delay(50);  // Small delay before reconnecting
          
          return true;
        }
      }
    }
    
    bufferPos = 0;
    return false;
  }
  
  // Standard MJPEG with boundaries
  while (true) {
    // Read more data if buffer is running low
    if (bufferPos < 1024) {
      if (!readMoreData()) {
        return false;
      }
    }
    
    // Find boundary marker
    int boundaryPos = findBoundaryInBuffer();
    if (boundaryPos == -1) {
      // No boundary found, need more data
      if (!readMoreData()) {
        return false;
      }
      continue;
    }
    
    // Skip past boundary and headers to find JPEG data
    int searchPos = boundaryPos + strlen(boundary);
    
    // Look for \r\n\r\n (end of headers)
    while (searchPos < (int)bufferPos - 3) {
      if (buffer[searchPos] == '\r' && buffer[searchPos + 1] == '\n' &&
          buffer[searchPos + 2] == '\r' && buffer[searchPos + 3] == '\n') {
        // Found end of headers
        searchPos += 4;
        break;
      }
      searchPos++;
    }
    
    if (searchPos >= (int)bufferPos - 3) {
      // Need more data to find headers
      if (!readMoreData()) {
        return false;
      }
      continue;
    }
    
    // JPEG data starts here
    int jpegStart = searchPos;
    
    // Find next boundary (end of JPEG)
    int nextBoundary = -1;
    for (int i = jpegStart; i < (int)bufferPos - (int)strlen(boundary); i++) {
      if (memcmp(&buffer[i], boundary, strlen(boundary)) == 0) {
        nextBoundary = i;
        break;
      }
    }
    
    if (nextBoundary == -1) {
      // Need more data to find end of frame
      if (bufferPos - jpegStart > bufferSize - 10000) {
        // Frame too large or corrupted, skip
        Serial.println("⚠ Frame too large, skipping");
        memmove(buffer, &buffer[jpegStart], bufferPos - jpegStart);
        bufferPos = bufferPos - jpegStart;
        continue;
      }
      if (!readMoreData()) {
        return false;
      }
      continue;
    }
    
    // Extract JPEG frame
    int jpegSize = nextBoundary - jpegStart;
    
    if (jpegSize > 0 && jpegSize < (int)bufferSize) {
      // Allocate frame buffer (caller must free this)
      *frameData = (uint8_t*)malloc(jpegSize);
      if (*frameData == nullptr) {
        Serial.println("ERROR: Failed to allocate frame buffer");
        return false;
      }
      
      memcpy(*frameData, &buffer[jpegStart], jpegSize);
      *frameSize = jpegSize;
      
      // Remove processed data from buffer
      memmove(buffer, &buffer[nextBoundary], bufferPos - nextBoundary);
      bufferPos = bufferPos - nextBoundary;
      
      frameCount++;
      lastFrameTime = millis();
      
      return true;
    } else {
      // Invalid frame size
      Serial.printf("⚠ Invalid frame size: %d\n", jpegSize);
      memmove(buffer, &buffer[nextBoundary], bufferPos - nextBoundary);
      bufferPos = bufferPos - nextBoundary;
      continue;
    }
  }
  
  return false;
}

int MJPEGStream::findBoundaryInBuffer() {
  for (size_t i = 0; i < bufferPos - strlen(boundary); i++) {
    if (memcmp(&buffer[i], boundary, strlen(boundary)) == 0) {
      return i;
    }
  }
  return -1;
}

bool MJPEGStream::readMoreData() {
  // Yield periodically to prevent watchdog
  static unsigned long lastYield = 0;
  if (millis() - lastYield > 1000) {
    yield();
    lastYield = millis();
  }
  
  // Get WiFi stream
  WiFiClient* stream = http.getStreamPtr();
  if (!stream) {
    Serial.println("ERROR: Stream pointer is null");
    connected = false;
    return false;
  }
  
  // Check if stream is still connected
  if (!stream->connected()) {
    Serial.println("⚠ Stream disconnected");
    connected = false;
    return false;
  }
  
  // Check if data is available
  if (!stream->available()) {
    delay(10);
    return bufferPos > 0; // Return true if we have some data buffered
  }
  
  // Read available data
  size_t available = stream->available();
  size_t spaceLeft = bufferSize - bufferPos;
  size_t toRead = min(available, spaceLeft);
  
  if (toRead == 0) {
    // Buffer full, shift data
    Serial.println("⚠ Buffer full, shifting data");
    size_t keepSize = bufferPos / 2;
    memmove(buffer, &buffer[bufferPos - keepSize], keepSize);
    bufferPos = keepSize;
    spaceLeft = bufferSize - bufferPos;
    toRead = min(available, spaceLeft);
  }
  
  size_t bytesRead = stream->readBytes(&buffer[bufferPos], toRead);
  bufferPos += bytesRead;
  
  return bytesRead > 0;
}

bool MJPEGStream::reconnect() {
  disconnect();
  delay(1000);
  return connectToStream();
}

void MJPEGStream::disconnect() {
  if (connected) {
    http.end();
    client.stop();
    connected = false;
    Serial.println("Disconnected from MJPEG stream");
  }
}

bool MJPEGStream::isConnected() {
  return connected && client.connected();
}

float MJPEGStream::getAverageFPS() {
  if (frameCount == 0 || firstFrameTime == 0) {
    return 0.0;
  }
  
  unsigned long elapsed = millis() - firstFrameTime;
  if (elapsed == 0) {
    return 0.0;
  }
  
  return (float)frameCount / (elapsed / 1000.0);
}
