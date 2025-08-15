# 日本語版

For the Japanese version, please visit [README_ja.md](README_ja.md).

---

# AtomS3Lite 2-Wire Capacitive Rain Sensor

An automatic rain detection system using M5AtomS3. It detects rain using a capacitive sensor with two wires and provides real-time rain information through MQTT communication and Pushover notifications.

![Rain Detection System](https://img.shields.io/badge/Status-Production%20Ready-green) ![Arduino](https://img.shields.io/badge/Platform-Arduino-blue) ![ESP32](https://img.shields.io/badge/MCU-ESP32--S3-orange)

<img width="501" height="502" alt="a" src="https://github.com/user-attachments/assets/28650388-753e-4202-bfaa-a2e7c17a6d79" />


## 📋 Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Setup](#setup)
- [Configuration](#configuration)
- [Usage](#usage)
- [LED Status Display](#led-status-display)
- [Troubleshooting](#troubleshooting)
- [API Reference](#api-reference)
- [License](#license)

## ✨ Features

### 🌧️ High-Precision Rain Detection
- **4 measurement methods** automatically selected for optimal detection performance
- **Dynamic baseline adjustment** automatically adapts to environmental changes
- **False detection prevention**: Stabilization filter with 3 consecutive detections
- **Noise removal**: Low-pass filter (α=0.8) and noise threshold (5%)

### 📡 IoT Communication Features
- **MQTT communication**: Sensor data transmission every 30 seconds
- **Pushover notifications**: Real-time notifications to smartphones
- **Time control**: Notifications only during 07:00-19:00 hours (neighbor consideration)
- **Cooldown**: Notification frequency limited to 3-hour intervals

### 🔧 Error Detection & Monitoring
- **Cable disconnection detection**: Automatic detection after 5 consecutive errors
- **WiFi connection monitoring**: Automatic reconnection when disconnected
- **Battery monitoring**: Warning notification after 25 hours of operation
- **NTP time synchronization**: Accurate time control

### 🔒 24/7 Robust Operation Features
- **Software watchdog**: Automatic restart after 120 seconds of no response
- **Periodic restart**: Preventive restart every 7 days
- **Memory monitoring**: Automatic detection of memory shortage (8KB threshold)
- **Connection monitoring**: WiFi/MQTT disconnection count monitoring and automatic recovery
- **System health reports**: Operation status transmission every hour
- **Error threshold management**: Forced restart after 10 connection errors

### 💡 Visual Status Display
- **LED status display**: Intuitive color display of operation status
  - 🟢 Green flashing: Starting up/Calibrating
  - 🔵 Blue solid: Normal operation
  - 🟣 Purple flashing: Rain detected
  - 🔴 Red flashing: Error occurred

## 🛠️ Hardware Requirements

### Required Components
- **M5AtomS3 Lite** (with ESP32-S3)
- **Capacitive Rain Sensor Board** ([NAOTO-001](https://www.switch-science.com/products/8202) - Switch Science)
- **GROVE/Dupont cables** (for PIN1/PIN2 connection)
- **Waterproof case** (for outdoor installation)

### Recommended Environment
- **WiFi environment**: 2.4GHz band compatible
- **Power supply**: USB-C or battery (25 hours continuous operation)
- **Installation location**: Outdoor environment exposed to direct rain

## 🚀 Setup

### 1. Development Environment Preparation

```bash
# Use Arduino IDE or PlatformIO
# Install required libraries
```

**Required libraries:**
- M5AtomS3
- WiFi
- PubSubClient
- HTTPClient
- ArduinoJson
- FastLED

### 2. Wiring

```
M5AtomS3    Sensor
G1      →   Pulse Out
G2      →   Sensor In
```

### 3. File Structure

```
rain_sensor/
├── rain_sensor.ino      # Main sketch
├── config.h             # Configuration file (to be created)
├── config.example.h     # Configuration sample
└── README.md           # This file
```

## ⚙️ Configuration

### Creating config.h file

Copy `config.example.h` to create `config.h` and configure it for your environment:

```cpp
// WiFi settings
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// MQTT settings
const char* mqtt_server = "192.168.1.100";  // MQTT broker IP
const int mqtt_port = 1883;
const char* mqtt_topic = "sensors/rain";
const char* mqtt_client_id = "rain_sensor_01";

// Pushover settings
const char* pushover_api_token = "your_api_token";
const char* pushover_user_key = "your_user_key";

// Location information (optional)
const char* location_name = "Garden";
```

### Configuration Parameters

| Parameter | Default Value | Description |
|-----------|---------------|-------------|
| `RAIN_THRESHOLD_PERCENT` | 15.0% | Rain detection threshold |
| `NOISE_THRESHOLD` | 5.0% | Noise removal threshold |
| `STABILITY_CHECK_COUNT` | 3 times | Consecutive detection count |
| `MQTT_SEND_INTERVAL` | 30 seconds | MQTT transmission interval |
| `PUSHOVER_START_HOUR` | 7 AM | Notification start time |
| `PUSHOVER_END_HOUR` | 7 PM | Notification end time |
| `PUSHOVER_COOLDOWN_HOURS` | 3 hours | Notification cooldown |

## 📱 Usage

### 1. Initial Startup

1. Upload sketch to M5AtomS3
2. Start up with sensor in **completely dry state**
3. Automatic calibration is performed while green LED is flashing
4. Blue LED solid indicates ready

### 2. Operation Check

```
Serial monitor output example:
=== Testing measurement methods ===
Method 1: Capacitance charge time (PIN1->PIN2)
Method 4: Analog difference
Selected Method 4 (Analog). Baseline: 1250
=== Calibration complete - System ready ===
```

### 3. Rain Detection Test

- Drop a few drops of water on the sensor to check operation
- Normal operation confirmed when purple LED flashing and notification are observed

## 🔍 LED Status Display

| LED Status | Operation Status | Action |
|------------|------------------|--------|
| 🟢 Green flashing (200ms) | Starting up/Calibrating | Wait for a while |
| 🔵 Blue solid | Normal operation (no rain) | Normal |
| 🟣 Purple flashing (500ms) | Rain detected | Normal (rain detection) |
| 🔴 Red flashing (300ms) | General error occurred | Check wiring/sensor |
| 🟠 Orange flashing (400ms) | WiFi connection error | Check WiFi settings/signal |
| 🟡 Yellow flashing (350ms) | MQTT connection error | Check MQTT broker |

## 📊 MQTT Data Format

JSON data transmitted every 30 seconds:

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

### Data Field Description

- `id`: Device ID
- `baseline`: Baseline value (dry state reference value)
- `current`: Current measurement value
- `change`: Change rate (%)
- `rain`: Rain detection state (true/false)
- `method`: Measurement method in use (1-4)
- `uptime`: Operation time (hours)
- `cable_ok`: Cable connection state
- `errors`: Consecutive error count
- `timestamp`: UNIX timestamp (Japan time)

## 🔧 Troubleshooting

### Common Issues and Solutions

#### 🟢 Green flashing continues
**Cause**: Sensor connection failure or calibration failure
**Solution**:
1. Check PIN1, PIN2 wiring
2. Ensure sensor is completely dry
3. Restart and recalibrate

#### 🔴 Red flashing
**Cause**: Cable disconnection or WiFi disconnection
**Solution**:
1. Check sensor cable connection
2. Check WiFi settings in config.h
3. Check distance to WiFi router

#### No notifications
**Cause**: Time restriction or Pushover configuration error
**Solution**:
1. Check if current time is within 07:00-19:00 range
2. Check Pushover API token/User key
3. Check 3-hour cooldown period

#### Too many false detections
**Cause**: Threshold too low
**Solution**:
1. Change `RAIN_THRESHOLD_PERCENT` from 15→20%
2. Review sensor installation location
3. Move to location less affected by wind

### Debug Mode

Enable the following flags during development/testing:

```cpp
#define DEBUG_IGNORE_TIME_LIMITS true   // Ignore time restrictions
#define DEBUG_SHORT_COOLDOWN true       // 1-minute cooldown
```

## 🌐 Application Examples

### Agricultural Use
- **Automatic irrigation system**: Stop irrigation when rain is detected
- **Greenhouse management**: Integration with ventilation control
- **Crop protection**: Combination with rainfall prediction

### Home Use
- **Laundry notification**: Prompt to bring in laundry when rain is detected
- **Window closing notification**: Rain countermeasures when away
- **Gardening**: Optimization of watering timing

### Facility Management
- **Outdoor events**: Support for rain countermeasure decisions
- **Construction sites**: Assistance with work suspension decisions
- **Meteorological observation**: Use as simple rain gauge

## 📚 API Reference

### Main Functions

#### Measurement Functions
```cpp
unsigned long measureCapacitanceChargeTime()    // Charge time measurement
unsigned long measureCapacitanceReverse()       // Reverse measurement
unsigned long measureOscillation()              // Oscillation detection
unsigned long measureAnalogDifference()         // Analog measurement
```

#### Communication Functions
```cpp
void sendMQTTData()                             // MQTT transmission
void sendPushoverNotification(message, title)   // Pushover notification
bool isNotificationTime()                       // Time check
```

#### LED Control
```cpp
void setLEDState(LEDState state)                // LED state change
void updateLEDStatus()                          // LED update
```

## 🏃‍♂️ 24/7 Operation Guide

### Recommendations for Long-term Operation

#### Hardware
- **Stable power supply**: Use USB adapters with stable 5V/1A or higher
- **WiFi router**: Equipment that can stably supply 2.4GHz band
- **Installation environment**: Install in a location not exposed to rain, with only sensor part outdoors

#### Software Monitoring
- **System health**: Monitor operation status via MQTT topic `/health`
- **Automatic restart**: Preventive restart is performed every 7 days
- **Error notifications**: Error status is notified via PUSHOVER

#### Regular Maintenance
- **Monthly inspection**: Monthly operation check and log verification
- **Sensor cleaning**: Clean sensor part monthly
- **Connection check**: Check WiFi signal strength and MQTT connection status

#### Troubleshooting
- **LED status check**: Error status displayed with red flashing
- **Serial logs**: Detailed debug information output to serial port
- **Forced restart**: Manual reset possible by unplugging and plugging power

## 🔒 Security Considerations

⚠️ **Important**: Please manage the following information appropriately as confidential

- **config.h**: Contains WiFi passwords and API keys
- **GitHub upload**: Add config.h to .gitignore
- **MQTT broker**: Set appropriate access control
- **Pushover**: Strict management of API token and User key

## 📄 License

MIT License

Copyright (c) 2025 omiya-bonsai

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## 📞 Support

For questions and bug reports, please use the GitHub Issues page.

**Author**: omiya-bonsai (GitHub Copilot assisted development)  
**Updated**: August 6, 2025  
**Version**: v2.2 (24/7 robust operation system implementation)

---

### 🙏 Acknowledgments

This project benefits from many libraries and tools from the open source community. We especially thank the following projects and creators:

#### Open Source Libraries & Platforms
- **M5Stack team** for M5AtomS3 library
- **Arduino community** for ESP32 support
- **FastLED project** for LED control
- **PubSubClient** for MQTT communication
- **ArduinoJson** for JSON handling

#### Hardware & Sales Platforms
- **NAOTO** for [Capacitive Rain Sensor Board](https://www.switch-science.com/products/8202)
- **Switch Science** for providing the marketplace and distribution platform

#### AI Development Support
- **Claude (Anthropic)** for code development assistance and documentation
- **Gemini (Google)** for technical consultation and problem-solving support
- **GitHub Copilot** for code completion and development acceleration