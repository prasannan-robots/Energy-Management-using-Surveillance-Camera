# Quick Start Guide - ESP32-CAM Smart Zone Switch

⚡ **Get up and running in 30 minutes!**

---

## ⚙️ Prerequisites

- **Hardware:** ESP32-CAM, relay module, USB programmer
- **Software:** Arduino IDE 2.x installed
- **CCTV:** ESP32-CAM running MJPEG2SD streaming at port 81

---

## 🚀 5-Step Quick Setup

### Step 1: Install Arduino ESP32 Board (5 min)

1. Arduino IDE → **File → Preferences**
2. Add board URL:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. **Tools → Board → Boards Manager** → Install "esp32"
4. Select: **Tools → Board → AI Thinker ESP32-CAM**

### Step 2: Install Libraries (5 min)

Go to **Tools → Manage Libraries**, install:
- `ESPAsyncWebServer` (by me-no-dev)
- `AsyncTCP` (by me-no-dev)
- `ArduinoJson` (by Benoit Blanchon)
- `JPEGDEC` (by Larry Bank) - optional

*If ESPAsyncWebServer not in library manager, download from GitHub:*
- https://github.com/me-no-dev/ESPAsyncWebServer
- https://github.com/me-no-dev/AsyncTCP

### Step 3: Configure & Upload Code (10 min)

1. Open `SmartSwitch.ino`
2. Set board settings:
   - **Partition:** Minimal SPIFFS (1.9MB APP/190KB SPIFFS)
   - **Upload Speed:** 115200
3. Connect ESP32-CAM via USB programmer
4. Put in programming mode (GPIO 0 → GND, press RESET)
5. Click **Upload** (→)
6. Remove GPIO 0 connection, press RESET

### Step 4: Upload Web Files (5 min)

1. Ensure `data/` folder contains: `index.html`, `app.js`, `config.json`, `zones.json`
2. **Tools → ESP32 Sketch Data Upload**
3. Wait for completion
4. Press RESET

### Step 5: Connect & Configure (5 min)

1. Open Serial Monitor (115200 baud)
2. Note IP address shown after boot
3. Open browser: `http://<IP_ADDRESS>/`
4. Go to **Settings** tab:
   - Enter CCTV IP (e.g., 192.168.4.100)
   - Click **Test Connection**
   - Click **Save Settings**

---

## 🎯 First Zone Setup

1. **Draw Zone:**
   - Click **"Draw Zone"** button
   - Click and drag on canvas to create rectangle

2. **Configure:**
   - Name: "Desk 1"
   - Relay Pins: 12
   - Timeout: 5 seconds

3. **Test:**
   - Stand in front of CCTV camera
   - Watch zone turn green
   - Verify relay activates (check LED or connected device)

---

## 🔌 Wiring Quick Reference

```
ESP32-CAM GPIO 12 → Relay IN1
ESP32-CAM GPIO 13 → Relay IN2
ESP32-CAM GND     → Relay GND
5V Supply         → Relay VCC
5V Supply         → ESP32-CAM 5V
```

---

## ✅ Verify Everything Works

- [ ] Web UI loads at `http://<IP>/`
- [ ] Can draw and save zones
- [ ] CCTV stream status shows "Connected"
- [ ] Zone turns green when person detected
- [ ] Relay clicks/activates
- [ ] Settings persist after reboot

---

## 🐛 Quick Troubleshooting

| Problem | Solution |
|---------|----------|
| Upload fails | Ground GPIO 0 during upload |
| Web UI blank | Re-upload SPIFFS data |
| WiFi not connecting | Check SSID/password, use 2.4GHz |
| Stream not working | Verify CCTV IP, test in browser |
| Relay not activating | Check wiring, test manual toggle |

---

## 📚 Full Documentation

For detailed setup, see **README.md**

For TFLite model integration, see **MODEL_GUIDE.md**

---

## 🎉 You're Done!

Your Smart Zone Switch is ready. Enjoy automated energy management!

**Next:** Add more zones, adjust detection threshold, connect lights/fans.
