---
comments: true
---

# Full hardware Specs

## General specs

- Wifi and bluetooth
- USB-C connection
- 16 (4x4) mechanical keys
  - RGB led for each key
  - Hot-Swappable for MX style switches
- 2 RGB LEDs for status located on top of the knobs.
- 128x64 OLED screen
- 2 clickable infinite knobs, with tactile click every step of a turn
- Gesture sensor, allowing gestures like swipe up, down, left, and right.
- 1 general purpose header (5V, IO18, IO2, GND)
- 1 battery connector
- 1 switch for turning on and off (when the battery is connected)

## Detailed specs

- [ESP32-wroom-32D microcontroller](https://www.espressif.com/en/products/modules) with Wifi and bluetooth integrated antenna.
- 2x EC11 encoders
- ssd1306 128x64 OLED screen
- CP2102N UART to serial converter
- NCP1117ST33T3G LDO Voltage Regulators 3.3V 1A
- LM2623 DC/DC converter
- MCP73831 battery charger
- 18 SK6812 RGB LEDs (neopixel style)
- 1 I2C header, used for gesture sensor (APDS-9960)
- 1 general purpose header (5V, IO18, IO2, GND)