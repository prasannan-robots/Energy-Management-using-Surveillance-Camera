/**
 * Utility Functions Implementation
 * 
 * JPEG decoding, color conversion, WiFi helpers, and logging.
 */

#include "utils.h"
#include <WiFi.h>

// Note: For JPEG decoding, you'll need to install a JPEG library
// Recommended: "ESP32_JPEG" library or "jpegdec" library
// For now, providing placeholder with Arduino-compatible approach

/**
 * Decode JPEG to RGB565 frame
 * 
 * Note: This requires a JPEG decoder library like:
 * - ESP32_JPEG by Bodmer
 * - JPEGDEC library
 * Install via Arduino Library Manager
 */
bool decodeJPEG(uint8_t* jpegData, size_t jpegSize, uint16_t** rgbFrame, int* width, int* height) {
  // Placeholder implementation
  // For actual implementation, use a JPEG decoder library:
  
  /*
  Example using JPEGDEC library:
  
  #include <JPEGDEC.h>
  
  JPEGDEC jpeg;
  
  // Open JPEG from memory
  if (jpeg.openRAM(jpegData, jpegSize, nullptr)) {
    *width = jpeg.getWidth();
    *height = jpeg.getHeight();
    
    // Allocate RGB565 buffer
    *rgbFrame = (uint16_t*)malloc((*width) * (*height) * sizeof(uint16_t));
    if (*rgbFrame == nullptr) {
      return false;
    }
    
    // Decode to RGB565
    jpeg.setPixelType(RGB565_BIG_ENDIAN);
    jpeg.decode(0, 0, 0);
    jpeg.close();
    
    return true;
  }
  
  return false;
  */
  
  // For now, return false and log
  Serial.println("⚠ JPEG decoding not implemented - install JPEGDEC library");
  Serial.println("   Library: JPEGDEC by Larry Bank");
  Serial.println("   Arduino Library Manager: Search for 'JPEGDEC'");
  
  // Create dummy frame for testing (320x240 black screen)
  *width = 320;
  *height = 240;
  *rgbFrame = (uint16_t*)malloc((*width) * (*height) * sizeof(uint16_t));
  if (*rgbFrame) {
    memset(*rgbFrame, 0, (*width) * (*height) * sizeof(uint16_t));
    return true;
  }
  
  return false;
}

/**
 * Decode JPEG to RGB888 frame
 */
bool decodeJPEGToRGB888(uint8_t* jpegData, size_t jpegSize, uint8_t** rgbFrame, int* width, int* height) {
  // Similar to RGB565 version but output RGB888
  // Use JPEGDEC library with RGB888 format
  
  Serial.println("⚠ JPEG decoding to RGB888 not implemented");
  return false;
}

/**
 * Resize frame using nearest neighbor
 */
uint16_t* resizeFrame(uint16_t* frame, int srcWidth, int srcHeight, int dstWidth, int dstHeight) {
  if (srcWidth == dstWidth && srcHeight == dstHeight) {
    // No resize needed
    return frame;
  }
  
  uint16_t* resized = (uint16_t*)malloc(dstWidth * dstHeight * sizeof(uint16_t));
  if (!resized) {
    Serial.println("ERROR: Failed to allocate resize buffer");
    return nullptr;
  }
  
  float xRatio = (float)srcWidth / dstWidth;
  float yRatio = (float)srcHeight / dstHeight;
  
  for (int y = 0; y < dstHeight; y++) {
    for (int x = 0; x < dstWidth; x++) {
      int srcX = (int)(x * xRatio);
      int srcY = (int)(y * yRatio);
      resized[y * dstWidth + x] = frame[srcY * srcWidth + srcX];
    }
  }
  
  return resized;
}

/**
 * Convert RGB565 to RGB888
 */
void convertRGB565toRGB888(uint16_t* rgb565, uint8_t* rgb888, int width, int height) {
  for (int i = 0; i < width * height; i++) {
    uint16_t pixel = rgb565[i];
    rgb565to888(pixel, &rgb888[i * 3], &rgb888[i * 3 + 1], &rgb888[i * 3 + 2]);
  }
}

/**
 * Convert RGB888 to RGB565
 */
void convertRGB888toRGB565(uint8_t* rgb888, uint16_t* rgb565, int width, int height) {
  for (int i = 0; i < width * height; i++) {
    rgb565[i] = rgb888to565(rgb888[i * 3], rgb888[i * 3 + 1], rgb888[i * 3 + 2]);
  }
}

/**
 * Convert RGB888 to RGB565
 */
uint16_t rgb888to565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/**
 * Convert RGB565 to RGB888
 */
void rgb565to888(uint16_t rgb565, uint8_t* r, uint8_t* g, uint8_t* b) {
  *r = ((rgb565 >> 11) & 0x1F) << 3;
  *g = ((rgb565 >> 5) & 0x3F) << 2;
  *b = (rgb565 & 0x1F) << 3;
}

/**
 * Connect to WiFi network
 */
bool connectWiFi(const char* ssid, const char* password, int timeoutSeconds) {
  Serial.printf("Connecting to WiFi: %s\n", ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < timeoutSeconds) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✓ WiFi connected");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  }
  
  Serial.println("✗ WiFi connection failed");
  return false;
}

/**
 * Check WiFi connection status
 */
bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

/**
 * Get WiFi status as string
 */
String getWiFiStatus() {
  switch (WiFi.status()) {
    case WL_CONNECTED:
      return "Connected";
    case WL_NO_SSID_AVAIL:
      return "SSID not available";
    case WL_CONNECT_FAILED:
      return "Connection failed";
    case WL_CONNECTION_LOST:
      return "Connection lost";
    case WL_DISCONNECTED:
      return "Disconnected";
    default:
      return "Unknown";
  }
}

/**
 * Log info message
 */
void logInfo(const char* message) {
  Serial.printf("[INFO] %s\n", message);
}

/**
 * Log warning message
 */
void logWarning(const char* message) {
  Serial.printf("[WARN] %s\n", message);
}

/**
 * Log error message
 */
void logError(const char* message) {
  Serial.printf("[ERROR] %s\n", message);
}

/**
 * Log debug message
 */
void logDebug(const char* message) {
  Serial.printf("[DEBUG] %s\n", message);
}

/**
 * Print memory information
 */
void printMemoryInfo() {
  Serial.println("--- Memory Info ---");
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
  Serial.printf("Largest Free Block: %d bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
  Serial.println("-------------------");
}

/**
 * Get free memory
 */
size_t getFreeMemory() {
  return ESP.getFreeHeap();
}

/**
 * Sanitize string (remove non-printable characters)
 */
void sanitizeString(char* str, size_t maxLen) {
  for (size_t i = 0; i < maxLen && str[i] != '\0'; i++) {
    if (str[i] < 32 || str[i] > 126) {
      str[i] = '_';
    }
  }
}

/**
 * Validate IP address format
 */
bool validateIPAddress(const char* ip) {
  int parts = 0;
  int value = 0;
  bool hasDigit = false;
  
  for (int i = 0; ip[i] != '\0'; i++) {
    if (ip[i] >= '0' && ip[i] <= '9') {
      value = value * 10 + (ip[i] - '0');
      hasDigit = true;
      
      if (value > 255) {
        return false;
      }
    } else if (ip[i] == '.') {
      if (!hasDigit) {
        return false;
      }
      parts++;
      value = 0;
      hasDigit = false;
    } else {
      return false;
    }
  }
  
  if (!hasDigit) {
    return false;
  }
  
  return parts == 3;
}

/**
 * Validate GPIO pin number (ESP32 safe pins)
 */
bool validateGPIOPin(int pin) {
  // ESP32 safe GPIO pins for general use
  // Avoid: 0, 1 (UART), 6-11 (Flash), 16 (PSRAM on some boards)
  
  const int safePins[] = {2, 4, 5, 12, 13, 14, 15, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
  const int numSafePins = sizeof(safePins) / sizeof(safePins[0]);
  
  for (int i = 0; i < numSafePins; i++) {
    if (pin == safePins[i]) {
      return true;
    }
  }
  
  return false;
}
