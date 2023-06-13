# DSD Esp32 Template - OTA Web Server

Esp32 FreeRTOS template for DSD projects.

## HW Details

This component was tested with ESP32 LyraT board and Wroom dev kit.

## How to use

1. Build and flash board using WebServer component.
2. Board will start WiFi host with name Atlantis 1.
3. Connect to Wifi using pass *Atlantis123*.
4. Once connected your device will be assigned with a new IP like 192.168.X.X
5. Open a browser in your connected device and search 192.168.x.1 (gateway)
6. The page created to select a bin file will be opened. 
![Web Page Reference](pageReference.png?raw=true)
7. Choose a compatible bin for your board from your device.
8. Press upload button.

## Maintenance

Current maintenance is done by Eng. Yeferson Flores (2023/05/30)