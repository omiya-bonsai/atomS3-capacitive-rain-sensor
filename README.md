# AtomS3 Lite Two‑Terminal Capacitive Rain Sensor
> 🌐 This document is also available in **Japanese**: [README_ja.md](./README_ja.md)


An automatic rain detection system built with **M5AtomS3**.  
It detects rainfall using a **two‑wire capacitive sensor** and delivers real‑time updates via **MQTT** and **Pushover** notifications.

![Rain Detection System](https://img.shields.io/badge/Status-Production%20Ready-green) ![Arduino](https://img.shields.io/badge/Platform-Arduino-blue) ![ESP32](https://img.shields.io/badge/MCU-ESP32--S3-orange)

<img width="501" height="502" alt="a" src="https://github.com/user-attachments/assets/28650388-753e-4202-bfaa-a2e7c17a6d79" />

![IMG_7593](https://github.com/user-attachments/assets/dde78c46-e2de-4a08-9e9f-bb1ce7493e10)



## 📋 Table of Contents

- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Setup](#-setup)
- [Configuration](#-configuration)
- [Usage](#-usage)
- [LED Status](#-led-status)
- [Troubleshooting](#-troubleshooting)
- [API Reference](#-api-reference)
- [License](#-license)

## ✨ Features

### 🌧️ High‑Accuracy Rain Detection
- **Four measurement methods** are auto‑selected for optimal detection performance.
- **Dynamic baseline adjustment** adapts to environmental changes.
- **False‑positive suppression:** requires **3 consecutive detections** to confirm rain.
- **Noise mitigation:** low‑pass filter (α = 0.8) and **noise threshold (5%)**.

### 📡 IoT Communications
- **MQTT publishing:** sends sensor data at **30‑second** intervals.
- **Pushover notifications:** real‑time alerts to your smartphone.
- **Time window control:** sends notifications **only 07:00–19:00** (neighbor‑friendly).
- **Cooldown:** rate‑limits alerts with a **3‑hour cooldown**.

### 🔧 Error Detection & Monitoring
- **Cable disconnection detection:** automatically flags after **5 consecutive errors**.
- **Wi‑Fi link monitor:** auto‑reconnect on disconnect.
- **Battery/runtime monitor:** warning after **25 hours** of continuous operation.
- **NTP time sync** for accurate timekeeping.

### 🔒 24/7 Robust Operation
- **Software watchdog:** auto‑reboot after **120 s** of no response.
- **Periodic reboot:** preventive reboot every **7 days**.
- **Memory monitor:** low‑memory auto‑detection (**8 KB threshold**).
- **Connectivity watchdog:** tracks Wi‑Fi/MQTT disconnect counts and restores service.
- **System health reports:** status pushed **hourly**.
- **Error threshold policy:** force reboot after **10 connection errors**.

### 💡 Visual Status Indication
- **LED status colors** provide quick insight:
  - 🟢 **Green blinking:** booting / calibrating
  - 🔵 **Solid blue:** normal operation
  - 🟣 **Purple blinking:** rain detected
  - 🔴 **Red blinking:** error

## 🛠️ Hardware Requirements

### Required Components
- **M5AtomS3 Lite** (ESP32‑S3)
- **Capacitive rain sensor PCB** ([NAOTO‑001](https://www.switch-science.com/products/8202) — Switch Science)
- **GROVE/Dupont cables** (connect to PIN1/PIN2)
- **Weatherproof enclosure** (for outdoor installation)

### Recommended Environment
- **Wi‑Fi:** 2.4 GHz
- **Power:** USB‑C or battery (**~25 hours** continuous operation)
- **Location:** outdoor site directly exposed to rain

## 🚀 Setup

### 1. Prepare the Development Environment

```bash
# Use Arduino IDE or PlatformIO
# Install the required libraries
```

**Required libraries**
- M5AtomS3
- WiFi
- PubSubClient
- HTTPClient
- ArduinoJson
- FastLED

### 2. Wiring

```
M5AtomS3       Sensor
G1         →   Pulse Out
G2         →   Sensor In
```

### 3. Project Structure

```
rain_sensor/
├── rain_sensor.ino      # Main sketch
├── config.h             # Your settings (create this)
├── config.example.h     # Example settings
└── README.md            # This document
```

## ⚙️ Configuration

### Create `config.h`

Copy `config.example.h` to `config.h` and edit for your environment:

```cpp
// Wi‑Fi
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT
const char* mqtt_server = "192.168.1.100";  // MQTT broker IP
const int mqtt_port = 1883;
const char* mqtt_topic = "sensors/rain";
const char* mqtt_client_id = "rain_sensor_01";

// Pushover
const char* pushover_api_token = "YOUR_API_TOKEN";
const char* pushover_user_key = "YOUR_USER_KEY";

// Location (optional)
const char* location_name = "Garden";
```

### Configuration Parameters

| Parameter | Default | Description |
|---|---:|---|
| `RAIN_THRESHOLD_PERCENT` | 15.0% | Threshold for rain detection |
| `NOISE_THRESHOLD` | 5.0% | Noise rejection threshold |
| `STABILITY_CHECK_COUNT` | 3 | Required consecutive detections |
| `MQTT_SEND_INTERVAL` | 30 s | MQTT publishing interval |
| `PUSHOVER_START_HOUR` | 7 | Notification start hour |
| `PUSHOVER_END_HOUR` | 19 | Notification end hour |
| `PUSHOVER_COOLDOWN_HOURS` | 3 h | Notification cooldown window |

## 📱 Usage

### 1. First Boot

1. Upload the sketch to the M5AtomS3.
2. Power on with the sensor **completely dry**.
3. During **green blinking**, automatic calibration runs.
4. When the LED turns **solid blue**, the system is ready.

### 2. Operation Check

```
Serial monitor example:
=== Testing measurement methods ===
Method 1: Capacitance charge time (PIN1->PIN2)
Method 4: Analog difference
Selected Method 4 (Analog). Baseline: 1250
=== Calibration complete - System ready ===
```

### 3. Rain Detection Test

- Drip a few drops of water on the sensor to verify detection.
- **Purple blinking** and a notification confirm normal behavior.

## 🔍 LED Status

| LED State | Meaning | Action |
|---|---|---|
| 🟢 Green blinking (200 ms) | Booting / calibrating | Wait |
| 🔵 Solid blue | Normal (no rain) | — |
| 🟣 Purple blinking (500 ms) | Rain detected | — |
| 🔴 Red blinking (300 ms) | General error | Check wiring/sensor |
| 🟠 Orange blinking (400 ms) | Wi‑Fi error | Check Wi‑Fi settings/signal |
| 🟡 Yellow blinking (350 ms) | MQTT error | Check broker |

## 📊 MQTT Payload

Sends the following JSON every **30 seconds**:

```json
{
  "id": "rain_sensor_01",
  "baseline": 1250,
  "current": 1350,
  "change": 8.0,
  "rain": true,
  "method": 4,
  "uptime": 2.5,
  "cable_ok": true,
  "errors": 0,
  "timestamp": 1691123456
}
```

### Field Descriptions

- `id`: Device ID  
- `baseline`: Baseline (dry reference)  
- `current`: Current measurement  
- `change`: Percent change (%)  
- `rain`: Rain state (true/false)  
- `method`: Measurement method in use (1–4)  
- `uptime`: Hours since boot  
- `cable_ok`: Cable connection status  
- `errors`: Consecutive error count  
- `timestamp`: UNIX epoch (JST)

## 🔧 Troubleshooting

### Common Issues

#### 🟢 Green blinking does not stop
**Cause:** Sensor wiring issue or calibration failure  
**Fix:**
1. Verify wiring for PIN1 and PIN2.
2. Ensure the sensor is **completely dry**.
3. Reboot to re‑run calibration.

#### 🔴 Red blinking
**Cause:** Cable disconnection or Wi‑Fi drop  
**Fix:**
1. Check the sensor cable connection.
2. Recheck Wi‑Fi settings in `config.h`.
3. Check distance to the Wi‑Fi router.

#### No notifications received
**Cause:** Time window or Pushover configuration  
**Fix:**
1. Confirm current time is **07:00–19:00**.
2. Verify Pushover API token/User key.
3. Remember the **3‑hour cooldown**.

#### Many false positives
**Cause:** Threshold too low  
**Fix:**
1. Increase `RAIN_THRESHOLD_PERCENT` from **15 → 20%**.
2. Reconsider the installation location.
3. Move the sensor to a spot less affected by wind.

### Debug Mode

Enable these flags during development/testing:

```cpp
#define DEBUG_IGNORE_TIME_LIMITS true   // Ignore time window
#define DEBUG_SHORT_COOLDOWN true       // 1‑minute cooldown
```

## 🌐 Example Use Cases

### Agriculture
- **Automatic irrigation:** pause watering when rain is detected
- **Greenhouse control:** integrate with ventilation
- **Crop protection:** combine with rainfall forecasts

### Home
- **Laundry alerts:** bring clothes in when it starts raining
- **Window reminders:** close windows when away
- **Gardening:** optimize watering timing

### Facilities
- **Outdoor events:** assist rain‑contingency decisions
- **Construction sites:** support stop/go decisions
- **Weather observation:** use as a simple rain gauge

## 📚 API Reference

### Core Measurement Functions
```cpp
unsigned long measureCapacitanceChargeTime();    // Charge time
unsigned long measureCapacitanceReverse();       // Reverse direction
unsigned long measureOscillation();              // Oscillation detection
unsigned long measureAnalogDifference();         // Analog method
```

### Communications
```cpp
void sendMQTTData();                             // MQTT publish
void sendPushoverNotification(message, title);   // Pushover alert
bool isNotificationTime();                       // Time‑window check
```

### LED Control
```cpp
void setLEDState(LEDState state);                // Change LED state
void updateLEDStatus();                          // Refresh LED
```

## 🏃‍♂️ 24/7 Operations Guide

### Long‑Term Operation Recommendations

#### Hardware
- **Stable power:** USB adapter ≥ **5 V / 1 A**
- **Wi‑Fi AP:** reliable 2.4 GHz coverage
- **Installation:** keep the device sheltered; expose only the sensor to rain

#### Software Monitoring
- **System health:** monitor MQTT topic **`/health`**
- **Auto reboot:** preventive reboot occurs every **7 days**
- **Error alerts:** reported via **Pushover**

#### Routine Maintenance
- **Monthly check:** verify operation and review logs
- **Sensor cleaning:** clean the sensor monthly
- **Connectivity check:** confirm Wi‑Fi signal and MQTT broker status

#### Troubleshooting Aids
- **LED states:** red blinking indicates error conditions
- **Serial logs:** detailed debug output over serial
- **Forced reboot:** power‑cycle to reset manually

## 🔒 Security Notes

⚠️ **Important:** Handle the following as confidential:

- **`config.h`** contains Wi‑Fi passwords and API keys
- **GitHub:** add `config.h` to `.gitignore` before pushing
- **MQTT broker:** enforce proper access control
- **Pushover:** protect API tokens and user keys

## 📄 License

MIT License

Copyright (c) 2025 omiya-bonsai

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the \"Software\"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## 📞 Support

Please open an issue on the GitHub Issues page for questions or bug reports.

**Author:** omiya-bonsai (with GitHub Copilot assistance)  
**Last Updated:** August 6, 2025  
**Version:** v2.2 (24/7 robust operations implemented)

---

### 🙏 Acknowledgements

This project benefits from many open‑source libraries and tools. Special thanks to:

#### Open‑Source Libraries & Platforms
- **M5Stack team** for the M5AtomS3 library
- **Arduino community** for ESP32 support
- **FastLED project** for LED control
- **PubSubClient** for MQTT communication
- **ArduinoJson** for JSON handling

#### Hardware & Marketplace
- **NAOTO** for the [capacitive rain sensor PCB](https://www.switch-science.com/products/8202)
- **Switch Science** for providing the marketplace and distribution platform

#### AI Assistance
- **Claude (Anthropic)** for development and documentation assistance
- **Gemini (Google)** for technical consultation and problem‑solving support
- **GitHub Copilot** for code completion and development acceleration
