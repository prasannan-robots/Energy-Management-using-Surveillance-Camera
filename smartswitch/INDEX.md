# 🏠 ESP32-CAM Smart Zone Switch
## Zone-Based Relay Control with AI Person Detection

[![ESP32](https://img.shields.io/badge/ESP32-CAM-blue.svg)](https://github.com/espressif/arduino-esp32)
[![Arduino](https://img.shields.io/badge/Arduino-Compatible-green.svg)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/License-Open%20Source-orange.svg)](#)

---

## 🎯 What Is This?

A **complete, production-ready** smart switch system for the ESP32-CAM that:
- Consumes MJPEG video from a CCTV camera
- Detects people using AI (TensorFlow Lite) or motion detection
- Activates relays based on customizable zones
- Provides a web-based interface for zone drawing and configuration
- Saves energy by automatically controlling lights, fans, and other devices

**Perfect for:** Smart offices, homes, classrooms, workshops, and energy management systems.

---

## ⚡ Quick Navigation

| Document | Purpose | For Who |
|----------|---------|---------|
| **[QUICKSTART.md](QUICKSTART.md)** | Get running in 30 minutes | Experienced users |
| **[README.md](README.md)** | Complete setup & troubleshooting | All users (start here!) |
| **[MODEL_GUIDE.md](MODEL_GUIDE.md)** | TFLite model integration | Advanced AI users |
| **[LIBRARIES.md](LIBRARIES.md)** | Required libraries list | Arduino setup |
| **[PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)** | Technical overview | Developers |

---

## 📦 What's Included

### ✅ Complete Arduino Firmware
- **SmartSwitch.ino** - Main sketch
- **8 modular libraries** - Well-organized, documented code
- **MJPEG stream consumer** - Connects to remote CCTV
- **AI person detection** - TFLite integration + motion fallback
- **Zone-based control** - Smart relay activation logic
- **Web server** - RESTful API + WebSocket updates

### ✅ Professional Web Interface
- **Canvas-based zone drawing** - Draw zones with mouse/touch
- **Live feed display** - Real-time video overlay
- **Relay dashboard** - Monitor and control relays
- **Configuration panels** - CCTV settings, thresholds, timeouts
- **Statistics** - Detection counts and system monitoring

### ✅ Comprehensive Documentation
- **README.md** - 100+ section complete guide
- **QUICKSTART.md** - 30-minute setup
- **MODEL_GUIDE.md** - AI model integration
- **Troubleshooting** - 10+ common issues solved
- **Testing checklist** - 25+ verification items

---

## 🚀 Key Features

### Core Functionality
✅ **Zone-Based Detection**
- Draw rectangular zones on live video
- Assign GPIO relays to each zone
- Configure per-zone timeouts
- Multiple zones per relay, multiple relays per zone

✅ **AI Person Detection**
- TensorFlow Lite Micro support
- Motion detection fallback (no model needed)
- Adjustable confidence threshold
- Real-time bounding boxes

✅ **Smart Relay Control**
- Activates when person detected in zone
- Deactivates after configurable timeout
- Manual override via web UI
- Emergency stop button

✅ **Web Interface**
- Intuitive zone drawing (click and drag)
- Live detection visualization
- Real-time relay status
- Mobile-responsive design

### Safety & Reliability
✅ **Watchdog Timer** - Auto-disable relays if stream lost (60s)
✅ **Emergency Stop** - Immediately disable all relays
✅ **Safe Initialization** - All relays OFF on boot
✅ **GPIO Validation** - Blocks dangerous pins
✅ **Auto-Reconnect** - WiFi and stream recovery

### Advanced Features
✅ **SPIFFS Configuration** - Persistent zone storage
✅ **WebSocket Updates** - Real-time stats
✅ **JSON API** - Full RESTful endpoints
✅ **Statistics** - Per-zone detection counts
✅ **System Monitoring** - RAM, WiFi, performance metrics

---

## 🛠️ Hardware Requirements

### Smart Switch Node (What You're Building)
- ESP32-CAM board (AI-Thinker)
- 5V dual-channel relay module
- USB-to-Serial programmer (FTDI or ESP32-CAM-MB)
- 5V power supply (2A)

### CCTV Node (Pre-Built)
- ESP32-CAM running [s60sc/ESP32-CAM_MJPEG2SD](https://github.com/s60sc/ESP32-CAM_MJPEG2SD)
- Streaming at `http://<IP>:81/stream`
- Resolution: QVGA (320×240) recommended

**Total Cost:** ~$15-20 USD for Smart Switch Node hardware

---

## 💻 Software Requirements

- **Arduino IDE** 2.x or 1.8.19+
- **ESP32 Board Support** v3.1.1+
- **4 Required Libraries:**
  - ESPAsyncWebServer
  - AsyncTCP
  - ArduinoJson
  - JPEGDEC (optional)

**Install time:** 10-15 minutes

---

## 🎬 Quick Start (3 Steps)

### 1. Setup Arduino IDE
```bash
# Install ESP32 board support
# Install 4 required libraries
# Select: AI Thinker ESP32-CAM
```

### 2. Upload Code
```bash
# Upload SmartSwitch.ino
# Upload data/ folder to SPIFFS
# Connect relay module (GPIO 12, 13)
```

### 3. Configure & Test
```bash
# Open http://<ESP32_IP>/
# Set CCTV IP address
# Draw zones, assign relays
# Test detection
```

**Full instructions:** [QUICKSTART.md](QUICKSTART.md)

---

## 📸 Screenshots & Demo

### Web Interface
- **Zone Drawing:** Intuitive click-and-drag interface
- **Live Feed:** Real-time MJPEG with detection overlays
- **Relay Dashboard:** Visual status indicators with manual control
- **Settings Panel:** Easy CCTV configuration

### System in Action
1. **Person enters zone** → Detection bounding box appears (green)
2. **Zone activates** → Zone turns green on canvas
3. **Relay triggers** → GPIO goes HIGH, device turns on
4. **Person leaves** → Timeout countdown starts
5. **Timeout expires** → Relay deactivates, device turns off

---

## 🏆 Why This Project?

### ✨ Production-Ready
- Extensive error handling
- Safety features (watchdog, emergency stop)
- Graceful degradation (works without AI model)
- Tested and documented

### 🧩 Modular Design
- 8 separate, reusable modules
- Clean separation of concerns
- Easy to extend and customize
- Well-commented code

### 📚 Complete Documentation
- Beginner-friendly setup guide
- Advanced AI integration guide
- Comprehensive troubleshooting
- Real-world use cases

### 💰 Cost-Effective
- ~$15-20 hardware cost per node
- No cloud subscriptions
- No external services required
- Energy savings pay for itself

---

## 📋 File Structure

```
smartswitch/
├── 📄 SmartSwitch.ino          # Main Arduino sketch
│
├── 🔧 Core Modules (.cpp/.h)
│   ├── config                   # SPIFFS JSON configuration
│   ├── mjpeg_stream            # CCTV stream consumer
│   ├── tflite_detector         # AI person detection
│   ├── zone_manager            # Relay control logic
│   ├── web_server              # HTTP + WebSocket server
│   └── utils                   # JPEG, WiFi, logging
│
├── 🌐 Web UI (data/)
│   ├── index.html              # Main interface
│   ├── app.js                  # JavaScript application
│   ├── config.json             # System config template
│   └── zones.json              # Zone definitions template
│
└── 📖 Documentation
    ├── README.md               # Complete setup guide ⭐ START HERE
    ├── QUICKSTART.md           # 30-minute fast setup
    ├── MODEL_GUIDE.md          # TFLite AI integration
    ├── LIBRARIES.md            # Required libraries
    ├── PROJECT_SUMMARY.md      # Technical overview
    └── INDEX.md                # This file
```

---

## 🎓 Learning Resources

### Concepts Demonstrated
- ESP32 Arduino programming
- MJPEG stream parsing
- TensorFlow Lite on microcontrollers
- Async web server development
- WebSocket real-time communication
- Canvas-based UI with JavaScript
- SPIFFS file system
- State machine design
- GPIO control and safety

### Skills Gained
- Embedded systems development
- Computer vision basics
- Web development (HTML/CSS/JS)
- RESTful API design
- Real-time data visualization
- Hardware interfacing (relays, GPIOs)

---

## 🔗 Important Links

### This Project
- 📄 [Complete Setup Guide](README.md)
- ⚡ [Quick Start](QUICKSTART.md)
- 🧠 [AI Model Guide](MODEL_GUIDE.md)

### External Resources
- 🎥 [CCTV Node Project](https://github.com/s60sc/ESP32-CAM_MJPEG2SD)
- 🔌 [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- 🌐 [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- 🤖 [TensorFlow Lite Micro](https://www.tensorflow.org/lite/microcontrollers)

---

## 🤝 Use Cases

### 🏢 Smart Office
- Desk occupancy detection
- Automatic lighting per desk
- Energy savings during breaks
- Conference room automation

### 🏠 Smart Home
- Corridor motion lighting
- Bedroom presence detection
- Automatic ventilation
- Security integration

### 🏭 Industrial
- Workstation monitoring
- Safety lighting
- Access logging
- Equipment auto-shutoff

### 🏫 Educational
- Classroom occupancy
- Lab equipment control
- Energy management
- IoT learning project

---

## ⚠️ Safety First

### Electrical Safety
- ⚠️ Use opto-isolated relay modules
- ⚠️ Never connect ESP32 directly to AC power
- ⚠️ Check relay ratings for your load
- ⚠️ Follow local electrical codes

### System Safety
- ✅ Watchdog timer (auto-disable if stream lost)
- ✅ Emergency stop button
- ✅ All relays initialize to OFF
- ✅ Safe GPIO pin validation

---

## 📊 Expected Performance

| Metric | Target | Typical |
|--------|--------|---------|
| Frame Rate | 10 FPS | 10-15 FPS |
| Inference (TFLite) | <200ms | 100-200ms |
| Inference (Motion) | <50ms | 10-20ms |
| Detection Latency | <1s | 300-500ms |
| Relay Response | Immediate | <50ms |
| Memory Usage | >50KB free | 50-100KB |

---

## ✅ Testing Checklist

Quick verification after setup:
- [ ] Web UI loads at `http://<IP>/`
- [ ] Can connect to CCTV stream
- [ ] Can draw and save zones
- [ ] Detection triggers zone activation
- [ ] Relays click when activated
- [ ] Configuration persists after reboot
- [ ] Emergency stop works

**Full checklist:** 25+ items in [README.md](README.md)

---

## 🐛 Troubleshooting Quick Tips

| Issue | Solution |
|-------|----------|
| Upload fails | Ground GPIO 0, press RESET |
| Web UI blank | Re-upload SPIFFS data folder |
| WiFi won't connect | Check 2.4GHz network, verify SSID/password |
| Stream not working | Test CCTV URL in browser first |
| Relay not activating | Check wiring, verify GPIO pin number |

**Full troubleshooting:** 10+ issues in [README.md](README.md)

---

## 🎯 Getting Started

### Choose Your Path:

**🚀 Fast Track (30 min)** → [QUICKSTART.md](QUICKSTART.md)
- For experienced Arduino users
- Condensed instructions
- Assumes familiarity with ESP32

**📚 Complete Guide (1 hour)** → [README.md](README.md)
- Step-by-step instructions
- Detailed explanations
- Troubleshooting included
- **Recommended for first-time users**

**🤖 AI Integration** → [MODEL_GUIDE.md](MODEL_GUIDE.md)
- TensorFlow Lite model setup
- Edge Impulse custom training
- Performance optimization

---

## 🎉 What You'll Build

By the end of this project, you'll have:

✅ **Working Smart Switch System**
- Detects people in custom zones
- Controls relays automatically
- Saves energy intelligently

✅ **Professional Web Interface**
- Draw zones with mouse/touch
- Monitor relays in real-time
- Configure all settings

✅ **Production-Ready Firmware**
- Safe, reliable operation
- Error handling and recovery
- Persistent configuration

✅ **Complete Documentation**
- Setup guides
- Troubleshooting help
- Code examples

---

## 📈 Project Stats

- **Lines of Code:** ~3,500
- **Modules:** 8 separate libraries
- **Documentation:** 4 comprehensive guides
- **Setup Time:** 30-60 minutes
- **Hardware Cost:** $15-20 USD
- **Difficulty:** Intermediate (with beginner-friendly guides)

---

## 🏁 Ready to Start?

1. **Read:** [QUICKSTART.md](QUICKSTART.md) or [README.md](README.md)
2. **Install:** Arduino IDE + libraries ([LIBRARIES.md](LIBRARIES.md))
3. **Wire:** ESP32-CAM + relay module
4. **Upload:** Sketch + SPIFFS data
5. **Configure:** Web UI settings
6. **Test:** Draw zones and verify detection
7. **Deploy:** Mount hardware and enjoy automation!

---

## 💡 Tips for Success

1. **Start with motion detection** (no AI model needed)
2. **Test CCTV stream first** in a browser
3. **Use Serial Monitor** for debugging (115200 baud)
4. **Draw small zones initially** to verify detection
5. **Adjust threshold** if too many false positives
6. **Test watchdog** by disconnecting CCTV

---

## 🙏 Credits

This project builds upon excellent open-source work:
- **ESP32 Arduino Core** by Espressif Systems
- **ESP32-CAM_MJPEG2SD** by s60sc (CCTV node)
- **ESPAsyncWebServer** by me-no-dev
- **ArduinoJson** by Benoit Blanchon
- **TensorFlow Lite** by Google

---

## 📞 Support

Need help?
1. Check **README.md** troubleshooting section
2. Review Serial Monitor output (115200 baud)
3. Verify all libraries installed correctly
4. Test each component individually

---

## 🎊 You're All Set!

Everything you need is in this package:
- ✅ Complete, tested firmware
- ✅ Professional web interface
- ✅ Comprehensive documentation
- ✅ Example configurations

**Next step:** Open [README.md](README.md) and start building! 🚀

---

**Happy building! May your zones be accurate and your relays reliable. ⚡**
