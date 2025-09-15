# VitalGuard: Smart Helmet with Health Monitoring for Firefighter Safety

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Arduino](https://img.shields.io/badge/Arduino-IDE-00979D?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![ESP32](https://img.shields.io/badge/ESP32-Development-red?logo=espressif&logoColor=white)](https://www.espressif.com/)

## 📋 Project Overview

VitalGuard is an innovative smart helmet prototype designed specifically for firefighter safety, integrating real-time health monitoring and communication capabilities. This project addresses critical challenges in firefighting operations by providing continuous vital sign monitoring, environmental hazard detection, and enhanced situational awareness.

### 🎯 Key Features

- **Real-time Health Monitoring**: Continuous tracking of heart rate, blood oxygen levels, and body temperature
- **Environmental Safety**: Carbon monoxide detection and air quality monitoring
- **Motion Detection**: Fall detection and impact monitoring using accelerometer/gyroscope
- **Wireless Communication**: WiFi/Bluetooth connectivity for data transmission to command centers
- **Long Battery Life**: 4000mAh rechargeable battery system with fast charging
- **Robust Design**: Built for extreme firefighting conditions


## 🔧 Hardware Components

### Core Processing Unit
- **ESP32 WROOM-32 Development Board**
  - Acts as the central processing unit
  - Handles sensor data collection, processing, and wireless communication
  - Supports WiFi and Bluetooth connectivity

### Health Monitoring Sensors
- **MAX30102 Pulse Oximeter**
  - Measures heart rate and blood oxygen saturation (SpO2)
  - Non-invasive optical sensing technology
  - Critical for detecting respiratory distress in smoke-filled environments

- **MLX90614 IR Temperature Sensor**
  - Non-contact infrared temperature measurement
  - Monitors forehead/skin temperature
  - Detects heat stress and hyperthermia risks

### Motion & Orientation Sensing
- **MPU-6050 6-DOF IMU**
  - 3-axis accelerometer + 3-axis gyroscope
  - Fall detection and impact monitoring
  - Orientation tracking for situational awareness

### Environmental Monitoring
- **MQ-7 Carbon Monoxide Gas Sensor**
  - Detects CO concentration in the environment
  - Warns about dangerous air quality
  - Prevents carbon monoxide poisoning

### Power Management System
- **18650 Lithium-ion Cells (4000mAh)**
  - High-capacity rechargeable power source
  - Long operational duration for extended missions

- **Type-C 3A Li-ion Charger (IP2312)**
  - Fast charging capabilities
  - Constant-current/constant-voltage (CC/CV) regulation
  - Modern Type-C interface for convenience

- **BMS Li-Ion 1S 18650 3.7V 3A**
  - Battery Management System for safety
  - Protection against overcharge, over-discharge, overcurrent
  - Short circuit protection
  - Extends battery lifespan

## 🏗️ System Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Sensor Layer  │───▶│ Processing Unit │───▶│ Communication   │
│                 │    │                 │    │     Layer       │
│ • MAX30102      │    │   ESP32-WROOM   │    │ • WiFi          │
│ • MLX90614      │    │      32         │    │ • Bluetooth     │
│ • MPU-6050      │    │                 │    │                 │
│ • MQ-7          │    │                 │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         ▲                       ▲                       │
         │                       │                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Power System   │    │   Data Storage  │    │ Command Center  │
│                 │    │                 │    │   Dashboard     │
│ • 18650 Battery │    │ • Local Buffer  │    │                 │
│ • BMS Circuit   │    │ • SD Card       │    │ • Real-time     │
│ • Type-C Charger│    │   (Optional)    │    │   Monitoring    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 🎯 Research Objectives

### General Objective
To design, create, and assess a helmet prototype equipped with vital sign monitoring and communication capabilities targeted to improving safety, efficacy, and situational awareness of firefighters.

### Specific Objectives
1. Identify health and operational risks encountered by firefighters
2. Design a prototype with sensors for monitoring vital health signs
3. Develop a centralized system for data transmission to command centers
4. Test reliability, accuracy, and usability during simulations
5. Assess impact on safety and response efficiency

## 🌟 Significance of the Study

### Primary Beneficiaries
- **Firefighters**: Enhanced individual safety through real-time health monitoring
- **Fire Stations**: Improved decision-making capabilities for command centers
- **Research Community**: Foundation for future smart safety equipment development
- **General Public**: More effective emergency response systems

## 📊 Technical Specifications

### Sensor Specifications
| Component | Measurement Range | Accuracy | Power Consumption |
|-----------|-------------------|----------|-------------------|
| MAX30102 | HR: 60-220 BPM, SpO2: 0-100% | ±2 BPM, ±2% | 1.8V, 50µA |
| MLX90614 | -70°C to +380°C | ±0.5°C | 3.3V, 1.4mA |
| MPU-6050 | ±16g, ±2000°/s | 16-bit ADC | 3.3V, 3.9mA |
| MQ-7 | 20-2000 ppm CO | <5% | 5V, 150mA |

### System Requirements
- **Operating Voltage**: 3.3V - 5V
- **Communication**: WiFi 802.11 b/g/n, Bluetooth 4.2
- **Battery Life**: 8-12 hours continuous operation
- **Charging Time**: 2-3 hours (Type-C fast charging)
- **Operating Temperature**: -10°C to 60°C

## 🚀 Getting Started

### Prerequisites
- Arduino IDE (version 1.8.0 or higher)
- ESP32 Board Package
- Required Libraries (see dependencies)

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/vitalguard-smart-helmet.git
   cd vitalguard-smart-helmet
   ```

2. **Install Arduino Libraries**
   ```bash
   # Install via Arduino Library Manager:
   # - MAX30105 Sensor Library
   # - Adafruit MLX90614 Library
   # - MPU6050 Library
   # - WiFi Library (ESP32)
   # - ArduinoJson Library
   ```

3. **Hardware Setup**
   - Connect sensors according to the wiring diagram
   - Ensure proper power connections with BMS
   - Mount components securely in helmet shell

### Quick Start
```cpp
#include "VitalGuard.h"

VitalGuard helmet;

void setup() {
  helmet.begin();
  helmet.connectWiFi("your-network", "password");
}

void loop() {
  helmet.readSensors();
  helmet.processData();
  helmet.transmitData();
  delay(1000);
}
```

## 📁 Repository Structure

```
vitalguard-smart-helmet/
├── src/
│   ├── main.cpp
│   ├── VitalGuard.h
│   ├── VitalGuard.cpp
│   ├── SensorManager.cpp
│   ├── CommunicationManager.cpp
│   └── PowerManager.cpp
├── hardware/
│   ├── schematics/
│   ├── pcb_design/
│   └── 3d_models/
├── docs/
│   ├── user_manual.md
│   ├── technical_specs.md
│   └── research_paper.pdf
├── examples/
│   ├── basic_monitoring/
│   ├── wireless_communication/
│   └── sensor_calibration/
├── tests/
│   ├── unit_tests/
│   └── integration_tests/
├── tools/
│   └── calibration_scripts/
└── README.md
```

## 🔍 Research Methodology

The project employs an **Iterative Process Model** for system development, incorporating:

- **Research Design**: Mixed-method approach combining quantitative sensor data and qualitative user feedback
- **Testing Environment**: Controlled simulation environments at Kabankalan City Fire Station
- **Data Collection**: Real-time sensor readings, user surveys, and performance metrics
- **Evaluation Criteria**: Reliability, accuracy, usability, and safety impact assessment

## ⚠️ Limitations & Considerations

- **Testing Constraints**: Limited to simulated environments for safety reasons
- **Prototype Durability**: Early version with limited exposure to extreme conditions
- **Battery Life**: Optimized for standard missions (8-12 hours)
- **Sample Size**: Limited testing group due to time and resource constraints
- **Environmental Factors**: Performance may vary in extreme electromagnetic interference

## 🔮 Future Enhancements

- GPS tracking integration
- AI-powered hazard detection
- Voice command interface
- Extended battery life (24+ hours)
- Integration with existing fire department communication systems
- Machine learning for predictive health analytics

## 🤝 Contributing

We welcome contributions from the community! Please read our [Contributing Guidelines](CONTRIBUTING.md) for details on how to submit pull requests, report issues, and suggest improvements.

### Development Setup
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Kabankalan City Fire Station** for collaboration and testing support
- **Faculty of Information Technology** for academic guidance
- **Open-source community** for libraries and tools

**Institution**: School of Engineering, Computer Studies and Architecture

---

## 📈 Project Status

- [x] Literature Review & Research
- [x] Hardware Component Selection
- [x] System Architecture Design
- [ ] Prototype Development
- [ ] Testing & Validation
- [ ] Performance Evaluation
- [ ] Documentation & Deployment

## 🏆 Awards & Recognition

*This section will be updated as the project receives recognition or awards.*

---

**⭐ If you find this project helpful, please consider giving it a star!**

For more information about the research methodology, detailed technical specifications, and complete academic documentation, please refer to the full research paper in the `docs/` directory.
