## Table of Contents
1. [Project Overview](#1-project-overview)
2. [Bill of Materials](#2-bill-of-materials)
3. [Pinout & Wiring](#3-pinout--wiring)
   - [ESP32-S3 Pinout](#esp32-s3-pinout-1)
   - [IO Shield Pinout](#io-shield-pinout-2)
   - [Wiring Diagram](#wiring-diagram)
   - [Physical Implementation](#physical-implementation)
   - [ESP32-S3 GPIO Mapping](#esp32-s3-gpio-mapping)
   - [Power Distribution & Actuator Wiring](#power-distribution--actuator-wiring)
4. [Hardware Decisions & Modifications](#4-hardware-decisions--modifications)
5. [Hardware Architecture](#5-hardware-architecture)
6. [Software Architecture](#6-software-architecture)
7. [System Output](#7-system-output)
8. [Limitations and Future Improvements](#8-limitations-and-future-improvements)
9. [Conclusion](#9-conclusion)
10. [Bibliography](#10-bibliography)

## 1. Project Overview

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
   <img src="IMG/Wiring.jpg" width - 850>
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
    <em>Figure 4.1. Resistive moisture sensor and controller circuit illustrating the voltage-divider measurement principle. Source: Spiess [[3]](#cite3).</em>
</p>

This measurement method is simple; however, it requires an electrical potential to be applied across two conductors that are directly exposed to moist soil. Consequently, current can flow through the water and dissolved ions in the soil. This creates conditions for **electrochemical reactions and electrode corrosion**.

#### Accelerated Corrosion Demonstration

Spiess demonstrates this problem experimentally by placing a resistive sensor in water. To make a process that would normally develop over a much longer period visible within a short demonstration, the electrodes are connected directly to a power supply **without the normal current limitation** [[3]](#cite3).

Therefore, the rate of damage shown in the experiment should not be interpreted as the normal corrosion rate of a sensor installed in soil. Instead, the experiment accelerates the same underlying electrochemical effect that can occur during long-term operation.

<p align="center">
    <img src="IMG/effect%20acceleration.jpg" width="700">
</p>

<p align="center">
    <em>Figure 4.2. Accelerated corrosion experiment in which the resistive electrodes are connected without normal current limitation. Source: Spiess [[3]](#cite3).</em>
</p>

Shortly after the electrodes are energized, **gas bubbles become visible in the water** and one of the sensor legs begins to change colour. The bubbles show that electrochemical reactions are taking place while current flows through the water. At the same time, the conductive plating on one electrode starts to deteriorate [[3]](#cite3).

<p align="center">
    <img src="IMG/buble.jpg" width="600">
</p>

<p align="center">
    <em>Figure 4.3. Gas bubbles forming around the energized resistive sensor electrodes during the accelerated corrosion test. Source: Spiess [[3]](#cite3).</em>
</p>

After several minutes, the damage becomes severe. One electrode loses its original conductive surface and the copper track is progressively removed. In the demonstration, enough copper is eventually removed that the conductive path becomes interrupted and current stops flowing. At this point, the sensor can no longer operate correctly [[3]](#cite3).

<p align="center">
    <img src="IMG/unhealthy,jpg.jpg" width="650">
</p>

<p align="center">
    <em>Figure 4.4. Visible deterioration of the resistive sensor electrodes after the accelerated electrolysis test. Source: Spiess [[3]](#cite3).</em>
</p>

The experiment also demonstrates another undesirable effect: material removed from the electrodes enters the surrounding water. The water becomes visibly discoloured as the electrodes degrade. In an irrigation application, the corresponding environment would be the soil surrounding the plant [[3]](#cite3).

<p align="center">
    <img src="IMG/polluted.jpg" width="600">
</p>

<p align="center">
    <em>Figure 4.5. Discoloration of the surrounding water following electrochemical degradation of the resistive electrodes. Source: Spiess [[3]](#cite3).</em>
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
    <em>Figure 4.6. Relationship between capacitance, excitation frequency and capacitive reactance used to explain capacitive moisture sensing. Source: Spiess [[3]](#cite3).</em>
</p>

In the capacitive sensor examined by Spiess, a **555-family timer** generates a square-wave excitation signal. One sensing conductor is excited by this signal while the other is connected to ground. When the sensor is surrounded by water or moist soil, the capacitance formed by the insulated sensing conductors changes. A diode and capacitor are then used to smooth the resulting signal and provide an **analog output voltage** that changes with moisture [[3]](#cite3).

<p align="center">
    <img src="IMG/capacitive%20schematic.jpg" width="800">
</p>

<p align="center">
    <em>Figure 4.7. Example circuit of a capacitive soil moisture sensor using a 555-family timer and an isolated sensing element. Source: Spiess [[3]](#cite3).</em>
</p>

The excitation frequency also affects the usable measurement range. In the experiment presented by Spiess, the largest difference between the minimum and maximum sensor output was observed at approximately **600 kHz to 900 kHz**. The tested capacitive sensor operated at approximately **570 kHz** [[3]](#cite3).

<p align="center">
    <img src="IMG/frequency.jpg" width="650">
</p>

<p align="center">
    <em>Figure 4.8. Effect of excitation frequency on the measurement range of the capacitive moisture sensor. Source: Spiess [[3]](#cite3).</em>
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

### Temperature and Humidity Sensor

An SHTC3 temperature and humidity sensor is installed inside the enclosure.

Unlike the soil moisture sensor, the SHTC3 is not currently used directly for irrigation control. Its purpose is to monitor the internal environmental conditions of the electronics enclosure.

The sensor provides:

- internal enclosure temperature;
- internal relative humidity.

This information can be useful for checking whether the electronics are operating in an unsuitable environment, for example if heat builds up inside the enclosure or if high humidity creates a risk of condensation.

The SHTC3 communicates with the ESP32-S3 using the I2C interface.

---

### Flyback Diode for the Water Pump

The water pump contains a DC motor, which is an inductive load.

When the motor is switched off, the collapsing magnetic field can generate a voltage spike in the opposite direction. This transient voltage can interfere with or damage the switching circuit.

A flyback diode was therefore connected across the water pump.

During normal operation, the diode does not conduct. When the pump is switched off, the diode provides a path for the inductive current and helps suppress the voltage transient.

This was added to improve the electrical protection and reliability of the pump switching circuit.

## 10. Bibliography:

<a id="cite1"></a>[1] MakerEduVN, "MKE-K01-ESP32-S3-DEV-KIT," _GitHub_, `MKE-K01-ESP32-S3-DEV-KIT/extras/MKE-K01_1.png`, May 2026. [Online]. Available: https://github.com/makereduvn/MKE-K01-ESP32-S3-DEV-KIT. [Accessed: July 27, 2026].

<a id="cite2"></a>[2] MakerEduVN, "MakerEdu MKE-B01 ESP32-S3 IO Shield," _GitHub_, `MKE-B01-ESP32-S3-DK-IO-SHIELD/extras/MKE-B01_2.png`, July 2026. [Online]. Available: https://github.com/makereduvn/MKE-B01-ESP32-S3-DK-IO-SHIELD. [Accessed: August 9, 2026].

<a id="cite3"></a>[3] Andreas Spiess, "#207 Why most Arduino Soil Moisture Sensors suck (incl. solution)," _YouTube_, June 2018. [Online]. Available: https://www.youtube.com/watch?v=udmJyncDvw0. [Accessed: June 3, 2026].

<a id="cite4"></a>[4] Flaura - Smart Plant Pot, "Capacitive Soil Moisture Sensors don't work correctly + Fix for v2.0 v1.2 Arduino ESP32 Raspberry Pi," _YouTube_, October 2021. [Online]. Available: https://www.youtube.com/watch?v=IGP38bz-K48. [Accessed: June 3, 2026].

<a id="cite5"></a>[5] Julian Ilett, "Flyback Diode to Protect Relay," _YouTube_, March 2020. [Online]. Available: https://www.youtube.com/watch?v=gGBrkVRu_Sk. [Accessed: June 9, 2026].

<!--
## References
- [ESP32: Build Your Own Smart Home Sensor in 10 Minutes](https://www.youtube.com/watch?v=llA2mdCh7Kc)
- [Wireless Soil Moisture Sensor Series](https://www.youtube.com/playlist?list=PLUUG94AI2ymwWiahyhoqb7W22lq5oEMo-)
- [Saving Plants - DIY Plant Watering Device](https://github.com/DFRobot/SmartWateringDevice)
-->
