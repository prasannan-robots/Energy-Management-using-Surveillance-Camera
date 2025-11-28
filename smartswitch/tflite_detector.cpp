/**
 * TensorFlow Lite Person Detector Implementation
 * 
 * This is a framework for TFLite integration. Full implementation requires:
 * 1. TensorFlow Lite for Microcontrollers library
 * 2. Pre-trained quantized person detection model (.tflite file)
 * 3. Model-specific input/output tensor parsing
 * 
 * For now, includes a motion detection fallback for testing.
 */

#include "tflite_detector.h"
#include <LittleFS.h>

// Uncomment when TFLite library is available:
// #include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
// #include "tensorflow/lite/micro/system_setup.h"
// #include "tensorflow/lite/schema/schema_generated.h"

#define TENSOR_ARENA_SIZE (500 * 1024)  // 500KB for tensor operations
#define MIN_CONFIDENCE 0.5

TFLiteDetector::TFLiteDetector() {
  initialized = false;
  interpreter = nullptr;
  model = nullptr;
  tensorArena = nullptr;
  tensorArenaSize = TENSOR_ARENA_SIZE;
  inputWidth = 320;
  inputHeight = 240;
  inputChannels = 3;
  detectionThreshold = MIN_CONFIDENCE;
  lastInferenceTime = 0;
  previousFrame = nullptr;
  previousWidth = 0;
  previousHeight = 0;
}

TFLiteDetector::~TFLiteDetector() {
  if (tensorArena) {
    free(tensorArena);
  }
  if (previousFrame) {
    free(previousFrame);
  }
}

bool TFLiteDetector::begin(const char* modelPath) {
  Serial.println("Initializing TFLite detector...");
  
  // Check if PSRAM is available
  if (ESP.getFreePsram() < TENSOR_ARENA_SIZE) {
    Serial.println("⚠ WARNING: Insufficient PSRAM for TFLite");
    Serial.printf("   Available: %d bytes, Required: %d bytes\n", 
                 ESP.getFreePsram(), TENSOR_ARENA_SIZE);
    Serial.println("   TFLite disabled - will use motion detection fallback");
    return false;
  }
  
  // Load model from SPIFFS
  if (!loadModel(modelPath)) {
    Serial.println("⚠ Model loading failed - using fallback detection");
    return false;
  }
  
  // Allocate tensor arena
  if (!allocateTensors()) {
    Serial.println("⚠ Tensor allocation failed");
    return false;
  }
  
  initialized = true;
  Serial.println("✓ TFLite detector initialized successfully");
  return true;
}

bool TFLiteDetector::loadModel(const char* path) {
  // Check if model file exists
  if (!LittleFS.exists(path)) {
    Serial.printf("Model file not found: %s\n", path);
    return false;
  }
  
  File modelFile = LittleFS.open(path, "r");
  if (!modelFile) {
    Serial.println("ERROR: Failed to open model file");
    return false;
  }
  
  size_t modelSize = modelFile.size();
  Serial.printf("Model size: %d bytes\n", modelSize);
  
  // For actual TFLite implementation:
  // 1. Read model data into buffer
  // 2. Parse with tflite::GetModel()
  // 3. Verify model schema version
  // 4. Create interpreter with op resolver
  
  modelFile.close();
  
  // Placeholder: mark as loaded
  Serial.println("⚠ TFLite model loading not fully implemented");
  Serial.println("   Add TensorFlow Lite for Microcontrollers library for full support");
  
  return false; // Return false until fully implemented
}

bool TFLiteDetector::allocateTensors() {
  // Allocate tensor arena in PSRAM
  tensorArena = (uint8_t*)ps_malloc(tensorArenaSize);
  if (!tensorArena) {
    Serial.println("ERROR: Failed to allocate tensor arena");
    return false;
  }
  
  Serial.printf("Tensor arena allocated: %d bytes\n", tensorArenaSize);
  
  // For actual TFLite implementation:
  // 1. Create MicroInterpreter with model, op resolver, and tensor arena
  // 2. Call AllocateTensors()
  // 3. Get input/output tensor pointers
  
  return true;
}

std::vector<Detection> TFLiteDetector::detect(uint16_t* frame, int width, int height) {
  unsigned long startTime = millis();
  std::vector<Detection> detections;
  
  if (!initialized) {
    // Fallback to motion detection
    detections = motionDetection(frame, width, height);
    lastInferenceTime = millis() - startTime;
    return detections;
  }
  
  // Preprocess frame
  preprocessFrame(frame, width, height);
  
  // Run inference (placeholder)
  // For actual TFLite implementation:
  // 1. Copy preprocessed data to input tensor
  // 2. Call interpreter->Invoke()
  // 3. Read output tensors
  // 4. Parse bounding boxes and scores
  
  // Parse output
  detections = parseOutputTensors();
  
  lastInferenceTime = millis() - startTime;
  return detections;
}

std::vector<Detection> TFLiteDetector::detectRGB(uint8_t* frame, int width, int height) {
  unsigned long startTime = millis();
  std::vector<Detection> detections;
  
  if (!initialized) {
    lastInferenceTime = millis() - startTime;
    return detections;
  }
  
  // Preprocess RGB frame
  preprocessFrameRGB(frame, width, height);
  
  // Run inference and parse output
  detections = parseOutputTensors();
  
  lastInferenceTime = millis() - startTime;
  return detections;
}

void TFLiteDetector::preprocessFrame(uint16_t* frame, int width, int height) {
  // Convert RGB565 to RGB888 and normalize
  // Resize to model input size if needed
  // For MobileNet: normalize to [-1, 1] or [0, 1]
  
  // Placeholder implementation
}

void TFLiteDetector::preprocessFrameRGB(uint8_t* frame, int width, int height) {
  // Resize to model input size if needed
  // Normalize pixel values
  
  // Placeholder implementation
}

std::vector<Detection> TFLiteDetector::parseOutputTensors() {
  std::vector<Detection> detections;
  
  // For actual TFLite SSD model output parsing:
  // Output tensors typically:
  //   - Bounding boxes: [1, num_detections, 4] (y1, x1, y2, x2)
  //   - Classes: [1, num_detections]
  //   - Scores: [1, num_detections]
  //   - Num detections: [1]
  
  // Parse and filter:
  // 1. Iterate through detections
  // 2. Filter by confidence threshold
  // 3. Filter by class (person = 0 or 1 depending on model)
  // 4. Create Detection objects
  
  // Placeholder: return empty
  return detections;
}

/**
 * Fallback Motion Detection
 * Simple frame difference for testing without TFLite
 */
std::vector<Detection> TFLiteDetector::motionDetection(uint16_t* frame, int width, int height) {
  std::vector<Detection> detections;
  
  if (!previousFrame || previousWidth != width || previousHeight != height) {
    // First frame or size change - store and return
    if (previousFrame) free(previousFrame);
    previousFrame = (uint16_t*)malloc(width * height * sizeof(uint16_t));
    if (previousFrame) {
      memcpy(previousFrame, frame, width * height * sizeof(uint16_t));
      previousWidth = width;
      previousHeight = height;
    }
    return detections;
  }
  
  // Calculate frame difference
  const int blockSize = 16; // 16x16 blocks
  const int threshold = 2000; // Motion threshold per block
  
  for (int by = 0; by < height / blockSize; by++) {
    for (int bx = 0; bx < width / blockSize; bx++) {
      int diff = 0;
      
      // Calculate difference in this block
      for (int y = 0; y < blockSize && (by * blockSize + y) < height; y++) {
        for (int x = 0; x < blockSize && (bx * blockSize + x) < width; x++) {
          int idx = (by * blockSize + y) * width + (bx * blockSize + x);
          
          // Extract RGB components
          uint16_t p1 = frame[idx];
          uint16_t p2 = previousFrame[idx];
          
          int r1 = (p1 >> 11) & 0x1F;
          int g1 = (p1 >> 5) & 0x3F;
          int b1 = p1 & 0x1F;
          
          int r2 = (p2 >> 11) & 0x1F;
          int g2 = (p2 >> 5) & 0x3F;
          int b2 = p2 & 0x1F;
          
          diff += abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2);
        }
      }
      
      // If significant motion detected
      if (diff > threshold) {
        Detection det;
        det.x = (float)(bx * blockSize) / width;
        det.y = (float)(by * blockSize) / height;
        det.width = (float)blockSize / width;
        det.height = (float)blockSize / height;
        det.confidence = min(1.0f, (float)diff / (threshold * 3));
        det.classId = 0;
        
        detections.push_back(det);
      }
    }
  }
  
  // Merge nearby detections
  // (Simple implementation - just keep all for now)
  
  // Update previous frame
  memcpy(previousFrame, frame, width * height * sizeof(uint16_t));
  
  return detections;
}

void TFLiteDetector::getModelInfo(char* buffer, size_t bufferSize) {
  if (initialized) {
    snprintf(buffer, bufferSize, 
            "Model: Loaded, Input: %dx%dx%d, Arena: %d KB",
            inputWidth, inputHeight, inputChannels, tensorArenaSize / 1024);
  } else {
    snprintf(buffer, bufferSize, "Model: Not loaded (using motion detection fallback)");
  }
}
