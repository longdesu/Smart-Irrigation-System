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


## 4. Sensors

### 4.1. Capacitive Soil Moisture Sensor

The main sensor used for the irrigation system is a **Generic Capacitive Soil Moisture Sensor v2.0**. Its purpose is to estimate the moisture condition of the soil and provide an analog signal to the ESP32-S3.

#### Capacitive vs. Resistive Soil Moisture Sensors

A common alternative is the resistive soil moisture sensor. Resistive sensors estimate soil moisture by passing electrical current between two exposed electrodes and measuring the electrical resistance of the soil.

This approach is simple, but the exposed electrodes can degrade over time due to corrosion and electrochemical effects. The measurement is also influenced by the electrical conductivity of the soil.

A capacitive soil moisture sensor does not rely on direct current passing through the soil. Instead, it detects changes in the dielectric properties around the sensing area. Because water has a strong influence on the dielectric properties of the surrounding material, the sensor output changes as the amount of water around the probe changes.

For this project, a capacitive sensor was therefore selected because it is more suitable for continuous soil monitoring and reduces the electrode-corrosion problem associated with simple resistive probes. *[[3]](#cite3)*

| Resistive Sensor | Capacitive Sensor |
| :--- | :--- |
| Measures soil resistance/conductivity | Measures changes related to capacitance |
| Uses exposed conductive electrodes | Sensing traces are insulated from direct soil contact |
| More susceptible to corrosion | Less susceptible to electrode corrosion |
| Strongly affected by soil conductivity | Better suited for continuous monitoring |

---

#### ESP32-S3 ADC Resolution

The soil moisture sensor produces an **analog voltage**, which is converted into a digital value by the Analog-to-Digital Converter (ADC) of the ESP32-S3.

The ESP32-S3 supports a **12-bit ADC**. *[[4]](#cite4)*

A 12-bit ADC can represent:

2^12 = 4096 levels

For comparison:

Arduino 10-bit ADC  = 1024 levels
ESP32-S3 12-bit ADC = 4096 levels

Therefore, a 12-bit ADC provides four times as many digital levels as a 10-bit ADC.

Assuming a nominal measurement range of 0 V to 3.3 V, the theoretical voltage represented by one ADC count is approximately:

Voltage step = 3.3 V / 4096
             ≈ 0.000806 V
             ≈ 0.806 mV/count

This finer ADC resolution is useful when observing relatively small changes in the analog output of the soil moisture sensor.

> **Note:** 0.806 mV/count represents the theoretical ADC resolution for a 0-3.3 V range. It does not mean that the practical measurement accuracy is exactly 0.806 mV, because noise and ADC characteristics also affect the measurement.

---

#### Initial Moisture Testing

Initial testing was performed using different moisture conditions.

The approximate raw ADC values observed were:

| Condition     | Raw ADC Value |
| :------------ | :-----------: |
| Idle / Dry    |     ~2350     |
| Lightly humid |   ~2000-2300  |
| Heavily humid |   ~1500-1800  |

A **higher ADC value indicates a drier condition**, while the ADC value decreases as more water is introduced around the sensor.

These measurements are later used to convert the raw sensor output into a more understandable moisture percentage before the data is transmitted to the server.

For example:

Raw ADC value
      ↓
Sensor calibration
      ↓
Moisture percentage
      ↓
MQTT
      ↓
Server / Web Visualizer

---

#### Water Depth Experiment

To further investigate the behavior of the capacitive sensor, the probe was immersed in water at different depths from **1 cm to 5 cm**.

Multiple measurements were recorded at each depth instead of relying on a single ADC reading.

##### Raw Measurements

| Water Depth | Raw ADC Measurements                                                      |
| :---------: | :------------------------------------------------------------------------ |
|     1 cm    | 1307, 1293, 1286, 1287, 1285, 1289, 1266, 1266, 1294, 1321, 1331, 1333    |
|     2 cm    | 1108, 1109, 1119, 1113, 1107, 1113, 1111, 1119, 1117, 1109, 1114, 1110    |
|     3 cm    | 1050, 1066, 1063, 1068, 1063, 1067, 1078, 1065, 1068, 1077, 1081, 1075    |
|     4 cm    | 1011, 1018, 1016, 1013, 1004, 995, 1007, 1011, 1015, 1021, 1019, 1013     |
|     5 cm    | 979, 970, 996, 996, 989, 975, 983, 978, 975, 983, 978, 983, 979, 986, 985 |

The average result for each water depth was:

| Water Depth | Average ADC Value |
| :---------: | :---------------: |
|     1 cm    |       1296.5      |
|     2 cm    |       1112.4      |
|     3 cm    |       1068.4      |
|     4 cm    |       1011.9      |
|     5 cm    |       982.3       |

The change between each measurement depth was:

| Depth Change | Change in Average ADC |
| :----------: | :-------------------: |
|  1 cm → 2 cm |         -184.1        |
|  2 cm → 3 cm |         -44.0         |
|  3 cm → 4 cm |         -56.5         |
|  4 cm → 5 cm |         -29.6         |

The experiment shows that increasing the water depth causes the ADC value to decrease. However, the response is **not linear**.

The largest change occurs between 1 cm and 2 cm. At greater depths, adding another centimeter of water generally produces a much smaller change in the sensor output.

Therefore, the experimental results suggest that the **sensitivity of the sensor decreases as a larger portion of the sensing area is surrounded by water**.

Water depth increases
        ↓
ADC value decreases
        ↓
Change per additional centimeter becomes smaller
        ↓
Sensor response begins to flatten

The response does not decrease by exactly the same amount for every centimeter. For example, the change from 3 cm to 4 cm is slightly larger than from 2 cm to 3 cm. This variation is expected in experimental measurements and is one reason multiple samples were collected at each depth.

> **Observation:** During testing, the front side of the sensor produced a noticeably stronger response than the back side. Therefore, sensor orientation should be kept consistent during calibration and normal operation.

---

#### Purpose of Calibration

The raw ADC measurement itself is difficult for a user to interpret. A value such as `1780` does not directly indicate whether the plant requires watering.

For this reason, the ESP32-S3 processes the raw value before transmitting the measurement to the network.

The intended data flow is:

Physical soil condition
        ↓
Capacitive sensor
        ↓
Raw ADC value
        ↓
ESP32-S3 calibration
        ↓
Moisture percentage
        ↓
MQTT Broker
        ↓
Server
        ↓
Web Browser

This allows the web interface to display a meaningful value such as:

Soil Moisture: 63%

instead of only displaying:

ADC: 1847

The calibration is therefore part of the sensor-processing stage of the project and provides more meaningful data for later visualization.

---

## 15. Citations:
<a id="cite1"></a>[1] [MakerEdu MKE-K01 ESP32-S3 DevKit](https://github.com/makereduvn/MKE-K01-ESP32-S3-DEV-KIT). Sơ đồ chân.  

<a id="cite2"></a>[2] [MakerEdu MKE-B01 ESP32-S3 IO Shield](https://github.com/makereduvn/MKE-B01-ESP32-S3-DK-IO-SHIELD). Hình ảnh sản phẩm.

<a id="cite3"></a>[3] Capacitive soil moisture sensing reference. 

<a id="cite4"></a>[4] Espressif ESP32-S3 Analog-to-Digital Converter documentation.
<!--
## References

- [Why most Arduino Soil Moisture Sensors suck](https://www.youtube.com/watch?v=udmJyncDvw0)
- [ESP32: Build Your Own Smart Home Sensor in 10 Minutes](https://www.youtube.com/watch?v=llA2mdCh7Kc)
- [Wireless Soil Moisture Sensor Series](https://www.youtube.com/playlist?list=PLUUG94AI2ymwWiahyhoqb7W22lq5oEMo-)
- [Flaura - Smart Plant Pot](https://www.youtube.com/@FlauraSmartPlantPot/featured)
- [Saving Plants - DIY Plant Watering Device](https://github.com/DFRobot/SmartWateringDevice)
-->
