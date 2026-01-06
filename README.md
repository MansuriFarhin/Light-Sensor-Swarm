# Light Sensor Swarm (ESP8266 + Raspberry Pi)
This project creates a distributed light-sensing system using multiple ESP8266 modules and a Raspberry Pi. The system determines a "Master" node based on the highest light reading and visualizes this data on an LED matrix and a web dashboard.

---
## System Overview
- **ESP8266** reads light intensity and broadcasts it via UDP.
- **Raspberry Pi** listens for packets, identifies the master device, and updates:
  - An 8×8 LED Matrix (MAX7219)
  - Dedicated GPIO LEDs for active devices
---

## Components
### Hardware
- ESP8266
- Raspberry Pi 5
- photocell sensor
- MAX7219 8x8 LED Matrix
- 3x GPIO LEDs
- Button

### Software
- Python 3 (for Raspberry Pi server)
- Node.js (for web dashboard)
- Arduino IDE for ESP8266
