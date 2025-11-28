/**
 * TensorFlow Lite Person Detector Header
 * 
 * Loads and runs quantized TFLite model for person detection.
 * Outputs bounding boxes with confidence scores.
 */

#ifndef TFLITE_DETECTOR_H
#define TFLITE_DETECTOR_H

#include <Arduino.h>
#include <vector>

// Detection result structure
struct Detection {
  float x;          // Bounding box top-left X (normalized 0-1)
  float y;          // Bounding box top-left Y (normalized 0-1)
  float width;      // Bounding box width (normalized 0-1)
  float height;     // Bounding box height (normalized 0-1)
  float confidence; // Detection confidence (0-1)
  int classId;      // Class ID (0 = person for single-class models)
};

/**
 * TFLite Detector Class
 * 
 * Note: This is a placeholder for TFLite integration.
 * Full implementation requires TensorFlow Lite for Microcontrollers library.
 * 
 * To integrate:
 * 1. Include tensorflow/lite/micro headers
 * 2. Load model data from SPIFFS or embedded array
 * 3. Initialize interpreter with appropriate tensor arena
 * 4. Implement input preprocessing (resize, normalize)
 * 5. Run inference and parse output tensors
 */
class TFLiteDetector {
public:
  TFLiteDetector();
  ~TFLiteDetector();
  
  // Initialize model (load from SPIFFS or embedded data)
  bool begin(const char* modelPath = "/model.tflite");
  
  // Run detection on RGB565 frame
  std::vector<Detection> detect(uint16_t* frame, int width, int height);
  
  // Run detection on RGB888 frame
  std::vector<Detection> detectRGB(uint8_t* frame, int width, int height);
  
  // Check if model is loaded
  bool isInitialized() { return initialized; }
  
  // Get model info
  void getModelInfo(char* buffer, size_t bufferSize);
  
  // Get last inference time
  unsigned long getLastInferenceTime() { return lastInferenceTime; }
  
private:
  bool initialized;
  unsigned long lastInferenceTime;
  
  // TFLite components (placeholders - implement with actual TFLite library)
  void* interpreter;        // TfLiteInterpreter pointer
  void* model;              // TfLiteModel pointer
  uint8_t* tensorArena;     // Memory arena for tensors
  size_t tensorArenaSize;
  
  // Model parameters
  int inputWidth;
  int inputHeight;
  int inputChannels;
  float detectionThreshold;
  
  // Private methods
  bool loadModel(const char* path);
  bool allocateTensors();
  void preprocessFrame(uint16_t* frame, int width, int height);
  void preprocessFrameRGB(uint8_t* frame, int width, int height);
  std::vector<Detection> parseOutputTensors();
  
  // Fallback: simple motion detection if TFLite fails
  std::vector<Detection> motionDetection(uint16_t* frame, int width, int height);
  uint16_t* previousFrame;
  int previousWidth;
  int previousHeight;
};

#endif // TFLITE_DETECTOR_H
