# SPIFFS Upload Guide - Web Files to ESP32

## Problem
You see "page can't be found" because the web files (index.html, app.js) aren't uploaded to the ESP32's SPIFFS filesystem yet.

## Solution: Upload Web Files

### **Method 1: Arduino IDE Plugin (Recommended)**

#### Step 1: Install ESP32 Sketch Data Upload Plugin

**For Arduino IDE 2.x:**
1. Download plugin: https://github.com/earlephilhower/arduino-esp32fs-plugin/releases
2. Extract `ESP32FS.zip`
3. Copy folder to: `C:\Users\prasa\Documents\Arduino\tools\`
4. Final path should be: `C:\Users\prasa\Documents\Arduino\tools\ESP32FS\tool\esp32fs.jar`
5. **Restart Arduino IDE**

**For Arduino IDE 1.8.x:**
1. Download: https://github.com/me-no-dev/arduino-esp32fs-plugin/releases
2. Extract to: `C:\Users\prasa\Documents\Arduino\tools\`
3. Restart Arduino IDE

#### Step 2: Upload Files
1. Open `SmartSwitch.ino` in Arduino IDE
2. Select your ESP32 board and COM port
3. Go to **Tools → ESP32 Sketch Data Upload** (or **Tools → Upload Sketch Data**)
4. Wait for "SPIFFS Image Uploaded" message
5. Press reset button on ESP32
6. Open Serial Monitor - you should see the IP address
7. Navigate to that IP in browser

---

### **Method 2: Manual Upload Using esptool (Alternative)**

If the plugin doesn't work, use Python tools:

#### Step 1: Install Tools
```powershell
pip install esptool mkspiffs
```

#### Step 2: Create SPIFFS Image
```powershell
cd C:\Users\prasa\Projects\Energy-Management-using-Surveillance-Camera\smartswitch

# Create SPIFFS image from data folder
mkspiffs -c data -b 4096 -p 256 -s 0x170000 spiffs.bin
```

#### Step 3: Find COM Port
- Open Arduino IDE
- Go to **Tools → Port**
- Note the COM port (e.g., COM3, COM5)

#### Step 4: Upload to ESP32
```powershell
# Replace COM3 with your actual port
python -m esptool --chip esp32 --port COM3 --baud 460800 write_flash 0x290000 spiffs.bin
```

#### Step 5: Reset ESP32
- Press the reset button on ESP32
- Check Serial Monitor for IP address

---

### **Method 3: PlatformIO (If You Switch IDEs)**

1. Install VS Code + PlatformIO extension
2. Create new ESP32 project
3. Copy files to project
4. Run: **Upload Filesystem Image** from PlatformIO menu

---

## Verification Steps

After uploading SPIFFS:

1. **Open Serial Monitor** (115200 baud)
2. Press ESP32 reset button
3. You should see:
   ```
   ✓ SPIFFS initialized
   ✓ Configuration loaded from SPIFFS
   ✓ WiFi connected!
   
   ███████████████████████████
   █ Web Interface URL:      █
   █ http://192.168.29.118   █
   ███████████████████████████
   ```

4. **Open browser** to that IP address
5. You should see the **ESP32 Smart Zone Switch** interface

---

## Troubleshooting

### Issue: "ESP32 Sketch Data Upload" option not in Tools menu
**Solution:** 
- Reinstall plugin (see Method 1)
- Make sure folder structure is correct
- Restart Arduino IDE completely

### Issue: Upload fails with "Timed out waiting for packet header"
**Solution:**
- Hold **BOOT** button on ESP32 during upload
- Try lower baud rate: `--baud 115200` instead of 460800

### Issue: SPIFFS mount failed in Serial Monitor
**Solution:**
```cpp
// In SmartSwitch.ino, change:
if (!SPIFFS.begin(true)) {
  // 'true' formats SPIFFS if mount fails
}
```

### Issue: Page loads but shows errors
**Solution:**
- Check all files uploaded: `index.html`, `app.js`, `config.json`, `zones.json`
- File names are case-sensitive
- No subdirectories in data folder

---

## Quick Checklist

- [ ] Install ESP32 Sketch Data Upload plugin
- [ ] Restart Arduino IDE
- [ ] Open SmartSwitch.ino
- [ ] Select correct Board (ESP32 Dev Module)
- [ ] Select correct COM Port
- [ ] Click **Tools → ESP32 Sketch Data Upload**
- [ ] Wait for "SPIFFS Image Uploaded"
- [ ] Reset ESP32
- [ ] Open Serial Monitor (115200 baud)
- [ ] Copy IP address from Serial Monitor
- [ ] Open IP in web browser
- [ ] See web interface ✓

---

## Expected File Structure

```
smartswitch/
├── SmartSwitch.ino
├── config.cpp
├── config.h
├── ... (other .cpp/.h files)
└── data/                  ← SPIFFS will upload this folder
    ├── index.html         ← Main web page
    ├── app.js             ← JavaScript code
    ├── config.json        ← Configuration
    └── zones.json         ← Zone definitions
```

---

## Still Not Working?

If you still see "page can't be found":

1. **Check Serial Monitor output** - any errors?
2. **Ping the ESP32:** `ping 192.168.29.118`
3. **Try accessing:** `http://192.168.29.118/api/config` - should return JSON
4. **Re-upload sketch** AND **SPIFFS** files
5. **Format SPIFFS:** Change `SPIFFS.begin(false)` to `SPIFFS.begin(true)`

---

## Next Steps After Upload Success

1. ✅ Web page loads
2. Go to **Settings** tab
3. Enter camera IP: `192.168.137.206`
4. Enter camera port: `8080`
5. Click **Save Settings**
6. Go to **Zones** tab
7. Click **Draw Zone** button
8. Draw rectangles on video feed
9. Configure relay pins for each zone
10. Test detection!
