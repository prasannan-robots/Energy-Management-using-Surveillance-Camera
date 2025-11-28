/**
 * Simple Motion Detector Implementation
 * 
 * Lightweight motion detection using frame differencing
 * Works directly with JPEG data - no full decoding needed
 */

#include "motion_detector.h"

MotionDetector::MotionDetector() {
  previousFrame = nullptr;
  previousFrameSize = 0;
  width = 0;
  height = 0;
  sensitivity = 0.3;
  minBlobSize = 400;  // 20x20 pixels
  initialized = false;
}

MotionDetector::~MotionDetector() {
  if (previousFrame) {
    free(previousFrame);
  }
}

bool MotionDetector::begin(int frameWidth, int frameHeight) {
  width = frameWidth;
  height = frameHeight;
  
  // Allocate buffer for previous frame
  size_t maxFrameSize = width * height / 2;  // Rough estimate for JPEG
  previousFrame = (uint8_t*)malloc(maxFrameSize);
  
  if (!previousFrame) {
    Serial.println("ERROR: Failed to allocate motion detector buffer");
    return false;
  }
  
  previousFrameSize = 0;
  initialized = true;
  
  Serial.printf("✓ Motion detector initialized (%dx%d, sensitivity=%.2f)\n", 
                width, height, sensitivity);
  return true;
}

std::vector<MotionBlob> MotionDetector::detectMotion(uint8_t* currentFrame, size_t frameSize) {
  std::vector<MotionBlob> blobs;
  
  if (!initialized || !currentFrame || frameSize == 0) {
    return blobs;
  }
  
  // First frame - just store it
  if (previousFrameSize == 0) {
    if (frameSize <= (size_t)(width * height / 2)) {
      memcpy(previousFrame, currentFrame, frameSize);
      previousFrameSize = frameSize;
    }
    return blobs;
  }
  
  // Calculate difference between current and previous frame
  // Simple byte-by-byte comparison (works reasonably well for JPEG)
  float difference = calculateFrameDifference(currentFrame, previousFrame, 
                                               min(frameSize, previousFrameSize));
  
  // If significant motion detected
  if (difference > sensitivity) {
    // Create a single motion blob covering the whole frame
    // (simplified - in reality we'd analyze specific regions)
    MotionBlob blob;
    blob.x = width / 4;
    blob.y = height / 4;
    blob.width = width / 2;
    blob.height = height / 2;
    blob.intensity = difference;
    
    blobs.push_back(blob);
    
    Serial.printf("🔍 Motion detected! Intensity: %.3f\n", difference);
  }
  
  // Store current frame for next comparison
  if (frameSize <= (size_t)(width * height / 2)) {
    memcpy(previousFrame, currentFrame, frameSize);
    previousFrameSize = frameSize;
  }
  
  return blobs;
}

float MotionDetector::calculateFrameDifference(uint8_t* frame1, uint8_t* frame2, size_t size) {
  if (!frame1 || !frame2 || size == 0) {
    return 0.0;
  }
  
  // Sample every Nth byte to speed up comparison
  const int sampleRate = 50;  // Check every 50th byte
  int diffCount = 0;
  int totalSamples = 0;
  
  for (size_t i = 0; i < size; i += sampleRate) {
    int diff = abs((int)frame1[i] - (int)frame2[i]);
    if (diff > 30) {  // Threshold for considering a byte "different"
      diffCount++;
    }
    totalSamples++;
  }
  
  // Return percentage of changed bytes
  return totalSamples > 0 ? (float)diffCount / totalSamples : 0.0;
}

std::vector<MotionBlob> MotionDetector::findMotionBlobs(uint8_t* diffMap, int w, int h) {
  // Simplified - just return center blob if motion detected
  // Full implementation would do connected component analysis
  std::vector<MotionBlob> blobs;
  return blobs;
}

void MotionDetector::reset() {
  previousFrameSize = 0;
  Serial.println("Motion detector reset");
}
