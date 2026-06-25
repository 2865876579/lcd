@echo off
setlocal EnableExtensions

set "ESP_PORT=COM3"
if not "%~1"=="" set "ESP_PORT=%~1"

set "PYTHON=D:\Espressif\python_env\idf5.1_py3.11_env\Scripts\python.exe"

echo ============================================
echo   ESP32-S3 Serial Monitor
echo   Port: %ESP_PORT%
echo   Baud: 115200
echo   Exit: Ctrl+]
echo ============================================
echo.

"%PYTHON%" -m serial.tools.miniterm "%ESP_PORT%" 115200 --raw
