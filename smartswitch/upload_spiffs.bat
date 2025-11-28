@echo off
echo =======================================
echo ESP32 SPIFFS File Uploader
echo =======================================
echo.

REM Find your ESP32's COM port (check Arduino IDE Tools menu)
set /p COM_PORT="Enter COM port (e.g., COM3): "

REM ESP32 SPIFFS partition size (1.5MB)
set SPIFFS_SIZE=0x170000
set SPIFFS_OFFSET=0x290000

echo.
echo Generating SPIFFS image from data folder...
python -m esptool --chip esp32 --port %COM_PORT% --baud 921600 write_flash %SPIFFS_OFFSET% spiffs.bin

echo.
echo =======================================
echo Upload complete! Reset your ESP32.
echo =======================================
pause
