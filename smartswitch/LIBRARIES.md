# Libraries Required for Smart Zone Switch

## Required Libraries

Install these via Arduino IDE Library Manager (**Tools → Manage Libraries**):

### 1. ESPAsyncWebServer
- **Author:** me-no-dev
- **Version:** Latest
- **Purpose:** Async HTTP server for web interface
- **Installation:**
  - Not always in Library Manager
  - Manual install from GitHub:
    1. Download: https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip
    2. Arduino IDE → Sketch → Include Library → Add .ZIP Library

### 2. AsyncTCP
- **Author:** me-no-dev
- **Version:** Latest
- **Purpose:** Required dependency for ESPAsyncWebServer
- **Installation:**
  - Download: https://github.com/me-no-dev/AsyncTCP/archive/master.zip
  - Arduino IDE → Sketch → Include Library → Add .ZIP Library

### 3. ArduinoJson
- **Author:** Benoit Blanchon
- **Version:** 7.x (latest)
- **Purpose:** JSON parsing and serialization for config files
- **Installation:** Library Manager → Search "ArduinoJson" → Install

---

## Optional Libraries

### 4. JPEGDEC
- **Author:** Larry Bank
- **Version:** Latest
- **Purpose:** JPEG decoding for MJPEG stream frames
- **Installation:** Library Manager → Search "JPEGDEC" → Install
- **Note:** System uses placeholder if not installed

### 5. TensorFlow Lite for Microcontrollers
- **Purpose:** AI person detection
- **Installation:**
  - Library Manager → Search "TensorFlow Lite Micro"
  - **OR** Use Edge Impulse Arduino library (easier)
- **Note:** System falls back to motion detection if not available

---

## Manual Installation Instructions

If libraries are not in Library Manager:

### Method 1: ZIP Installation
1. Download library ZIP from GitHub
2. Arduino IDE → **Sketch → Include Library → Add .ZIP Library**
3. Select downloaded ZIP
4. Restart Arduino IDE

### Method 2: Manual Copy
1. Download library
2. Extract to: `~/Documents/Arduino/libraries/`
   - Windows: `C:\Users\<username>\Documents\Arduino\libraries\`
   - Mac: `/Users/<username>/Documents/Arduino/libraries/`
   - Linux: `/home/<username>/Arduino/libraries/`
3. Restart Arduino IDE

---

## Verification

After installing, verify libraries are available:

1. Arduino IDE → **Sketch → Include Library**
2. Check for:
   - ESPAsyncWebServer
   - AsyncTCP
   - ArduinoJson

---

## Library Links

- **ESPAsyncWebServer:** https://github.com/me-no-dev/ESPAsyncWebServer
- **AsyncTCP:** https://github.com/me-no-dev/AsyncTCP
- **ArduinoJson:** https://arduinojson.org/
- **JPEGDEC:** https://github.com/bitbank2/JPEGDEC
- **TensorFlow Lite Micro:** https://www.tensorflow.org/lite/microcontrollers

---

## Troubleshooting

### "Library not found" during compilation
- Ensure library is installed in correct folder
- Restart Arduino IDE
- Check library name matches include statement

### Version conflicts
- Update all libraries to latest versions
- Check ESP32 board package version (3.1.1+)

### Memory issues
- Some libraries require PSRAM
- Select partition scheme with sufficient space
- Reduce tensor arena size if using TFLite

---

**All required libraries must be installed before uploading code!**
