/**
 * Utility Functions Header
 * 
 * JPEG decoding, frame processing, WiFi helpers, and logging.
 */

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// JPEG decoding
bool decodeJPEG(uint8_t* jpegData, size_t jpegSize, uint16_t** rgbFrame, int* width, int* height);
bool decodeJPEGToRGB888(uint8_t* jpegData, size_t jpegSize, uint8_t** rgbFrame, int* width, int* height);

// Frame processing
uint16_t* resizeFrame(uint16_t* frame, int srcWidth, int srcHeight, int dstWidth, int dstHeight);
void convertRGB565toRGB888(uint16_t* rgb565, uint8_t* rgb888, int width, int height);
void convertRGB888toRGB565(uint8_t* rgb888, uint16_t* rgb565, int width, int height);

// Color conversion helpers
uint16_t rgb888to565(uint8_t r, uint8_t g, uint8_t b);
void rgb565to888(uint16_t rgb565, uint8_t* r, uint8_t* g, uint8_t* b);

// WiFi helpers
bool connectWiFi(const char* ssid, const char* password, int timeoutSeconds = 30);
bool isWiFiConnected();
String getWiFiStatus();

// Logging
void logInfo(const char* message);
void logWarning(const char* message);
void logError(const char* message);
void logDebug(const char* message);

// Memory helpers
void printMemoryInfo();
size_t getFreeMemory();

// String helpers
void sanitizeString(char* str, size_t maxLen);
bool validateIPAddress(const char* ip);
bool validateGPIOPin(int pin);

#endif // UTILS_H
