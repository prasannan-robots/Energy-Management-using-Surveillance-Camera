/**
 * Simple Motion Detector Header
 * 
 * Lightweight motion detection using frame differencing
 * No AI/ML required - just compares consecutive frames
 */

#ifndef MOTION_DETECTOR_H
#define MOTION_DETECTOR_H

#include <Arduino.h>
#include <vector>
#include "config.h"

/**
 * Motion Detection Result
 */
struct MotionBlob {
  int x;
  int y;
  int width;
  int height;
  float intensity;  // Motion intensity (0.0 - 1.0)
};

/**
 * Simple Motion Detector Class
 */
class MotionDetector {
public:
  MotionDetector();
  ~MotionDetector();
  
  // Initialize detector
  bool begin(int frameWidth, int frameHeight);
  
  // Process new frame and detect motion
  std::vector<MotionBlob> detectMotion(uint8_t* currentFrame, size_t frameSize);
  
  // Configure sensitivity (0.0 - 1.0, default 0.3)
  void setSensitivity(float sens) { sensitivity = sens; }
  
  // Configure minimum blob size (default 20x20 pixels)
  void setMinBlobSize(int minSize) { minBlobSize = minSize; }
  
  // Reset (clear previous frame)
  void reset();
  
private:
  uint8_t* previousFrame;
  size_t previousFrameSize;
  int width;
  int height;
  float sensitivity;
  int minBlobSize;
  bool initialized;
  
  // Helper: Calculate difference between frames
  float calculateFrameDifference(uint8_t* frame1, uint8_t* frame2, size_t size);
  
  // Helper: Find motion regions
  std::vector<MotionBlob> findMotionBlobs(uint8_t* diffMap, int w, int h);
};

#endif
