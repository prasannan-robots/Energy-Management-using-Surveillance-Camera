# TensorFlow Lite Model Integration Guide

## Overview

The ESP32-CAM Smart Zone Switch supports TensorFlow Lite for Microcontrollers for AI-based person detection. This document explains how to obtain, prepare, and integrate a quantized person detection model.

---

## Model Requirements

### Technical Specifications

- **Format:** TensorFlow Lite (.tflite)
- **Quantization:** INT8 or UINT8 (required for ESP32)
- **Input Size:** 320×240×3 (QVGA RGB) recommended
  - Smaller models (e.g., 96×96) also work but less accurate
- **Output:** Bounding boxes + confidence scores
- **Model Size:** <2 MB (to fit in ESP32 flash/PSRAM)
- **Architecture:** MobileNetV2 SSD or similar lightweight model

### Supported Models

1. **MobileNet SSD Person Detector** (Recommended)
   - Pre-trained on COCO dataset
   - ~1.5 MB quantized
   - Good balance of speed and accuracy

2. **Edge Impulse Custom Model**
   - Train your own with Edge Impulse Studio
   - Optimized for ESP32
   - Export as quantized TFLite

3. **TinyYOLOv2 Quantized**
   - Smaller, faster
   - Lower accuracy
   - ~500 KB

---

## Option 1: Pre-trained Model from TensorFlow

### Download Pre-converted Model

TensorFlow provides pre-trained quantized models in their model zoo:

**MobileNet SSD V2 (COCO)**
- Source: https://www.tensorflow.org/lite/examples/object_detection/overview
- Download: `ssd_mobilenet_v2_quantized_coco.tflite`
- Size: ~4 MB (needs further optimization)

### Optimize for ESP32

Use TensorFlow Lite converter to reduce size:

```python
import tensorflow as tf

# Load the model
converter = tf.lite.TFLiteConverter.from_saved_model('path/to/saved_model')

# Enable optimizations
converter.optimizations = [tf.lite.Optimize.DEFAULT]

# Set input/output types to INT8
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

# Convert
tflite_model = converter.convert()

# Save
with open('person_detector_quantized.tflite', 'wb') as f:
    f.write(tflite_model)
```

### Resize Input (if needed)

To reduce to QVGA (320×240):

```python
converter.representative_dataset = representative_dataset_gen
converter.target_spec.supported_types = [tf.int8]
```

---

## Option 2: Train Custom Model with Edge Impulse

Edge Impulse provides an easy way to train and deploy models to ESP32.

### Steps:

1. **Create Edge Impulse Account**
   - Visit: https://www.edgeimpulse.com/
   - Sign up for free

2. **Create New Project**
   - Project type: Object Detection
   - Target: ESP32

3. **Collect Training Data**
   - Upload images of people in your environment
   - Use Edge Impulse's data acquisition tools
   - Minimum: 100 images per class

4. **Label Data**
   - Draw bounding boxes around people
   - Use Edge Impulse's labeling tool

5. **Train Model**
   - Select "MobileNetV2 SSD 320×320" or "FOMO" (faster)
   - Training time: ~5-10 minutes
   - Validate accuracy on test set

6. **Export for Arduino**
   - Go to: Deployment → Arduino Library
   - Select: "Quantized (int8)"
   - Download ZIP file

7. **Install Library**
   - Arduino IDE: Sketch → Include Library → Add .ZIP Library
   - Select downloaded ZIP

8. **Integrate**
   - Replace `tflite_detector.cpp` model loading code
   - Follow Edge Impulse's integration guide

---

## Option 3: Use Pre-built Person Detector (Placeholder)

For testing without TFLite, the system includes a **motion detection fallback**:

- Uses frame difference algorithm
- Detects motion in 16×16 pixel blocks
- No model file needed
- Lower accuracy but functional

---

## Integrating TFLite Model

### Step 1: Prepare Model File

1. Convert/download model as described above
2. Name file: `model.tflite`
3. Size should be <2 MB

### Step 2: Upload to SPIFFS

**Option A: Manual Upload**
```bash
# Create data/model.tflite in your sketch folder
cp person_detector.tflite smartswitch/data/model.tflite

# Upload via Arduino IDE: Tools → ESP32 Sketch Data Upload
```

**Option B: Embed in Code**

Convert `.tflite` to C array:
```bash
xxd -i model.tflite > model_data.h
```

Then include in `tflite_detector.cpp`:
```cpp
#include "model_data.h"
// Use model_tflite array instead of SPIFFS
```

### Step 3: Update Code

In `tflite_detector.cpp`, uncomment TFLite headers:

```cpp
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
```

### Step 4: Implement Model Loading

Replace placeholder in `loadModel()`:

```cpp
bool TFLiteDetector::loadModel(const char* path) {
    File modelFile = SPIFFS.open(path, "r");
    if (!modelFile) return false;
    
    size_t modelSize = modelFile.size();
    uint8_t* modelData = (uint8_t*)ps_malloc(modelSize);
    modelFile.read(modelData, modelSize);
    modelFile.close();
    
    model = tflite::GetModel(modelData);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.println("Model schema version mismatch!");
        return false;
    }
    
    // Create op resolver
    static tflite::MicroMutableOpResolver<10> resolver;
    resolver.AddQuantize();
    resolver.AddDequantize();
    resolver.AddConv2D();
    resolver.AddDepthwiseConv2D();
    resolver.AddReshape();
    resolver.AddSoftmax();
    // Add other ops as needed
    
    // Create interpreter
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensorArena, tensorArenaSize);
    interpreter = &static_interpreter;
    
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("Tensor allocation failed!");
        return false;
    }
    
    return true;
}
```

### Step 5: Implement Inference

Update `detect()` method:

```cpp
std::vector<Detection> TFLiteDetector::detect(uint16_t* frame, int width, int height) {
    std::vector<Detection> detections;
    
    // Get input tensor
    TfLiteTensor* input = interpreter->input(0);
    
    // Preprocess and copy frame to input tensor
    preprocessFrame(frame, width, height, input->data.int8);
    
    // Run inference
    if (interpreter->Invoke() != kTfLiteOk) {
        Serial.println("Inference failed!");
        return detections;
    }
    
    // Parse output tensors
    return parseOutputTensors();
}
```

### Step 6: Parse Output

Implement `parseOutputTensors()`:

```cpp
std::vector<Detection> TFLiteDetector::parseOutputTensors() {
    std::vector<Detection> detections;
    
    // For SSD models, output typically contains:
    // - Bounding boxes: tensor(0) - [1, num_detections, 4]
    // - Classes: tensor(1) - [1, num_detections]
    // - Scores: tensor(2) - [1, num_detections]
    // - Count: tensor(3) - [1]
    
    TfLiteTensor* boxes = interpreter->output(0);
    TfLiteTensor* classes = interpreter->output(1);
    TfLiteTensor* scores = interpreter->output(2);
    TfLiteTensor* count = interpreter->output(3);
    
    int numDetections = count->data.f[0];
    
    for (int i = 0; i < numDetections; i++) {
        float score = scores->data.f[i];
        int classId = classes->data.f[i];
        
        // Filter: only person class (usually 0 or 1)
        if (classId != 0 && classId != 1) continue;
        
        // Filter by confidence
        if (score < detectionThreshold) continue;
        
        Detection det;
        det.y = boxes->data.f[i * 4 + 0];
        det.x = boxes->data.f[i * 4 + 1];
        det.height = boxes->data.f[i * 4 + 2] - det.y;
        det.width = boxes->data.f[i * 4 + 3] - det.x;
        det.confidence = score;
        det.classId = classId;
        
        detections.push_back(det);
    }
    
    return detections;
}
```

---

## Model Performance Tuning

### Optimize Input Size

Smaller input = faster inference:
- **96×96:** ~50ms inference, lower accuracy
- **160×120:** ~100ms inference, good balance
- **320×240:** ~200ms inference, best accuracy

### Adjust Tensor Arena Size

In `tflite_detector.h`:
```cpp
#define TENSOR_ARENA_SIZE (400 * 1024)  // Reduce if memory limited
```

### Quantization Awareness

Ensure preprocessing matches quantization:
- INT8: Scale to [-128, 127]
- UINT8: Scale to [0, 255]

---

## Testing Model

### Verify Model Loads

Check Serial Monitor for:
```
✓ TFLite model loaded successfully
Model size: 1234567 bytes
Tensor arena allocated: 512000 bytes
```

### Test Inference Speed

Monitor Serial output:
```
✓ Detected 1 person(s) in frame (inference: 123ms)
```

Target: <200ms for acceptable latency

### Validate Detections

1. Stand in front of CCTV camera
2. Check web UI for green bounding boxes
3. Verify zones activate when you're detected
4. Adjust `detectionThreshold` if too many false positives

---

## Fallback: Motion Detection

If TFLite integration is too complex, the system includes motion detection:

**How it works:**
- Compares consecutive frames
- Detects changes in 16×16 blocks
- Creates bounding boxes around motion
- No model file needed

**Performance:**
- Fast (~20ms per frame)
- Works out-of-the-box
- Less accurate (triggers on any motion)

**To use:**
- Simply don't upload a `.tflite` file
- System automatically falls back to motion detection

---

## Recommended Models

### For Best Accuracy
**MobileNet SSD V2 (Quantized)**
- Size: ~1.5 MB
- Inference: ~150-200ms
- Accuracy: High

### For Best Speed
**FOMO (Edge Impulse)**
- Size: ~500 KB
- Inference: ~50-80ms
- Accuracy: Medium

### For Minimal Memory
**Custom Tiny Model**
- Size: <500 KB
- Inference: ~30-50ms
- Accuracy: Lower
- Train with Edge Impulse on your specific environment

---

## Model Sources

1. **TensorFlow Lite Model Garden**
   - https://www.tensorflow.org/lite/examples/object_detection/overview

2. **Edge Impulse Public Projects**
   - https://www.edgeimpulse.com/projects

3. **TensorFlow Hub**
   - https://tfhub.dev/s?deployment-format=lite&q=object%20detection

4. **PINTO Model Zoo**
   - https://github.com/PINTO0309/PINTO_model_zoo

---

## Troubleshooting

### Model Won't Load
- Check file size (<2 MB recommended)
- Verify SPIFFS upload successful
- Check Serial Monitor for errors
- Try embedded model instead of SPIFFS

### Out of Memory
- Reduce tensor arena size
- Use smaller input resolution
- Close Serial Monitor to free RAM
- Select partition with more PSRAM

### Slow Inference
- Reduce input size (e.g., 160×120 instead of 320×240)
- Use INT8 quantization
- Optimize model with TFLite converter
- Consider simpler model architecture

### No Detections
- Check detection threshold (lower = more sensitive)
- Verify preprocessing matches model training
- Test with motion detection fallback first
- Ensure lighting is adequate

---

## Next Steps

1. **Start with motion detection** to verify system works
2. **Try Edge Impulse** for easiest custom model training
3. **Optimize model** for your specific use case
4. **Fine-tune threshold** to balance false positives/negatives

---

**Model integration is optional - the system works with motion detection fallback!**
