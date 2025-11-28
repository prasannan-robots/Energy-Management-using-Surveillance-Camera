# ESP32-CAM Smart Zone Switch
## Complete Setup and Usage Guide

---

## 📋 Table of Contents
1. [Hardware Requirements](#hardware-requirements)
2. [Software Requirements](#software-requirements)
3. [Arduino IDE Setup](#arduino-ide-setup)
4. [Library Installation](#library-installation)
5. [Hardware Wiring](#hardware-wiring)
6. [Uploading the Code](#uploading-the-code)
7. [SPIFFS Upload](#spiffs-upload)
8. [First Boot Configuration](#first-boot-configuration)
9. [Using the Web Interface](#using-the-web-interface)
10. [Troubleshooting](#troubleshooting)
11. [Testing Checklist](#testing-checklist)

---

## 🔧 Hardware Requirements

### ESP32-CAM Smart Switch Node
- **ESP32-CAM board** (AI-Thinker or similar)
- **USB-to-Serial programmer** (FTDI adapter or ESP32-CAM-MB)
- **5V power supply** (2A minimum recommended)
- **Dual-channel relay module** (5V, opto-isolated recommended)
  - Example: SRD-05VDC-SL-C relay module
- **Jumper wires**
- **Breadboard** (optional, for prototyping)

### CCTV Node (Pre-built)
- ESP32-CAM running [s60sc/ESP32-CAM_MJPEG2SD](https://github.com/s60sc/ESP32-CAM_MJPEG2SD)
- Configured to stream MJPEG at `http://<IP>:81/stream`
- Recommended resolution: QVGA (320x240)

### Load Devices (What the Relays Control)
- LED lights, desk lamps (AC or DC)
- Fans or ventilation systems
- **⚠️ SAFETY WARNING:** If controlling 220V AC loads, use properly rated relays with isolation!

---

## 💻 Software Requirements

### Arduino IDE
- **Arduino IDE 2.x** or **1.8.19+**
- Download: https://www.arduino.cc/en/software

### ESP32 Board Support
- **ESP32 Arduino Core v3.1.1+**
- Installation instructions below

---

## 🛠️ Arduino IDE Setup

### Step 1: Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. In **Additional Board Manager URLs**, add:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for **"ESP32"**
6. Install **"esp32 by Espressif Systems"** (version 3.1.1 or later)

### Step 2: Select Board

1. Go to **Tools → Board → ESP32 Arduino**
2. Select **"AI Thinker ESP32-CAM"**

### Step 3: Configure Board Settings

Set the following in **Tools** menu:
- **Upload Speed:** 115200
- **Flash Frequency:** 40MHz
- **Flash Mode:** QIO
- **Partition Scheme:** "Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)"
  - *Important:* Select a partition scheme with SPIFFS for web files
- **Core Debug Level:** None (or "Info" for debugging)
- **Erase All Flash Before Sketch Upload:** No (unless starting fresh)

---

## 📚 Library Installation

Install these libraries via **Tools → Manage Libraries** or **Sketch → Include Library → Manage Libraries**:

### Required Libraries

1. **ESPAsyncWebServer** (by me-no-dev)
   - Search: "ESPAsyncWebServer"
   - GitHub: https://github.com/me-no-dev/ESPAsyncWebServer
   - *Note:* May need to install manually via ZIP if not in library manager

2. **AsyncTCP** (by me-no-dev)
   - Required for ESPAsyncWebServer
   - GitHub: https://github.com/me-no-dev/AsyncTCP

3. **ArduinoJson** (by Benoit Blanchon)
   - Search: "ArduinoJson"
   - Install version **7.x** (latest)

4. **JPEGDEC** (by Larry Bank) - *Optional but Recommended*
   - Search: "JPEGDEC"
   - Used for JPEG decoding
   - If not installed, system will use placeholder frames

### Optional: TensorFlow Lite for Microcontrollers
- For full AI person detection
- Installation:
  ```
  Tools → Manage Libraries → Search "TensorFlow Lite Micro"
  ```
- **Note:** The project includes motion detection fallback if TFLite is not available

---

## 🔌 Hardware Wiring

### ESP32-CAM to Relay Module

```
ESP32-CAM GPIO 12  →  Relay Module IN1
ESP32-CAM GPIO 13  →  Relay Module IN2
ESP32-CAM GND      →  Relay Module GND
5V Power Supply    →  Relay Module VCC
5V Power Supply    →  ESP32-CAM 5V pin
```

### Relay Module to Load (Example: LED Light)

**Relay 1 (IN1):**
```
Load Power Supply (+) → Relay1 COM (Common)
Relay1 NO (Normally Open) → Load Device (+)
Load Device (-) → Power Supply (-)
```

**⚠️ Safety Notes:**
- Use opto-isolated relay modules to protect ESP32
- If controlling 220V AC: Use relays rated for AC switching (10A minimum)
- Never connect ESP32 directly to AC power
- Follow local electrical codes

### USB-to-Serial Programming Connection

For uploading code via FTDI adapter:
```
FTDI TX   → ESP32-CAM RX (U0RXD)
FTDI RX   → ESP32-CAM TX (U0TXD)
FTDI GND  → ESP32-CAM GND
FTDI 5V   → ESP32-CAM 5V

To enter programming mode:
  Connect GPIO 0 to GND
  Press RESET button
  Upload code
  Disconnect GPIO 0 from GND
  Press RESET button
```

*Alternative:* Use ESP32-CAM-MB programmer board (simpler, plug-and-play)

---

## 📤 Uploading the Code

### Step 1: Open Project

1. Open `SmartSwitch.ino` in Arduino IDE
2. Ensure all `.cpp` and `.h` files are in the same folder

### Step 2: Configure WiFi (Optional)

Edit `config.cpp` or set via web UI after first boot:
- Default AP: `ESP32_SmartSwitch_Setup` / Password: `12345678`

### Step 3: Verify Code

Click **✓ Verify** button to compile
- Expected output: "Done compiling"
- Note any library errors and install missing libraries

### Step 4: Upload to ESP32

1. Connect ESP32-CAM via USB programmer
2. Put ESP32-CAM in programming mode (GPIO 0 to GND, press RESET)
3. Click **→ Upload** button
4. Wait for upload to complete (~30 seconds)
5. Remove GPIO 0 connection and press RESET

### Step 5: Verify Upload

1. Open **Tools → Serial Monitor** (115200 baud)
2. Press RESET button on ESP32-CAM
3. You should see initialization messages:
   ```
   === ESP32-CAM Smart Zone Switch ===
   ✓ SPIFFS initialized
   ✓ Configuration loaded
   ✓ WiFi connected
   IP Address: 192.168.x.x
   ✓ Web server started
   ```

---

## 📁 SPIFFS Upload

The web interface files must be uploaded to SPIFFS.

### Step 1: Install SPIFFS Upload Tool

#### For Arduino IDE 2.x:
1. Download **arduino-esp32fs-plugin** from:
   https://github.com/me-no-dev/arduino-esp32fs-plugin/releases
2. Extract to `~/Documents/Arduino/tools/`
3. Restart Arduino IDE

#### For Arduino IDE 1.8.x:
1. Download ESP32FS plugin ZIP
2. Extract to `Arduino/tools/ESP32FS/tool/esp32fs.jar`
3. Restart Arduino IDE

### Step 2: Prepare Data Folder

Ensure your project structure is:
```
SmartSwitch/
├── SmartSwitch.ino
├── config.cpp/h
├── mjpeg_stream.cpp/h
├── ...
└── data/              ← Must have this folder
    ├── index.html
    ├── app.js
    ├── config.json
    └── zones.json
```

### Step 3: Upload SPIFFS

1. In Arduino IDE: **Tools → ESP32 Sketch Data Upload**
2. Wait for upload to complete
3. Press RESET button on ESP32-CAM

---

## 🚀 First Boot Configuration

### Step 1: Connect to AP Mode

If no WiFi configured, ESP32 starts in AP mode:
1. Connect to WiFi network: `ESP32_SmartSwitch_Setup`
2. Password: `12345678`
3. Open browser: `http://192.168.4.1`

### Step 2: Configure WiFi

1. Go to **Settings** tab
2. Enter your WiFi SSID and password
3. Click **Save Settings**
4. ESP32 will reboot and connect to your WiFi

### Step 3: Find ESP32 IP Address

Option A: Check Serial Monitor (recommended)
```
WiFi connected
IP Address: 192.168.1.100
```

Option B: Check your router's DHCP client list

### Step 4: Access Web Interface

Open browser: `http://<ESP32_IP>/`
Example: `http://192.168.1.100/`

---

## 🌐 Using the Web Interface

### Live Feed & Zone Drawing

1. **Canvas Area:**
   - Shows live feed placeholder (MJPEG stream will display here)
   - Zones overlaid as colored rectangles
   - Active zones highlighted in green

2. **Drawing Zones:**
   - Click **"Draw Zone"** button
   - Click and drag on canvas to create rectangle
   - Configure zone in popup modal:
     - **Name:** Descriptive name (e.g., "Desk 1")
     - **Relay Pins:** GPIO numbers (e.g., `12,13`)
     - **Timeout:** Seconds to keep relay active after detection
   - Click **Save Zone**

3. **Editing Zones:**
   - Click directly on a zone in the canvas
   - OR use **Edit** button in Zone List
   - Modify settings and save

### Zones Tab

- View all configured zones
- Each zone shows:
  - Name and position
  - Assigned relay GPIOs
  - Timeout setting
  - Active status (green when person detected)
- Actions: Edit, Delete

### Relays Tab

- View all relay states
- Manual control: Toggle relays ON/OFF
- **Emergency Stop:** Immediately disables ALL relays

### Settings Tab

- **CCTV IP Address:** IP of your CCTV camera
- **CCTV Port:** Port number (default: 81)
- **Detection Threshold:** Confidence level for person detection (0.3-0.9)
- **Global Timeout:** Default timeout for new zones (1-30 seconds)
- **Test Connection:** Verify CCTV stream is reachable

### Statistics Tab

- Total detections across all zones
- Per-zone detection counts
- Reset statistics button

---

## 🐛 Troubleshooting

### Problem: ESP32 won't enter programming mode

**Solution:**
- Ensure GPIO 0 is connected to GND during upload
- Press and hold RESET button, then release
- Some boards require holding BOOT button while pressing RESET

### Problem: Upload fails with "Timed out waiting for packet header"

**Solution:**
- Check USB cable (use data cable, not charge-only)
- Reduce upload speed: Tools → Upload Speed → 115200
- Try different USB port
- Ensure GPIO 0 is grounded

### Problem: SPIFFS upload fails

**Solution:**
- Ensure partition scheme includes SPIFFS
- Re-select board and partition scheme
- Close Serial Monitor before uploading

### Problem: Web UI not loading

**Solution:**
- Verify SPIFFS upload completed successfully
- Check Serial Monitor for error messages
- Try accessing: `http://<IP>/index.html` directly
- Re-upload SPIFFS data

### Problem: WiFi not connecting

**Solution:**
- Check SSID and password in config
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check router compatibility (some enterprise WiFi doesn't work)
- Use AP mode to reconfigure

### Problem: MJPEG stream not displaying

**Solution:**
- Verify CCTV node is running and streaming
- Test stream in browser: `http://<CCTV_IP>:81/stream`
- Check CCTV IP address in Settings
- Click "Test Connection" in Settings tab
- Check same WiFi network

### Problem: Relay not activating

**Solution:**
- Check relay module wiring (IN1, IN2, GND)
- Verify GPIO pins are correct (not 0, 1, 6-11)
- Test relay manually in Relays tab
- Check relay module power supply (5V)
- Some relay modules are active LOW - check `relayActiveHigh` setting

### Problem: Person detection not working

**Solution:**
- TFLite model may not be loaded - system falls back to motion detection
- Check Serial Monitor for TFLite initialization messages
- Ensure PSRAM is available (check "Free PSRAM" in System Info)
- Lower detection threshold in Settings
- Verify MJPEG stream is providing frames

### Problem: Out of memory errors

**Solution:**
- Reduce frame buffer size in code
- Select partition scheme with more RAM
- Disable debug output: Tools → Core Debug Level → None
- Check for memory leaks in Serial Monitor

### Problem: Watchdog timer triggers

**Solution:**
- MJPEG stream may be dropping frames
- Check network stability
- Reduce frame rate on CCTV node
- Increase watchdog timeout in config

---

## ✅ Testing Checklist

Use this checklist to verify your system:

### Basic Functionality
- [ ] ESP32 boots without errors (check Serial Monitor)
- [ ] WiFi connects successfully
- [ ] Web UI accessible at `http://<ESP32_IP>/`
- [ ] All tabs load correctly (Zones, Relays, Settings, Statistics)

### MJPEG Stream
- [ ] CCTV stream URL configured correctly
- [ ] "Test Connection" button shows success
- [ ] Stream status shows "Connected"
- [ ] Frames display on canvas (or placeholder if no stream)

### Zone Drawing
- [ ] Can draw new zone by clicking and dragging
- [ ] Zone configuration modal opens
- [ ] Zone saves successfully
- [ ] Zone appears in Zone List
- [ ] Zone persists after ESP32 reboot

### Person Detection
- [ ] TFLite model loads (or motion detection fallback works)
- [ ] Detection count updates when person appears
- [ ] Bounding boxes shown on canvas (green rectangles)
- [ ] Zone activates (turns green) when person detected

### Relay Control
- [ ] Relay activates when person detected in zone
- [ ] Relay stays active during detection
- [ ] Relay deactivates after timeout expires
- [ ] Manual relay toggle works in Relays tab
- [ ] Emergency Stop disables all relays immediately

### Configuration Persistence
- [ ] Zones saved to SPIFFS
- [ ] Settings saved to SPIFFS
- [ ] Configuration loads correctly after reboot
- [ ] Statistics persist across reboots

### Safety Features
- [ ] Watchdog timer disables relays if stream lost
- [ ] All relays initialize to OFF on boot
- [ ] Emergency Stop button works
- [ ] Invalid GPIO pins rejected

### Advanced Features
- [ ] WebSocket updates work in real-time
- [ ] Multiple zones can control same relay (OR logic)
- [ ] Multiple relays can be assigned to one zone
- [ ] Statistics track detections per zone
- [ ] System info displays correctly

---

## 📊 Expected Performance

### Frame Processing
- **Target FPS:** 10 FPS minimum
- **Inference Time:** 100-300ms per frame (with TFLite)
- **Total Latency:** <1 second from detection to relay activation

### Memory Usage
- **Free Heap:** ~50-100 KB during operation
- **Free PSRAM:** ~3.5 MB (if 4MB PSRAM installed)

### WiFi Performance
- **Signal Strength:** -70 dBm or better recommended
- **Network Latency:** <50ms to CCTV node

---

## 🔗 Useful Links

- **ESP32-CAM MJPEG2SD (CCTV Node):** https://github.com/s60sc/ESP32-CAM_MJPEG2SD
- **ESP32 Arduino Core:** https://github.com/espressif/arduino-esp32
- **ESPAsyncWebServer:** https://github.com/me-no-dev/ESPAsyncWebServer
- **ArduinoJson:** https://arduinojson.org/
- **TensorFlow Lite for Microcontrollers:** https://www.tensorflow.org/lite/microcontrollers

---

## 📝 Next Steps

1. **Deploy your system:**
   - Mount ESP32-CAM Smart Switch near relay module
   - Position CCTV camera with good view of monitored area
   - Configure zones for desks, corridors, etc.

2. **Optimize detection:**
   - Adjust detection threshold for false positive rate
   - Fine-tune zone sizes and positions
   - Set appropriate timeouts per zone

3. **Integrate loads:**
   - Connect lights, fans, or other devices to relays
   - Test activation timing
   - Verify safety mechanisms

4. **Monitor and iterate:**
   - Use Statistics tab to analyze usage
   - Adjust zones based on detection patterns
   - Optimize for energy savings

---

## ⚠️ Important Safety Notes

1. **Electrical Safety:**
   - Never connect ESP32 directly to AC power
   - Use properly rated relays for your load
   - Follow local electrical codes
   - If unsure, consult a licensed electrician

2. **Relay Ratings:**
   - Check relay module specs (voltage, current)
   - Don't exceed rated load
   - Use opto-isolated relays for safety

3. **Watchdog Protection:**
   - System has built-in watchdog to disable relays if stream lost
   - Test watchdog by disconnecting CCTV node

4. **Emergency Stop:**
   - Familiarize yourself with Emergency Stop button
   - Consider adding physical emergency stop switch

---

## 🤝 Support

For issues or questions:
1. Check Serial Monitor for error messages
2. Review Troubleshooting section above
3. Check GitHub Issues (if project is public)
4. Verify all libraries are installed correctly

---

**End of Setup Guide**
