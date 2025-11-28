# ESP32-CAM Smart Zone Switch
## License and Attribution

---

## 📜 Project License

This project is released as **open source** for educational, personal, and commercial use.

### Terms of Use

**You are free to:**
- ✅ Use this code in personal projects
- ✅ Use this code in commercial products
- ✅ Modify and adapt the code
- ✅ Distribute the code and derivatives
- ✅ Use for educational purposes

**Under these conditions:**
- 📝 Provide attribution to the original project
- 📝 Include this license and attribution file
- ⚠️ No warranty is provided (see disclaimer below)

---

## 🙏 Third-Party Components

This project uses the following open-source libraries and components:

### 1. ESP32 Arduino Core
- **Project:** https://github.com/espressif/arduino-esp32
- **Author:** Espressif Systems
- **License:** Apache License 2.0
- **Usage:** Core ESP32 functionality, WiFi, GPIO control

### 2. ESPAsyncWebServer
- **Project:** https://github.com/me-no-dev/ESPAsyncWebServer
- **Author:** me-no-dev
- **License:** LGPL 3.0
- **Usage:** Async HTTP server for web interface

### 3. AsyncTCP
- **Project:** https://github.com/me-no-dev/AsyncTCP
- **Author:** me-no-dev
- **License:** LGPL 3.0
- **Usage:** Async TCP library (dependency for ESPAsyncWebServer)

### 4. ArduinoJson
- **Project:** https://arduinojson.org/
- **Author:** Benoit Blanchon
- **License:** MIT License
- **Usage:** JSON parsing and serialization for configuration

### 5. JPEGDEC (Optional)
- **Project:** https://github.com/bitbank2/JPEGDEC
- **Author:** Larry Bank (bitbank2)
- **License:** Apache License 2.0
- **Usage:** JPEG decoding for MJPEG stream frames

### 6. TensorFlow Lite for Microcontrollers (Optional)
- **Project:** https://www.tensorflow.org/lite/microcontrollers
- **Author:** Google / TensorFlow Team
- **License:** Apache License 2.0
- **Usage:** AI person detection models

---

## 🎥 CCTV Node Project

This Smart Switch Node is designed to work with:

**ESP32-CAM_MJPEG2SD**
- **Project:** https://github.com/s60sc/ESP32-CAM_MJPEG2SD
- **Author:** s60sc
- **License:** Apache License 2.0
- **Usage:** CCTV camera node that provides MJPEG stream

This Smart Switch Node does NOT include the CCTV node code. You must set up the CCTV node separately using the s60sc project.

---

## 📖 Documentation & Guides

### Inspired By
- ESP32 official documentation and examples
- TensorFlow Lite Micro examples
- Arduino community tutorials and forums

### Web UI Design
- CSS and JavaScript frameworks: Vanilla (no external dependencies)
- Canvas API: HTML5 standard
- WebSocket: Standard web protocol

---

## ⚖️ License Compliance

### LGPL 3.0 (ESPAsyncWebServer, AsyncTCP)

When using this project, you are incorporating LGPL 3.0 licensed components (ESPAsyncWebServer and AsyncTCP).

**LGPL 3.0 Requirements:**
- If you distribute binary firmware, you must:
  - Provide source code for LGPL components
  - Allow users to relink with modified LGPL libraries
- Alternatively: Dynamically link these libraries (not practical on ESP32)
- Or: Release your entire project under LGPL 3.0 or compatible license

**Recommendation for Commercial Use:**
- Release your firmware source code, OR
- Replace ESPAsyncWebServer with alternative (e.g., standard ESP32 WebServer)

For educational/personal use, LGPL compliance is generally straightforward.

---

## ⚠️ Disclaimer

### No Warranty

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

### Electrical Safety

**Important:**
- This project involves controlling relays that may switch AC power
- Improper wiring can cause:
  - Electric shock
  - Fire hazard
  - Equipment damage
  - Personal injury

**You are responsible for:**
- Following local electrical codes
- Using properly rated relays and components
- Ensuring safe installation
- Testing in a controlled environment first
- Consulting licensed electricians for AC wiring

**The authors and contributors are NOT responsible for:**
- Electrical accidents or injuries
- Property damage
- Incorrect wiring or installation
- Misuse of the system

---

## 🔒 Security Considerations

### Web Interface Security

This project provides a web interface accessible over WiFi.

**Security Notes:**
- No authentication implemented by default
- Anyone on the same WiFi network can access the interface
- WebSocket connections are not encrypted

**Recommendations:**
- Use on a private, secured WiFi network
- Add password authentication if needed (not included)
- Use HTTPS (requires SSL certificate setup)
- Consider network segmentation for IoT devices

### Privacy Considerations

**This system processes video streams:**
- MJPEG frames are processed locally
- No video is stored or transmitted to external servers
- Detection data (bounding boxes) is temporary
- Statistics are stored locally on the ESP32

**Privacy Responsibilities:**
- Comply with local privacy laws (e.g., GDPR, CCPA)
- Inform occupants if cameras are used for detection
- Secure the WiFi network
- Do not use for covert surveillance

---

## 🤝 Contributing

If you wish to contribute to this project:

1. **Improvements are welcome:**
   - Bug fixes
   - Performance optimizations
   - New features
   - Documentation enhancements

2. **Guidelines:**
   - Maintain code style and structure
   - Add comments for new functionality
   - Test changes thoroughly
   - Update documentation if needed

3. **Attribution:**
   - Contributors will be acknowledged
   - Significant contributions may be credited in project files

---

## 📊 Usage Statistics & Telemetry

**This project does NOT:**
- Collect usage statistics
- Send telemetry data
- Phone home to any server
- Track users in any way

**All processing is local:**
- Video analysis happens on the ESP32
- Configuration stored in SPIFFS
- No external network connections (except CCTV stream)

---

## 🎓 Educational Use

**This project is ideal for:**
- Learning embedded systems
- Understanding computer vision basics
- Exploring IoT device development
- Studying web-based device control
- Teaching microcontroller programming

**Educational institutions may:**
- Use this project in courses and workshops
- Modify for educational demonstrations
- Share with students
- Include in curriculum materials

**Please:**
- Provide attribution to the original project
- Share improvements back with the community

---

## 💼 Commercial Use

**You MAY use this project commercially:**
- In products you sell
- In services you provide
- In commercial installations

**Requirements:**
- Comply with LGPL 3.0 for included libraries
- Provide attribution (see below)
- Ensure electrical safety compliance
- Follow local regulations

**Recommended Attribution:**
```
This product uses ESP32-CAM Smart Zone Switch
Open source project by [Your Name]
```

---

## 🏷️ Attribution Template

If you use this project, please include attribution:

### In Documentation:
```
This system is based on the ESP32-CAM Smart Zone Switch project.
Project: [Repository URL]
License: Open Source (see LICENSE.md)
```

### In Source Code:
```cpp
/*
 * Based on ESP32-CAM Smart Zone Switch
 * Original project: [Repository URL]
 * Modified by: [Your Name]
 * Date: [Date]
 */
```

### In Web Interface:
```html
<footer>
  Powered by ESP32-CAM Smart Zone Switch
  <a href="[Repository URL]">Open Source Project</a>
</footer>
```

---

## 📝 Version History

### v1.0.0 (Initial Release)
- Complete firmware implementation
- Web-based zone drawing interface
- TFLite integration framework
- Motion detection fallback
- Comprehensive documentation
- Example configurations
- Safety features (watchdog, emergency stop)

---

## 🔗 Useful Links

### Project Resources
- **GitHub:** [Repository URL]
- **Documentation:** README.md, QUICKSTART.md, MODEL_GUIDE.md
- **Support:** See README.md troubleshooting section

### Related Projects
- **ESP32-CAM_MJPEG2SD:** https://github.com/s60sc/ESP32-CAM_MJPEG2SD
- **ESP32 Arduino Core:** https://github.com/espressif/arduino-esp32
- **TensorFlow Lite:** https://www.tensorflow.org/lite/microcontrollers

### Community
- **Arduino Forum:** https://forum.arduino.cc/
- **ESP32 Forum:** https://www.esp32.com/
- **Reddit:** r/esp32, r/arduino

---

## 🙏 Special Thanks

**To the open-source community:**
- Espressif for the excellent ESP32 platform
- Arduino community for extensive libraries and support
- s60sc for the ESP32-CAM_MJPEG2SD project
- All library authors whose work made this possible

**To you:**
- Thank you for using this project!
- Happy building and enjoy your smart zone switch! 🎉

---

## 📧 Contact

For questions, issues, or contributions:
- Check README.md troubleshooting section first
- Review documentation files
- Check Serial Monitor output for debugging
- Consult Arduino/ESP32 community forums

---

**This project is provided freely to the community. Use it wisely, safely, and ethically.**

*Last updated: 2024*
