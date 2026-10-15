# Smart Irrigation System

**Automated plant watering with ESP32-S3, MQTT telemetry, and a self-hosted monitoring dashboard**

An IoT-based irrigation prototype built around an **ESP32-S3**, a **capacitive soil-moisture sensor**, an **SHTC3 environmental sensor**, and a **12 V water pump**. The controller waters plants locally based on soil-moisture readings and transmits telemetry over Wi-Fi/MQTT to a laptop-hosted data logger and web dashboard.

![Smart Irrigation System hardware](IMG/physical1.jpg)

## Table of Contents
1. [Project Overview](#1-project-overview)
2. [Bill of Materials](#2-bill-of-materials)
3. [Pinout & Wiring](#3-pinout--wiring)
4. [Hardware Decisions & Modifications](#4-hardware-decisions--modifications)
5. [Hardware Architecture](#5-hardware-architecture)
6. [Software Architecture](#6-software-architecture)
   - [ESP32-S3 Monitoring & Irrigation Control](#esp32-s3-monitoring--irrigation-control)
   - [MQTT Communication](#mqtt-communication)
   - [Web Backend](#web-backend)
   - [Web Frontend](#web-frontend)
   - [Running the Complete System](#running-the-complete-system)
7. [System Output](#7-system-output)
8. [Limitations and Future Improvements](#8-limitations-and-future-improvements)
9. [Conclusion](#9-conclusion)
10. [Bibliography](#10-bibliography)

## 1. Project Overview

This Smart Irrigation System is designed to **monitor soil conditions and water a plant automatically** without continuous human intervention. The ESP32-S3 samples a capacitive soil-moisture sensor, measures temperature and relative humidity inside the enclosure with an SHTC3, and switches an R385 pump through a relay. A soil-moisture **ADC threshold of 2000** governs the local watering decision; the pump-control loop does not require an active network connection.

For remote monitoring, a separate firmware version publishes the sensor's raw ADC value and SHTC3 temperature/humidity readings to a **self-hosted MQTT broker** over Wi-Fi. A Python subscriber writes the messages to a local SQLite database. A Flask web application serves a dashboard that displays the latest values, historical charts, and an inferred last-watering time.

**Implemented features**

- Automatic on/off irrigation using a calibrated soil-moisture ADC threshold.
- Capacitive sensing with a 1 MΩ analog-output modification, plus enclosure temperature/humidity measurement.
- Serial-monitor readings for raw ADC value, estimated moisture percentage, and experimental standing-water depth.
- MQTT publish/subscribe telemetry over a local Wi-Fi network.
- Persistent measurements in SQLite and a Flask/Chart.js dashboard that refreshes every five seconds.

**Project scope:** This is a locally hosted prototype, not a cloud-hosted or internet-secured irrigation product. The published MQTT `moisture` topic contains the **raw ADC reading**, *not* the calculated moisture percentage; the percentage and water-depth estimates are currently printed to Serial rather than sent as MQTT data. The water-depth estimate comes from controlled sensor-immersion calibration and should not be treated as a validated reservoir-level measurement.

**Repository layout**

```text
Smart-Irrigation-System/
├── src/
│   ├── sensors.ino            # Local sensor + automatic pumping demo (Serial)
│   └── smart_irrigation.ino   # Integrated ESP32-S3 + Wi-Fi/MQTT firmware
├── server/
│   ├── db.py                  # SQLite schema initialization
│   ├── subscribe.py           # MQTT topic subscriber and data logger
│   ├── view_ref.py            # Flask backend and API
│   ├── requirements.txt      # Python dependencies
│   └── templates/
│       └── index.html        # HTML/JS dashboard using Chart.js
├── IMG/                      # Hardware and calibration images
├── Demo/                     # Demonstration videos
└── README.md
```

The Python server implementation was provided in the **Group17_2026** report archive and has been included under `server/` in the updated repository package.

## 2. Bill of Materials

| Component             | Model                                       | Specification              |
| :-------------------- | :------------------------------------------ | :------------------------: |
| Microcontroller       | MKE-K01 ESP32-S3 DevKit                     | 16MB Flash - 8MB PSRAM     |
| Expansion Board       | MKE-B01 ESP32-S3 DevKit IO Shield           | GPIO Breakout              |
| Soil Moisture Sensor  | Capacitive Soil Moisture Sensor v2.0        | 3.3V-5V, Analog            |
| Resistor              | Metal-Film Resistor                         | 1MΩ±1%                     |
| Environmental Sensor  | SHTC3 Digital Humidity & Temperature Sensor | 1.62V-3.6V, I2C            |
| Relay Module          | 1-Channel Opto-Isolated Relay               | 12VDC, 30A Contacts        |
| Flyback Diode         | 1N4007 Silicon Rectifier Diode              | 1A, 1000V                  |
| DC Power Adapter      | 5.5x2.1mm AC/DC Wall Adapter                | 12VDC, 3A                  |
| Step-down Converter   | Dual USB Buck Converter                     | 12VDC to 5VDC, 3A          |
| Water Pump            | R385 Water Pump                             | 12VDC                      |
| Adapter Cable         | Female DC Barrel Jack Cable                 | 5.5x2.1mm                  |
| Water Tubing          | Silicone Tube                               | 2x2m, 6mm ID - 8mm OD      |
| Electronics Enclosure | Waterproof Plastic Enclosure                | 83x58x33mm, IP65           |

## 3. Pinout & Wiring

### ESP32-S3 Pinout *[[1]](#cite1)*
<p align="center">
   <img src="IMG/MKE-K01_1.png" width = 850>
</p>

### IO Shield Pinout *[[2]](#cite2)*
<p align="center">
   <img src="IMG/MKE-B01_2.png" width = 600>
</p>

### Wiring Diagram
<p align="center">
   <img src="IMG/Wiring.jpg" width="850">
</p>

### Physical Implementation

<p align="center">
    <img src="IMG/physical1.jpg" width="400">
    <img src="IMG/physical2.jpg" width="400">
</p>

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
| **12V Power Adapter** | `+` | Buck Converter `IN+`, Relay `COM`, Relay `DC+` |
| | `-` | Buck Converter `IN-`, Pump `(-)`, Relay `DC-` |
| **5V Buck Converter** | `USB-A` | ESP32-S3 `USB-C` |
| **Relay Module** | `NO` | Water Pump `(+)` |
| **Flyback Diode** | Stripe `Cathode` | Water Pump `(+)` |
| | No-Stripe `Anode` | Water Pump `(-)` |

---

## 4. Hardware Decisions & Modifications

### Soil Moisture Sensor

A **Capacitive Soil Moisture Sensor v2.0** was selected instead of a conventional resistive soil moisture sensor. The selection was based mainly on the sensing principle, expected durability during continuous installation, and the electrochemical problems associated with exposed resistive electrodes.

#### Resistive Soil Moisture Sensors

A typical resistive soil moisture sensor consists of two exposed conductive electrodes inserted into the soil. Electrically, the soil between the two electrodes behaves as a resistance whose value changes with the amount of water present.

In the circuit examined by Spiess [[3]](#cite3), the soil sensor is connected with a **510 kΩ resistor to form a voltage divider**. The analog output corresponds to the voltage produced by this divider. When more of the sensor is placed in water, the resistance between the two electrodes decreases and the analog output voltage also decreases. The controller board additionally contains a comparator for producing a digital threshold output, although the analog signal is more useful when the moisture level is processed directly in software [[3]](#cite3).

<p align="center">
    <img src="IMG/resistive%20schematic.jpg" width="750">
</p>

<p align="center">
    <em>
        Figure 4.1. Resistive moisture sensor and controller circuit illustrating the voltage-divider measurement principle.
        Source: Spiess <a href="#cite3">[3]</a>.
    </em>
</p>

This measurement method is simple; however, it requires an electrical potential to be applied across two conductors that are directly exposed to moist soil. Consequently, current can flow through the water and dissolved ions in the soil. This creates conditions for **electrochemical reactions and electrode corrosion**.

#### Accelerated Corrosion Demonstration

Spiess demonstrates this problem experimentally by placing a resistive sensor in water. To make a process that would normally develop over a much longer period visible within a short demonstration, the electrodes are connected directly to a power supply **without the normal current limitation** [[3]](#cite3).

Therefore, the rate of damage shown in the experiment should not be interpreted as the normal corrosion rate of a sensor installed in soil. Instead, the experiment accelerates the same underlying electrochemical effect that can occur during long-term operation.

<p align="center">
    <img src="IMG/effect%20acceleration.jpg" width="700">
</p>

<p align="center">
    <em>
        Figure 4.2. Accelerated corrosion experiment in which the resistive electrodes are connected without normal current limitation.
        Source: Spiess <a href="#cite3">[3]</a>.
    </em>
</p>

Shortly after the electrodes are energized, **gas bubbles become visible in the water** and one of the sensor legs begins to change colour. The bubbles show that electrochemical reactions are taking place while current flows through the water. At the same time, the conductive plating on one electrode starts to deteriorate [[3]](#cite3).

<p align="center">
    <img src="IMG/buble.jpg" width="600">
</p>

<p align="center">
    <em>
        Figure 4.3. Gas bubbles forming around the energized resistive sensor electrodes during the accelerated corrosion test.
        Source: Spiess <a href="#cite3">[3]</a>.
    </em>
</p>

After several minutes, the damage becomes severe. One electrode loses its original conductive surface and the copper track is progressively removed. In the demonstration, enough copper is eventually removed that the conductive path becomes interrupted and current stops flowing. At this point, the sensor can no longer operate correctly [[3]](#cite3).

<p align="center">
    <img src="IMG/unhealthy.jpg" width="650">
</p>

<p align="center">
    <em>
        Figure 4.4. Visible deterioration of the resistive sensor electrodes after the accelerated electrolysis test.
        Source: Spiess <a href="#cite3">[3]</a>.
    </em>
</p>

The experiment also demonstrates another undesirable effect: material removed from the electrodes enters the surrounding water. The water becomes visibly discoloured as the electrodes degrade. In an irrigation application, the corresponding environment would be the soil surrounding the plant [[3]](#cite3).

<p align="center">
    <img src="IMG/polluted.jpg" width="600">
</p>

<p align="center">
    <em>
        Figure 4.5. Discoloration of the surrounding water following electrochemical degradation of the resistive electrodes.
        Source: Spiess <a href="#cite3">[3]</a>.
    </em>
</p>

Simply insulating the two electrodes is not an effective solution for this type of **resistive** sensor. Its operating principle depends on electrical conduction through the material between the electrodes. If the electrodes are completely isolated from the water or soil, the required conduction path is removed and the original resistive measurement principle no longer works [[3]](#cite3).

For a system intended to remain installed in soil and repeatedly measure moisture over a long period, this represents an important disadvantage. Electrode degradation not only reduces sensor lifetime but also changes the electrical properties of the sensing element, which may affect measurement consistency before complete failure occurs.

#### Capacitive Soil Moisture Sensor

A capacitive moisture sensor uses a different sensing principle. Its conductive sensing areas do not need to be directly exposed to the soil. Instead, the insulated conductive regions behave as the plates of a capacitor, and the electrical characteristics of this capacitor change depending on the surrounding moisture [[3]](#cite3).

The capacitive reactance can be expressed as:

$$
X_C = \frac{1}{2\pi f C}
$$

where:

- $X_C$ is the capacitive reactance;
- $f$ is the excitation frequency;
- $C$ is the capacitance.

As the effective capacitance changes with the surrounding moisture, the capacitive reactance also changes. From the equation, increasing capacitance causes the magnitude of the capacitive reactance to decrease when frequency is kept constant.

<p align="center">
    <img src="IMG/capacitor%20resistance%20formula.jpg" width="800">
</p>

<p align="center">
    <em>
        Figure 4.6. Relationship between capacitance, excitation frequency and capacitive reactance used to explain capacitive moisture sensing.
        Source: Spiess <a href="#cite3">[3]</a>.
    </em>
</p>

In the capacitive sensor examined by Spiess, a **555-family timer** generates a square-wave excitation signal. One sensing conductor is excited by this signal while the other is connected to ground. When the sensor is surrounded by water or moist soil, the capacitance formed by the insulated sensing conductors changes. A diode and capacitor are then used to smooth the resulting signal and provide an **analog output voltage** that changes with moisture [[3]](#cite3).

<p align="center">
    <img src="IMG/capacitive%20schematic.jpg" width="800">
</p>

<p align="center">
    <em>
        Figure 4.7. Example circuit of a capacitive soil moisture sensor using a 555-family timer and an isolated sensing element.
        Source: Spiess <a href="#cite3">[3]</a>.
    </em>
</p>

The excitation frequency also affects the usable measurement range. In the experiment presented by Spiess, the largest difference between the minimum and maximum sensor output was observed at approximately **600 kHz to 900 kHz**. The tested capacitive sensor operated at approximately **570 kHz** [[3]](#cite3).

<p align="center">
    <img src="IMG/frequency.jpg" width="650">
</p>

<p align="center">
    <em>
        Figure 4.8. Effect of excitation frequency on the measurement range of the capacitive moisture sensor.
        Source: Spiess <a href="#cite3">[3]</a>.
    </em>
</p>

The important difference for this project is that the sensing conductors of the capacitive sensor can remain **electrically isolated from the wet soil**. Therefore, moisture measurement does not require the continuous DC conduction path between exposed metal electrodes that causes the corrosion demonstrated with resistive probes.

#### Sensor Selection

The comparison that led to the final sensor selection is summarized below.

| Design Consideration | Resistive Sensor | Capacitive Sensor |
| :--- | :--- | :--- |
| Measurement principle | Electrical conduction through soil | Change in capacitance |
| Exposed sensing conductors | Yes | No exposed sensing copper required |
| DC current through moist soil | Required for measurement | Not required between exposed electrodes |
| Electrode corrosion | Significant long-term concern | Corrosion mechanism greatly reduced |
| Electrode material entering soil | Possible as electrodes degrade | Avoided at the insulated sensing surface |
| Analog measurement | Available | Available |
| Suitability for continuous installation | Lower | Better suited to the project |

For these reasons, the **capacitive soil moisture sensor was selected for the Smart Irrigation System**. The main advantage is not simply that it is a different type of moisture sensor, but that its sensing principle removes the exposed DC electrode interface responsible for the degradation demonstrated with resistive sensors.

In this project, the sensor is powered from **3.3 V**, and its analog output is connected to **GPIO 1** of the ESP32-S3. The sensor does not directly provide an absolute soil moisture percentage. Instead, its analog output is measured by the ESP32-S3 ADC and calibrated using experimentally determined dry and wet reference values. The measured ADC value is then converted into an estimated **soil moisture percentage** before being transmitted to the server.

> **Note:** The circuit-level explanation above describes the capacitive sensor examined in reference [[3]](#cite3). Different commercial revisions of capacitive soil moisture sensors may use different components or circuit implementations. The important design principle for this project is the insulated capacitive sensing method rather than the exact PCB implementation.

---

#### Capacitive Sensor Modification

Although the capacitive sensor avoids the electrode-corrosion problem of resistive sensors, some low-cost **v1.2/v2.0 capacitive soil moisture sensors contain a PCB design issue**. In the version discussed in [[4]](#cite4), the **1 MΩ resistor associated with the analog-output circuit is not correctly connected to ground**.

The sensor converts the moisture-dependent signal into an analog voltage using a diode and capacitor. The capacitor smooths the signal before it reaches the analog output. A high-value resistor provides a discharge path for this capacitor. Without this path, the capacitor can retain its charge, causing the analog output to respond slowly when the moisture level changes [[4]](#cite4).

To correct this issue, a **1 MΩ ±1% resistor** was added between the sensor's analog-output and ground connections.

<p align="center">
    <img src="IMG/capacitive1.jpg" width="700">
</p>

<p align="center">
    <em>
        Figure 4.9. Capacitive Soil Moisture Sensor v2.0 with the added 1 MΩ resistor used to correct the analog-output circuit.
    </em>
</p>

The modification does not change the capacitive sensing principle. Instead, it improves the analog-output stage by providing the capacitor with a discharge path, allowing the output voltage to respond more consistently to changes in soil moisture. This provides a more suitable analog signal for measurement by the ESP32-S3 ADC.

---
### Temperature and Humidity Sensor

An SHTC3 temperature and humidity sensor is installed inside the enclosure.

Unlike the soil moisture sensor, the SHTC3 is not currently used directly for irrigation control. Its purpose is to monitor the internal environmental conditions of the electronics enclosure.

The sensor provides:

- internal enclosure temperature;
- internal relative humidity.

This information can be useful for checking whether the electronics are operating in an unsuitable environment, for example if heat builds up inside the enclosure or if high humidity creates a risk of condensation.

The SHTC3 communicates with the ESP32-S3 using the I2C interface.

---

### Flyback Diode for Water Pump

The R385 water pump is driven by a **12 V DC motor**, which behaves as an inductive load. While the motor is running, energy is stored in its magnetic field. When the relay switches the pump off, this magnetic field collapses and the motor can generate a short **reverse-polarity voltage spike**, known as inductive kickback or flyback voltage [[5]](#cite5).

This transient voltage can be much higher than the normal supply voltage. If it is not suppressed, it may cause arcing across the relay contacts and introduce electrical noise or voltage disturbances into the power system. These disturbances can affect sensitive electronics such as the ESP32-S3, potentially causing unstable operation or unexpected resets.

For this reason, a **1N4007 flyback diode** was installed directly across the terminals of the pump motor.

<p align="center">
    <img src="IMG/motor1.jpg" width="45%">
    <img src="IMG/motor2.jpg" width="45%">
</p>

<p align="center">
    <em>
        Figure 4.10. 1N4007 flyback diode installed across the terminals of the R385 water-pump motor.
    </em>
</p>

During normal pump operation, the diode is **reverse-biased**, so it does not conduct and has no effect on the motor. When the pump is switched off, the polarity across the motor reverses. The diode then becomes forward-biased and provides a path for the remaining motor current to circulate and decay safely [[5]](#cite5).

Instead of allowing the stored energy in the motor to create a large voltage spike, the diode helps dissipate this energy through the motor and diode path. This provides several protective benefits:

* reduces the voltage spike produced when the motor is switched off;
* reduces arcing and wear on the relay contacts;
* reduces electrical noise in the power system;
* helps prevent switching transients from disturbing the ESP32-S3 and other electronics;
* improves the reliability of repeated pump switching.

The flyback diode therefore acts as a simple but important **protection component** in the irrigation system. Its purpose is not to supply power to the motor, but to safely handle the inductive energy released when the motor is turned off, protecting both the relay switching circuit and the sensitive control electronics.

---

## 5. Hardware Architecture

The hardware architecture of the Smart Irrigation System is organized around the **ESP32-S3**, which acts as the central controller. It acquires measurements from the soil-moisture and environmental sensors, processes these measurements, and controls the water pump through a relay module.

The system uses a **12 V DC power supply** as its main power source. The 12 V supply powers the water pump and relay module directly, while a buck converter steps the voltage down to **5 V** for the ESP32-S3. The sensors are then powered from the ESP32-S3's **3.3 V rail**.

```mermaid
flowchart LR
    PSU["12 V DC<br>Power Adapter"]

    BUCK["12 V → 5 V<br>Buck Converter"]
    ESP["ESP32-S3<br>Central Controller"]

    SOIL["Capacitive Soil<br>Moisture Sensor"]
    SHT["SHTC3<br>Temperature & Humidity Sensor"]

    RELAY["12 V Relay Module"]
    PUMP["R385<br>12 V Water Pump"]
    DIODE["1N4007<br>Flyback Diode"]

    PSU --> BUCK
    BUCK --> ESP

    ESP -->|"3.3 V"| SOIL
    SOIL -->|"Analog → GPIO 1"| ESP

    ESP -->|"3.3 V"| SHT
    SHT -->|"I²C SDA GPIO 8<br>SCL GPIO 9"| ESP

    PSU --> RELAY
    ESP -->|"GPIO 4"| RELAY

    PSU -->|"12 V"| PUMP
    RELAY -->|"Switches pump supply"| PUMP

    DIODE --- PUMP
```

<p align="center">
    <em>
        Figure 5.1. Hardware architecture of the Smart Irrigation System.
    </em>
</p>

### Sensing Layer

The system contains two sensing devices.

The **Capacitive Soil Moisture Sensor v2.0** measures the moisture condition of the soil and produces an analog voltage. Its analog output is connected to **GPIO 1**, where it is measured using the ESP32-S3 ADC. The sensor is powered from 3.3 V. As discussed previously, a **1 MΩ resistor modification** was also applied to improve the response of the sensor's analog-output circuit.

The **SHTC3 temperature and humidity sensor** is installed inside the electronics enclosure to monitor its environmental conditions. It communicates digitally with the ESP32-S3 through the I²C interface, using **GPIO 8 for SDA** and **GPIO 9 for SCL**.

### Control Layer

The **ESP32-S3** is the main processing and control device. The MKE-B01 IO Shield provides convenient access to its GPIO and power connections.

The ESP32-S3 performs three main hardware-level functions:

* acquires the analog soil-moisture measurement;
* communicates with the SHTC3 environmental sensor;
* controls the relay responsible for switching the water pump.

The relay control input is connected to **GPIO 4**. This allows the low-voltage ESP32-S3 control signal to switch the separate 12 V pump circuit without requiring the microcontroller to supply the motor current directly.

### Actuation Layer

Water delivery is performed by the **12 V R385 water pump**. Because the pump requires considerably more current than an ESP32-S3 GPIO can provide, it is switched through a dedicated relay.

When irrigation is required, the ESP32-S3 activates the relay, which connects the 12 V supply to the pump. When irrigation is stopped, the relay disconnects the pump.

A **1N4007 flyback diode** is installed directly across the pump motor terminals. The diode suppresses the inductive voltage transient generated when the motor is switched off. This reduces stress on the relay contacts and helps prevent switching noise and voltage disturbances from affecting the ESP32-S3 and other electronics.

### Power Architecture

The system uses two main voltage levels:

|    Voltage   | Devices Supplied                               |
| :----------: | :--------------------------------------------- |
|  **12 V DC** | Water pump, relay module, buck-converter input |
|  **5 V DC**  | ESP32-S3 through USB-C                         |
| **3.3 V DC** | Soil-moisture sensor and SHTC3 sensor          |

The use of separate voltage levels allows the high-current pump to operate from its required 12 V supply while the ESP32-S3 and sensors operate at their appropriate lower voltages.

Overall, the architecture separates the system into **sensing, control, and actuation stages**. The ESP32-S3 forms the connection between these stages: sensor information is acquired and processed by the controller, while the irrigation output is implemented through the relay and 12 V water pump. This separation prevents the motor load from being driven directly by the microcontroller and provides a more reliable hardware design for automatic irrigation.

## 6. Software Architecture

### ESP32-S3 Monitoring & Irrigation Control

The ESP32-S3 firmware is responsible for **sensor acquisition, data processing, irrigation control, and preparation of sensor data for communication with the rest of the system**. The program runs continuously and updates the measurements and pump state approximately once per second.

The embedded part of the software is divided logically into four main stages:

1. **Hardware initialization**
2. **Sensor data acquisition**
3. **Data processing and calibration**
4. **Automatic irrigation control**

The firmware prints the processed soil measurements as a structured JSON message on Serial. The MQTT-enabled sketch additionally publishes **three numeric values** (raw soil ADC, temperature, and humidity) on separate MQTT topics to a self-hosted broker; it does **not** publish the Serial JSON object.

```mermaid
flowchart TD
    START["ESP32-S3 Start"] --> INIT["Initialize ADC, SHTC3<br>and Pump GPIO"]

    INIT --> READ["Read Sensors"]

    READ --> SHT["Temperature & Humidity<br>SHTC3 via I²C"]
    READ --> SOIL["Soil Moisture<br>ADC via GPIO 1"]

    SHT --> PROCESS["Process Sensor Data"]
    SOIL --> PROCESS

    PROCESS --> MOIST["Calculate<br>Moisture Percentage"]
    PROCESS --> DEPTH["Estimate<br>Water Depth"]

    MOIST --> JSON["Create Sensor Data<br>JSON Output"]
    DEPTH --> JSON

    PROCESS --> CONTROL{"Soil ADC ≥ 2000?"}

    CONTROL -->|Yes| ON["Pump ON"]
    CONTROL -->|No| OFF["Pump OFF"]

    ON --> WAIT["1 Second Delay"]
    OFF --> WAIT
    JSON --> WAIT

    WAIT --> READ

    JSON -. "Communication layer" .-> MQTT["MQTT Broker / Server<br>Covered in next section"]
```

<p align="center">
    <em>
        Figure 6.1. ESP32-S3 software architecture showing sensor acquisition, processing, irrigation control, and the interface to the communication layer.
    </em>
</p>

#### Sensor Interface & Data Processing

When the ESP32-S3 starts, the firmware initializes the required hardware interfaces. The ADC is configured to **12-bit resolution**, the SHTC3 temperature and humidity sensor is initialized through I²C, and the pump-control GPIO is configured as a digital output.

```cpp
analogReadResolution(12);

if (!shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
    while (1) delay(1);
}

pinMode(PUMP_PIN, OUTPUT);
```

After initialization, the ESP32-S3 continuously acquires measurements from the two sensors.

The **SHTC3** provides enclosure temperature and relative humidity:

```cpp
sensors_event_t humidity, temp;
shtc3.getEvent(&humidity, &temp);
```

The **capacitive soil moisture sensor** is read through GPIO 1 using the ESP32-S3 ADC:

```cpp
int rawValue = analogRead(SOIL_PIN);
```

The raw soil-moisture ADC value is then converted into an estimated moisture percentage using the experimentally determined dry and wet calibration references:

```cpp
const int VAL_DRY = 2350;
const int VAL_WET_SOIL = 1550;

int moisturePercent =
    map(rawValue, VAL_DRY, VAL_WET_SOIL, 0, 100);

moisturePercent =
    constrain(moisturePercent, 0, 100);
```

A reading close to the dry reference corresponds to approximately **0% soil moisture**, while readings at or below the wet reference are limited to **100%**. This provides a more understandable representation of the soil condition than displaying the raw ADC value alone.

The processed soil measurement is then used for both **water-depth estimation** and **automatic irrigation control** in the following stages of the software.

#### Water-Depth Calibration and Estimation

Additional calibration measurements were performed by immersing the capacitive sensor at fixed water depths. Multiple ADC measurements were recorded at each depth and averaged to characterize the response of the sensor. The recorded calibration data shows that the sensor output decreases as the immersion depth increases.

| Water Depth | Average ADC Reading |
| :---------: | :-----------------: |
|     1 cm    |         1297        |
|     2 cm    |         1112        |
|     3 cm    |         1068        |
|     4 cm    |         1012        |
|     5 cm    |         982         |

<p align="center">
    <img src="IMG/soil_moisture_calibration.png" width="700">
</p>

<p align="center">
    <em>
        Figure 6.2. Measured ADC response of the capacitive soil moisture sensor at different immersion depths.
    </em>
</p>

The calibration results show that the relationship between water depth and ADC reading is **non-linear**. The largest ADC change occurs at shallow immersion depths. As more of the sensor is submerged, the difference in ADC reading produced by each additional centimetre generally becomes smaller.

For example, increasing the water depth from 1 cm to 2 cm changes the average ADC reading by approximately **185 counts**, whereas increasing the depth from 4 cm to 5 cm changes the reading by only approximately **30 counts**.

This indicates that the **sensor sensitivity generally decreases as the water depth increases**. In other words, the calibration curve becomes flatter at greater immersion depths.

Because of this non-linear response, the firmware does not use one single linear equation for the entire measurement range. Instead, it uses several calibration points and performs **piecewise linear interpolation** between adjacent values.

During later testing of the complete system, the calculated depth was also observed to underestimate the physical water level by approximately **1 cm in the shallow measurement range**. The calibration values used in the final firmware were therefore adjusted empirically based on physical validation.

The final reference points used in the software are:

| Estimated Water Depth | Final ADC Reference |
| :-------------------: | :-----------------: |
|          1 cm         |         1550        |
|          2 cm         |         1300        |
|          3 cm         |         1113        |
|          4 cm         |         1068        |
|          5 cm         |         1012        |
|          6 cm         |         981         |

The value of **1550** is used as the transition between saturated soil and approximately 1 cm of standing water. The remaining values are based on the measured calibration results with the approximately 1 cm correction applied.

For example, when the ADC reading lies between the 2 cm and 3 cm calibration points, the firmware calculates an intermediate depth using:

```cpp
if (rawValue > VAL_3CM) {
    waterDepthCm =
        2.0 + ((VAL_2CM - rawValue) /
        (float)(VAL_2CM - VAL_3CM));
}
```

The same interpolation method is repeated between the remaining calibration points.

This approach produces a continuous water-depth estimate instead of forcing the output into fixed integer steps. It also allows the software to better account for the reduced sensitivity observed at higher water levels.

#### Automatic Irrigation Control

The irrigation decision is currently performed directly by the ESP32-S3 using the soil-moisture ADC measurement.

A threshold of **2000** is used:

```cpp
if (rawValue >= 2000) {
    digitalWrite(PUMP_PIN, HIGH);
}
else {
    digitalWrite(PUMP_PIN, LOW);
}
```

Because a higher ADC value represents drier soil, the pump is activated when the reading reaches or exceeds 2000. When the reading falls below this threshold, the pump is switched off.

This means that the basic irrigation function remains local to the ESP32-S3. The pump does not depend on the remote server or MQTT connection being available before an irrigation decision can be made.

This separation is useful because the **control loop remains functional even if network communication is temporarily unavailable**.

#### Sensor Data Output

After the measurements have been processed, the ESP32-S3 combines the important soil values into a **JSON-formatted data structure**:

```json
{
  "raw": 1840,
  "moisture_percent": 68,
  "water_depth_cm": 0.00
}
```

The current firmware outputs this structure through the serial interface. The structured format provides a convenient interface between the embedded firmware and the later communication layer.

This Serial JSON is useful during development and could support richer MQTT messages in a future version. **In the implemented MQTT sketch**, however, telemetry is sent as simple numeric strings on the `moisture`, `temp`, and `humidity` topics.

The overall software data flow can therefore be summarized as:

`Sensors → ESP32-S3 → Calibration & Processing → Irrigation Decision → Structured Data → Communication Layer`

The ESP32-S3 firmware is responsible for acquiring and processing the sensor measurements and controlling the physical irrigation hardware. The networked firmware extends this local control system by transmitting selected measurements through **MQTT** to the project's **self-hosted server**, where they are logged in SQLite and displayed on a website.

### MQTT Communication

The Computer Networks component turns the standalone embedded controller into a **local-network IoT monitoring system**. It uses a publish/subscribe architecture: the ESP32-S3 produces measurements as an **MQTT publisher**, a laptop runs the **MQTT broker**, and a **Python subscriber** records received data. The Flask web service reads the database rather than subscribing directly from the browser.

```mermaid
flowchart TD
    S["Capacitive sensor + SHTC3"] --> E["ESP32-S3: local pump control + MQTT publisher"]
    E -->|"Wi-Fi / TCP 1883 · 3 MQTT topics"| B["MQTT broker on laptop"]
    B -->|"Topic subscriptions"| P["Python / Paho subscriber"]
    P -->|"Insert records"| DB[("SQLite: sensor_data")]
    DB -->|"SQL queries"| F["Flask API on laptop"]
    F -->|"HTTP GET :5000 / JSON"| W["Browser / Chart.js dashboard"]
    E -->|"GPIO 4: independent local control"| R["Relay and 12 V pump"]
```

#### MQTT Architecture and Network Protocols

| Layer / Component | Protocol or Technology | Role |
| :--- | :--- | :--- |
| ESP32-S3 ↔ Wi-Fi access point | IEEE 802.11 / local IP network | Provides network connectivity for the publisher |
| ESP32-S3 → broker → subscriber | MQTT over TCP, **port 1883** | Sends measurement updates using topic-based publish/subscribe |
| Python subscriber → storage | SQLite | Stores timestamped readings locally on the laptop |
| Browser → Flask backend | HTTP, default **port 5000** | Fetches the dashboard and JSON endpoints |
| Browser visualization | JavaScript + Chart.js | Polls Flask for updates and renders time-series plots |

The broker and Flask server are separate processes. The implementation uses a **local network without TLS or MQTT authentication** in its demonstration configuration. Do not expose MQTT port 1883 or the Flask development server to the public internet. Add authentication, TLS, and deployment hardening before any real remote deployment.

**Published topics (implemented in `src/smart_irrigation.ino`):**

| MQTT Topic | Published Value | Units / Notes | Source |
| :--- | :--- | :--- | :--- |
| `moisture` | Raw soil sensor ADC value | ADC counts (0–4095 at 12-bit resolution), **not %** | `analogRead(GPIO 1)` |
| `temp` | Enclosure temperature | °C | SHTC3 |
| `humidity` | Enclosure relative humidity | % RH | SHTC3 |

Each topic is published roughly once per second. The project uses MQTT to send monitoring information **from device to server**; no remote pump-control MQTT subscriber or return-command topic is implemented. The local watering threshold remains effective when the dashboard or broker is offline (although the current firmware's initial Wi-Fi connection step can block startup if Wi-Fi is unavailable; see limitations).

#### Publishing from the ESP32-S3

The networked firmware includes the ESP32 Wi-Fi and `PubSubClient` libraries, then configures the broker and client:

```cpp
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
#define MQTT_HOST "192.168.1.100"  // Replace with your broker PC's LAN IPv4
#define MQTT_PORT 1883
#define CLIENT_ID "my_first_publisher"
```

**Important:** `MQTT_HOST` must be the laptop's **LAN IPv4 address** as seen from the ESP32. The Python subscriber may use `127.0.0.1` when the broker and subscriber run on the same laptop. Both clients must connect to **the same broker**. Do not commit real Wi-Fi passwords to a public repository.

The firmware connects to Wi-Fi, connects to the MQTT broker, and publishes numeric strings:

```cpp
char msg[20];

sprintf(msg, "%d", rawValue);
mqtt_client.publish("moisture", msg);

dtostrf(temp.temperature, 4, 2, msg);
mqtt_client.publish("temp", msg);

dtostrf(humidity.relative_humidity, 4, 2, msg);
mqtt_client.publish("humidity", msg);
```

`mqtt_client.loop()` maintains the MQTT client; the sketch also attempts a broker reconnection when disconnected. The numeric payloads allow a subscriber to parse readings directly as floating-point values. **The UART JSON object is a separate Serial output, not the payload sent to the broker.**

#### Python MQTT Subscriber

The server uses [`paho-mqtt`](https://pypi.org/project/paho-mqtt/) in `server/subscribe.py`. After the connection is established, it subscribes to all three topics:

```python
client.subscribe("moisture")
client.subscribe("temp")
client.subscribe("humidity")
```

For each received message it decodes the numeric payload, labels the measurement by its topic, and inserts a new database record with the **subscriber's local receive time**. It does not reconstruct the ESP32 Serial-only moisture percentage or water-depth values.

#### SQLite Database

The `server/db.py` script creates `server/water_moisture.db` and its `sensor_data` table:

| Column | SQLite Type | Description |
| :--- | :--- | :--- |
| `id` | INTEGER PRIMARY KEY | Auto-incrementing measurement identifier |
| `time` | TEXT | Local reception time formatted `YYYY-MM-DD HH:MM` |
| `topic` | TEXT | `moisture`, `temp`, or `humidity` |
| `value` | REAL | Parsed numeric MQTT payload |

Readings are stored independently for each topic. The timestamps are recorded only to the **minute** in this version, so samples within a minute share the same visible timestamp. The local database is generated at runtime and is not required to be committed to Git.

### Web Backend

The monitoring backend uses **Flask** (`server/view_ref.py`) and SQLite. Flask serves the dashboard from `server/templates/index.html` and exposes three HTTP endpoints, all responding with JSON except the homepage:

| Route | Function | Response |
| :--- | :--- | :--- |
| `/` | Web dashboard | HTML page |
| `/data/latest` | Most recent measurement for each MQTT topic | `time` and `value` per topic |
| `/data/graph` | Last **48 records per topic** in chronological display order | Arrays of timestamp/value pairs |
| `/data/last-watered` | Inferred most recent dry-threshold crossing | Time or `N/A` |

For `/data/latest`, the backend selects the latest row by descending `id`; for the chart endpoint it obtains the newest 48 readings of each topic and then reverses them so the graph runs oldest-to-newest.

**About “Last time watered”:** the backend currently estimates this from a transition of `moisture` raw ADC **from below 2000 to 2000 or above** using SQL `LAG()`. This is an **inferred pump-start event**, not a separately logged pump-state confirmation. It may differ from the actual watering time if MQTT messages are dropped, the controller restarts, or the signal fluctuates around the threshold.

### Web Frontend

The dashboard (`server/templates/index.html`) uses **HTML, CSS, and JavaScript** with [Chart.js](https://www.chartjs.org/) to display latest sensor readings and three historical line charts. When the page opens, it makes requests to the Flask endpoints; JavaScript repeats the requests every **five seconds**.

```javascript
fetchAndUpdateData();
setInterval(fetchAndUpdateData, 5000);
```

The dashboard has separate panels for **raw soil moisture (ADC counts)**, **temperature (°C)**, and **relative humidity (% RH)**, plus the estimated last-watering timestamp. Chart.js is loaded from a CDN, so the graphs require the browser to be able to retrieve that library (or a locally hosted copy).

![Sensor Data Dashboard](IMG/dashboard.png)

*Dashboard screenshot extracted from the team's project report.*

### Running the Complete System

The following steps describe the integrated demonstration on a **trusted local Wi-Fi network**. The broker, subscriber, SQLite file, and Flask server run on the same computer.

**1. Flash the MQTT-enabled firmware**

- Install the ESP32 board support package and the Arduino libraries **Adafruit SHTC3**, **Adafruit Unified Sensor**, and **PubSubClient** (plus the dependencies requested by the library manager).
- Open `src/smart_irrigation.ino` in Arduino IDE using an ESP32-S3-compatible board configuration.
- Edit `WIFI_SSID`, `WIFI_PASS`, and `MQTT_HOST`. Point `MQTT_HOST` at the broker PC's LAN IPv4 address, not `localhost`.
- Upload the sketch; use the **115200-baud** Serial Monitor to check sensor readings and connection messages.

For a sensor-only, offline demo, use `src/sensors.ino` instead.

**2. Start a local MQTT broker**

Install a local MQTT broker such as **Eclipse Mosquitto**. Configure it to listen on TCP **1883** on the local network interface, and allow only the appropriate local clients through the computer's firewall. Recent Mosquitto installations may default to loopback-only mode; this must be configured if the ESP32 is connecting from another device. The initial prototype uses unencrypted local-network MQTT and is **not appropriate for public internet exposure**.

**3. Start the subscriber and create the database**

From the repository root, install the Python dependencies and initialize the SQLite table:

```bash
python -m pip install -r server/requirements.txt
cd server
python db.py
```

Set `MQTT_HOST` to your broker address if it is not on the same PC (`MQTT_PORT` defaults to 1883), and run the subscriber in a terminal:

```bash
python subscribe.py
```

By default, the bundled subscriber connects to `127.0.0.1:1883`, suitable when Mosquitto and Python are on the same laptop. Incoming messages are saved to `server/water_moisture.db`.

**4. Run the web dashboard**

Open a **second terminal** in `server/`, then run:

```bash
python view_ref.py
```

Open **http://127.0.0.1:5000/** in a browser on the host PC. The dashboard polls every five seconds. By default, Flask's development server is bound to localhost; viewing it from a second device would require an appropriate network bind, firewall rule, and a safer deployment configuration. Do not use Flask debug mode on an untrusted network.

**5. Check the measurement path**

Verify `ESP32 → broker → Python subscriber → SQLite → Flask → browser` in sequence. If the dashboard is empty, check that (a) the two MQTT clients are using the same broker, (b) the ESP32 can reach port 1883, (c) `subscribe.py` is running, (d) the database has been initialized, and (e) the browser can access Flask's HTTP server.

## 7. System Output

### Soil Moisture Sensor & Automatic Pump Test

[▶ View Demo](Demo/demo1.mp4)

### Final Enclosed System & Serial Monitor Demonstration

[▶ View Demo](Demo/demo2.mp4)

### Web Interface & Live-data Demonstration

[▶ View Demo](Demo/demo3.mp4)

The third demonstration shows the integrated, networked monitoring setup described in [MQTT Communication](#mqtt-communication).

## 8. Limitations and Future Improvements

The current Smart Irrigation System successfully performs soil-moisture monitoring and automatic pump control, but several parts of the final design were influenced by component availability, troubleshooting difficulties, and the project deadline. As a result, there are several areas where the system could be made smaller, more power-efficient, and more suitable for long-term deployment.

### Lower-Power Controller and Motor Driver

The original design used an **ESP32-C3 together with a MOSFET-based motor driver**. The intention was to create a smaller and more energy-efficient system than the final ESP32-S3 and relay implementation.

During prototyping, the MOSFET switching circuit initially operated but later could not be switched reliably by the ESP32-C3. Directly driving the switching stage from the supply produced the expected response, and separate LED testing confirmed that the ESP32-C3 GPIO was still producing an output signal. However, the exact cause of the MOSFET-driver problem was not identified. During further troubleshooting, the ESP32-C3 was accidentally damaged by a short circuit.

Because the remaining development time was limited, the controller and switching stage were replaced with an **ESP32-S3, IO shield, and relay module**, which provided a more straightforward and reliable implementation. The available 12 V relay module was rated for 30 A, which is considerably higher than required by the R385 pump. Although functional, this combination occupies considerably more enclosure space than the original ESP32-C3 and MOSFET design.

A future version could return to the ESP32-C3 and use a properly designed logic-level MOSFET driver. This would reduce the physical footprint and remove the continuously energized relay coil. The ESP32-C3 could also make greater use of **deep-sleep operation**, waking periodically to take measurements and returning to sleep when no control action or communication is required. This could substantially reduce the controller's average power consumption.

### Variable Pump Control and Drip Irrigation

The current relay provides only two pump states: **fully ON or fully OFF**. A MOSFET driver would allow the pump to be controlled using PWM, providing the possibility of adjusting the average motor drive according to the required water flow.

This would allow the pump to operate more gently instead of repeatedly switching immediately between zero and full power. PWM itself does not directly guarantee a fixed motor current, so a future current-controlled design would require appropriate electrical feedback if precise current limiting were required. However, variable-speed control could still provide better control over the delivered water flow.

A future irrigation design could also use a **recirculating water reservoir and drip-irrigation network**. Small outlet holes or dedicated drip emitters could distribute water gradually around the plant rather than relying on a comparatively large amount of water delivered directly by the pump. Combining variable pump control with drip irrigation could provide more controlled watering and potentially reduce unnecessary water usage.

### Battery-Powered Operation

The present system requires an external power adapter, which limits where the unit can be installed.

A future version could be designed around a rechargeable battery system. When combined with a lower-power ESP32-C3, deep-sleep operation, and a MOSFET motor driver, battery operation could make the system significantly more portable and reduce its dependence on a nearby electrical outlet.

However, integrating lithium-based batteries would require substantially more attention to electrical safety. A practical design would require appropriate battery protection, charging circuitry, over-current protection, undervoltage protection, and a suitable enclosure. These features were intentionally not implemented in the current project because an improperly designed lithium-battery system can introduce fire and thermal hazards.

### Custom Electronics and PCB Design

The current prototype relies heavily on commercially available development boards and modules. This approach reduced development risk and allowed individual sections of the system to be replaced and tested easily, but it also increases the overall physical size and introduces circuitry that is not necessarily required by the final application.

A more advanced version could replace several pre-built modules with only the required integrated circuits and discrete components. Voltage, current, and operating conditions could then be characterized using appropriate measurement equipment before integrating them into the final system.

The project initially avoided this approach because designing the electronics at component level would have introduced additional development and debugging risk under the available time constraint.

A further improvement would be the development of a **custom PCB** containing the microcontroller, sensor interfaces, motor driver, voltage regulation, and protection circuitry. This could substantially reduce wiring, enclosure volume, and unnecessary module overhead while producing a more compact and power-efficient system.

### Water Reservoir Monitoring

An ultrasonic distance sensor was obtained with the intention of measuring the remaining water level inside the reservoir, but this feature was not completed within the project timeframe.

In a future implementation, the ultrasonic sensor could measure the distance between the sensor and the surface of the water. From the known dimensions of the reservoir, this measurement could be converted into an estimated water level or remaining volume.

This would allow the system to detect when the irrigation reservoir is nearly empty and notify the user before the pump attempts to operate without sufficient water.

### Environmental Safety Monitoring

The SHTC3 currently measures the temperature and relative humidity inside the electronics enclosure, but these measurements are used primarily for monitoring rather than active protection.

A future version could define safe operating limits for enclosure temperature and humidity. If either measurement exceeds its permitted range for a sustained period, the system could generate an alarm through the server interface.

For example, unusually high internal humidity could indicate water ingress or condensation, while excessive temperature could indicate unsuitable environmental conditions or an electrical problem. The server could notify the user that the enclosure should be inspected before the condition causes damage to the electronics.

### Sensor Measurement Limitations

Experimental calibration of the capacitive soil moisture sensor also showed that its response is not linear over the complete sensing area. The change in ADC reading was considerably larger at shallow immersion depths and became progressively smaller as more of the sensor was submerged.

This means that the sensor provides lower sensitivity at greater water depths, making small changes increasingly difficult to distinguish in the upper portion of the measurement range. The current software partially compensates for this behaviour through experimentally determined calibration points and piecewise linear interpolation.

For future development, calibration could be performed using a larger number of measurement points and repeated across different soil types rather than relying primarily on controlled water-immersion testing. This would provide a calibration model that more closely represents the conditions experienced during actual irrigation.

## 9. Conclusion

The system demonstrates a complete local **IoT measurement and monitoring pipeline**, combining an embedded sensing/actuation controller with Wi-Fi, an MQTT broker, a subscriber, SQLite storage, and a web dashboard. In the final prototype, the ESP32-S3 can detect dry soil and control the pump independently, while measurements are transmitted for visualization without making the physical control loop depend on the website.

MQTT's publish/subscribe approach decouples the device from its monitoring clients and allows additional consumers of the same sensor topics to be added later. The current system is a working proof of concept rather than a deployment-ready irrigation product: longer-term accuracy, power use, pump-drive hardware, network security, and reservoir sensing all remain areas for improvement.

## 10. Bibliography

<a id="cite1"></a>[1] MakerEduVN, "MKE-K01-ESP32-S3-DEV-KIT," _GitHub_, `MKE-K01-ESP32-S3-DEV-KIT/extras/MKE-K01_1.png`, May 2026. [Online]. Available: https://github.com/makereduvn/MKE-K01-ESP32-S3-DEV-KIT. [Accessed: July 27, 2026].

<a id="cite2"></a>[2] MakerEduVN, "MakerEdu MKE-B01 ESP32-S3 IO Shield," _GitHub_, `MKE-B01-ESP32-S3-DK-IO-SHIELD/extras/MKE-B01_2.png`, July 2026. [Online]. Available: https://github.com/makereduvn/MKE-B01-ESP32-S3-DK-IO-SHIELD. [Accessed: August 9, 2026].

<a id="cite3"></a>[3] Andreas Spiess, "#207 Why most Arduino Soil Moisture Sensors suck (incl. solution)," _YouTube_, June 2018. [Online]. Available: https://www.youtube.com/watch?v=udmJyncDvw0. [Accessed: June 3, 2026].

<a id="cite4"></a>[4] Flaura - Smart Plant Pot, "Capacitive Soil Moisture Sensors don't work correctly + Fix for v2.0 v1.2 Arduino ESP32 Raspberry Pi," _YouTube_, October 2021. [Online]. Available: https://www.youtube.com/watch?v=IGP38bz-K48. [Accessed: June 3, 2026].

<a id="cite5"></a>[5] Julian Ilett, "Flyback Diode to Protect Relay," _YouTube_, March 2020. [Online]. Available: https://www.youtube.com/watch?v=gGBrkVRu_Sk. [Accessed: June 9, 2026].


<a id="cite6"></a>[6] Rui Santos, "ESP8266 Publishing DHT22 Readings with MQTT to Raspberry Pi," _Random Nerd Tutorials_. [Online]. Available: https://randomnerdtutorials.com/esp8266-publishing-dht22-readings-with-mqtt-to-raspberry-pi/. [Accessed: August 3, 2026].

<!--
## References
- [ESP32: Build Your Own Smart Home Sensor in 10 Minutes](https://www.youtube.com/watch?v=llA2mdCh7Kc)
- [Wireless Soil Moisture Sensor Series](https://www.youtube.com/playlist?list=PLUUG94AI2ymwWiahyhoqb7W22lq5oEMo-)
- [Saving Plants - DIY Plant Watering Device](https://github.com/DFRobot/SmartWateringDevice)
-->
