## Table of Contents

1. [Project Overview](#1-project-overview)

2. [Bill of Materials](#2-bill-of-materials)

3. [Pinout & Wiring Diagram](#3-pinout--wiring-diagram)
   - [ESP32-S3 Pinout](#esp32-s3-pinout)
   - [IO Shield Pinout](#io-shield-pinout)
   - [Wiring Diagram](#wiring-diagram)
   - [ESP32-S3 GPIO Mapping](#esp32-s3-gpio-mapping)
   - [Power Distribution & Actuator Wiring](#power-distribution--actuator-wiring)

4. [Sensors](#4-sensors)
   - [Capacitive Soil Moisture Sensor](#capacitive-soil-moisture-sensor)
   - [SHTC3 Temperature & Humidity Sensor](#shtc3-temperature--humidity-sensor)
   - [Sensor Calibration](#sensor-calibration)

5. [Automation Logic](#5-automation-logic)
   - [Soil Moisture Processing](#soil-moisture-processing)
   - [Automatic Watering Decision](#automatic-watering-decision)
   - [Relay and Pump Control](#relay-and-pump-control)

6. [Hardware Architecture](#6-hardware-architecture)
   - [Sensor Layer](#sensor-layer)
   - [Control Layer](#control-layer)
   - [Actuator Layer](#actuator-layer)
   - [Power Architecture](#power-architecture)

7. [Software Architecture](#7-software-architecture)
   - [ESP32 Firmware](#esp32-firmware)
   - [Program Initialization](#program-initialization)
   - [Main Program Loop](#main-program-loop)
   - [Data Processing](#data-processing)

8. [Network Architecture](#8-network-architecture)
   - [System Communication Flow](#system-communication-flow)
   - [ESP32 Network Connection](#esp32-network-connection)
   - [MQTT Broker](#mqtt-broker)

9. [MQTT Communication](#9-mqtt-communication)
   - [MQTT Topics](#mqtt-topics)
   - [Published Sensor Data](#published-sensor-data)
   - [Subscribed Control Data](#subscribed-control-data)
   - [Message Format](#message-format)

10. [Server and Visualizer](#10-server-and-visualizer)
    - [Server-Side Software](#server-side-software)
    - [MQTT Data Handling](#mqtt-data-handling)
    - [Data Visualization](#data-visualization)

11. [System Output](#11-system-output)
    - [Web Browser Interface](#web-browser-interface)
    - [Displayed Sensor Data](#displayed-sensor-data)
    - [Pump Status](#pump-status)

12. [Testing](#12-testing)
    - [Sensor Testing](#sensor-testing)
    - [Automation Testing](#automation-testing)
    - [MQTT Communication Testing](#mqtt-communication-testing)
    - [End-to-End System Testing](#end-to-end-system-testing)

13. [Limitations and Future Improvements](#13-limitations-and-future-improvements)

14. [Conclusion](#14-conclusion)

15. [Citations](#15-citations)




## 2. Bill of Materials

| Item | Model | Specification|
|:------------|:------------|:------------:|
| Microcontroller | ESP32-S3 DevKit | 16MB Flash - 8MB PSRAM |
| Expansion Board | MKE-B01 ESP32-S3 DK IO Shield | Breakout |
| Soil Moisture Sensor | Generic Capacitive Soil Moisture Sensor v2.0 | 3.3V-5V (Analog) |
| Environmental Sensor | SHTC3 Digital Humidity & Temperature Sensor | 3.3V (I2C) |
| Motor Controller | 1-Channel Opto-Isolated Relay Module | 12VDC - 30A |
| DC Power Adapter |  AC/DC Wall Adapter | 12VDC - 3A (5.5x2.1mm) |
| Step-down Converter | Dual USB Buck Converter | 5VDC - 3A |
| Water Pump | R385 Water Pump | 12VDC |
| Adapter Cable | Female DC Barrel Jack Cable| 5.5x2.1mm Jack |

## 3. Pinout & Wiring Diagram

### ESP32-S3 Pinout *[[1]](#cite1)*
<img src="IMG/MKE-K01_1.png">

### IO Shield Pinout *[[2]](#cite2)*
<img src="IMG/MKE-B01_2.png">

### Wiring Diagram
<img src="IMG/Schematic.PNG">

---

### ESP32-S3 GPIO Mapping

| Component | Component Pin | GPIO Connection | Notes |
| :--- | :---: | :---: | :--- |
| **Soil Moisture Sensor** | VCC | `3V3` | 3.3V Power |
| | GND | `GND` | Ground |
| | AOUT | `GPIO 1` | Analog Output Signal |
| **SHTC3 Temp & Humidity** | VCC | `3V3` | 3.3V Power |
| | GND | `GND` | Ground |
| | SDA | `GPIO 8` | I2C Data Line |
| | SCL | `GPIO 9` | I2C Clock Line |
| **1-Channel Relay Module** | IN | `GPIO 4` | Relay Trigger Signal |

---

### Power Distribution & Actuator Wiring

| Source / Device | Pin / Terminal | Connected To |
| :--- | :---: | :--- |
| **12V DC Adapter** | `+` (Positive) | Buck Converter `IN+`, Relay `COM`, Relay `DC+` |
| | `-` (Ground) | Buck Converter `IN-`, Pump `(-)`, Relay `DC-` |
| **Step-Down Buck Converter** | USB A | ESP32-S3 USB C |
| **1-Channel Relay Module** | `NO` (Normally Open) | Water Pump `(+)` |

## 15. Citations:
<a id="cite1"></a>[1] [MakerEdu MKE-K01 ESP32-S3 DevKit](https://github.com/makereduvn/MKE-K01-ESP32-S3-DEV-KIT). Sơ đồ chân.  
<a id="cite2"></a>[2] [MakerEdu MKE-B01 ESP32-S3 IO Shield](https://github.com/makereduvn/MKE-B01-ESP32-S3-DK-IO-SHIELD). Hình ảnh sản phẩm.
<!--
## References

- [Why most Arduino Soil Moisture Sensors suck](https://www.youtube.com/watch?v=udmJyncDvw0)
- [ESP32: Build Your Own Smart Home Sensor in 10 Minutes](https://www.youtube.com/watch?v=llA2mdCh7Kc)
- [Wireless Soil Moisture Sensor Series](https://www.youtube.com/playlist?list=PLUUG94AI2ymwWiahyhoqb7W22lq5oEMo-)
- [Flaura - Smart Plant Pot](https://www.youtube.com/@FlauraSmartPlantPot/featured)
- [Saving Plants - DIY Plant Watering Device](https://github.com/DFRobot/SmartWateringDevice)
-->
