# Analysis: Integrated I2C Bridge Node & Power Autonomy Strategy

**Author:** Rocco Reus – Embedded & Robotics Engineer  
**Date:** April 22, 2026  
**Version:** 1.5 <br>
**Classification:** Internal <br>
**Company**: Smart Heaven B.V.

---

## Table of Contents

<!-- TOC -->
* [Analysis: Integrated I2C Bridge Node & Power Autonomy Strategy](#analysis-integrated-i2c-bridge-node--power-autonomy-strategy)
  * [Table of Contents](#table-of-contents)
  * [1. Introduction](#1-introduction)
  * [2. Problem Statement](#2-problem-statement)
  * [3. Research Questions](#3-research-questions)
  * [4. I2C vs Shift Registers](#4-i2c-vs-shift-registers)
    * [4.1 Table 1: Comparison of I2C and Shift Registers](#41-table-1-comparison-of-i2c-and-shift-registers)
    * [4.2 Findings](#42-findings)
  * [5. Single Addressable Node Concept](#5-single-addressable-node-concept)
    * [5.1 Table 2: Characteristics of Addressable Nodes](#51-table-2-characteristics-of-addressable-nodes)
    * [5.2 Findings](#52-findings)
  * [6. Scalability and Physical Constraints](#6-scalability-and-physical-constraints)
    * [6.1 Findings](#61-findings)
  * [7. Power Analysis](#7-power-analysis)
    * [7.1 Table 3: Power Consumption Overview](#71-table-3-power-consumption-overview)
    * [7.2 Findings](#72-findings)
  * [8. Synthesis](#8-synthesis)
  * [9. References](#9-references)
<!-- TOC -->

---

## 1. Introduction

The development of smart infrastructure requires embedded systems that are scalable, reliable, and maintainable. Within
this project, the goal is to design a modular bridge system in which each segment functions as a single addressable
node.

Smart city systems increasingly rely on interconnected digital technologies, such as IoT and embedded sensing, to
improve infrastructure efficiency and resilience (Kaiser & Deb, 2025).

The ESP32-S3 is used as the central controller due to its integrated wireless communication and processing capabilities.
However, scaling such a system introduces challenges related to communication architecture, modularity, and power
stability.

This analysis investigates how an I2C-based bus architecture supports modular scalability and evaluates whether a
dedicated power regulation strategy is required for stable operation. This work builds upon earlier analysis of the City
Hub architecture and system power constraints (Reus, 2026a; Reus, 2026b).

---

## 2. Problem Statement

Traditional embedded system designs often rely on direct GPIO expansion or shift-register cascades. While these
approaches are sufficient for small-scale systems, they introduce limitations when applied to modular infrastructure.

Microcontrollers such as the ESP32-S3 have a limited number of GPIO pins, restricting scalability as additional
components are added (Espressif Systems, 2026). Furthermore, shift-register-based systems rely on sequential data
propagation, which increases complexity and reduces reliability as system size grows (Wevolver, 2023).

In addition, modern IoT controllers exhibit dynamic power consumption. The ESP32-S3 can draw peak currents exceeding 300
mA during wireless communication, which can lead to voltage instability if not properly managed (Espressif Systems, 2026).

These issues highlight the need for a communication architecture that is scalable and fault-tolerant, as well as a power
design capable of handling peak loads reliably.

---

## 3. Research Questions

**Main Research Question**  
How can an I2C-based bus architecture contribute to a scalable and stable modular bridge system?

**Sub-questions**

1. How does I2C compare to shift registers in terms of scalability and reliability?
2. How does I2C support the concept of a single addressable node?
3. What are the physical limitations of I2C and how can they be mitigated?
4. What are the system’s power requirements, and is a dedicated regulator necessary?

---

## 4. I2C vs Shift Registers

I2C is a two-wire communication protocol that allows multiple devices to communicate over a shared bus using unique
hardware addresses (NXP Semiconductors, 2021). This significantly reduces wiring complexity and enables efficient
scaling.

In contrast, shift registers require sequential data transmission, meaning that the full data chain must be updated even
when only a single output changes. This increases CPU overhead and reduces efficiency in larger systems (Wevolver, 2023).

### 4.1 Table 1: Comparison of I2C and Shift Registers

| Feature         | I2C           | Shift Registers |
|-----------------|---------------|-----------------|
| Wiring          | 2 wires       | 3+ wires        |
| Addressing      | Device-based  | Positional      |
| CPU Load        | Low           | High            |
| Fault Tolerance | High          | Low             |
| Direction       | Bidirectional | Output only     |

### 4.2 Findings

I2C provides clear advantages for scalable systems, including reduced wiring complexity, direct device addressing, and
lower CPU load compared to shift-register cascades.

---

## 5. Single Addressable Node Concept

The concept of a single addressable node means that each bridge module behaves as one logical device on the
communication bus.

Using I2C, each module can be assigned a unique address, allowing the controller to communicate directly with individual
modules. If a device does not respond to its assigned address, this may indicate a communication failure at that node 
(NXP Semiconductors, 2021).

This allows faults to be isolated more efficiently compared to sequential architectures. Modular IoT systems benefit
from such addressable designs, as they enable independent node replacement without redesigning the entire system (Badii
et al., 2020).

This approach aligns with the modular City Hub architecture described in earlier work (Reus, 2026a).

### 5.1 Table 2: Characteristics of Addressable Nodes

| Property        | Impact               |
|-----------------|----------------------|
| Unique address  | Direct communication |
| Modular design  | Easy replacement     |
| Fault detection | Faster diagnostics   |
| Plug-and-play   | Scalable deployment  |

### 5.2 Findings

I2C enables the implementation of single addressable nodes, significantly improving maintainability and modular
scalability.

---

## 6. Scalability and Physical Constraints

A limitation of I2C is its communication range, which is constrained by bus capacitance and signal integrity. As the
number of connected devices and cable length increases, communication reliability decreases (All About Circuits, 2018).

However, this limitation can be mitigated through:

- I2C bus extenders
- Twisted-pair cabling
- Bus segmentation

These techniques allow I2C systems to scale beyond standard limitations while maintaining reliable communication (NXP
Semiconductors, 2021).

### 6.1 Findings

Although I2C has physical limitations, these can be effectively mitigated, making it suitable for modular bridge
applications.

---

## 7. Power Analysis

System reliability depends heavily on stable power delivery. Based on the power analysis, the system exhibits the
following peak current demands:

- **Actuator rail:** approximately 1460 mA peak
- **ESP32-S3:** approximately 240–350 mA peak
- **Sensor and logic rail:** approximately 200 mA peak

(Reus, 2026b)

### 7.1 Table 3: Power Consumption Overview

| System Part     | Peak Current |
|-----------------|--------------|
| ESP32-S3        | ~240–350 mA  |
| Actuators       | ~1460 mA     |
| Sensors & Logic | ~200 mA      |

Such dynamic loads can cause voltage drops and brownout conditions if not properly regulated. Sudden variations in load
current can cause temporary voltage deviations, which must be corrected quickly by the regulator to maintain stable
operation (Texas Instruments, 2000; NXP Semiconductors, 2021).

### 7.2 Findings

A dedicated on-board voltage regulator is required to ensure stable operation and protect the system from power-related
failures.

---

## 8. Synthesis

The findings show that an I2C-based bus architecture provides a strong foundation for building a scalable and modular
bridge system.

Compared to shift-register-based designs, I2C offers improved scalability, better fault isolation, and reduced system
complexity. In addition, the power analysis demonstrates that a dedicated power regulation strategy is necessary to
handle peak current demands and maintain stable system operation.

By combining:

- I2C-based addressable nodes
- Modular system design
- Dedicated power regulation

the bridge system can be developed into a robust and scalable architecture suitable for real-world smart infrastructure
applications.

---

## 9. References

* All About Circuits. (2018). *I2C design mathematics: Capacitance and resistance*. Retrieved April 23, 2026,
  from https://www.allaboutcircuits.com/technical-articles/i2c-design-mathematics-capacitance-and-resistance/
* Badii, C., Bellini, P., Cenni, D., Difino, A., & Nesi, P. (2020). *Smart city IoT platform respecting GDPR privacy and
  security aspects*. IEEE Access, 8, 55200–55216. https://doi.org/10.1109/ACCESS.2020.2982313
* Espressif Systems. (2026). *ESP32-S3 series datasheet (Version 2.2)*. Retrieved April 22, 2026,
  from https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
* Kaiser, Z. R. M. A., & Deb, A. (2025). *Sustainable smart city and Sustainable Development Goals (SDGs): A review*.
  Regional Sustainability, 6(1), 100193. https://doi.org/10.1016/j.regsus.2025.100193
* NXP Semiconductors. (2021). *UM10204 I2C-bus specification and user manual*. Retrieved April 22, 2026,
  from https://www.nxp.com/docs/en/user-guide/UM10204.pdf
* Texas Instruments. (2000). *Understanding the load-transient response of LDOs*. Retrieved April 23, 2026,
  from https://www.ti.com/lit/an/slyt151/slyt151.pdf
* Wevolver. (2023). *I2C vs SPI protocols: Differences, advantages, and use cases*. Retrieved April 23, 2026,
  from https://www.wevolver.com/article/i2c-vs-spi-protocols-differences-pros-cons-use-cases


* Reus, R. (2026a). [Analysis: Standardizing the modular City Hub architecture](https://gitlab.fdmci.hva.nl/studio/smart-cities/projecten/2025-2026-semester-2/city-sim-learning-group/city-smart-heaven-city-sim-learning-group/-/blob/3a7268202b115466cdf45af6927b4cc020d3f98b/docs/features/city-hub/city-hub-analysis.md).
  Retrieved April 23, 2026
* Reus, R. (2026b). [Power usage analysis and constraints](https://gitlab.fdmci.hva.nl/studio/smart-cities/projecten/2025-2026-semester-2/city-sim-learning-group/city-smart-heaven-city-sim-learning-group/-/blob/3a7268202b115466cdf45af6927b4cc020d3f98b/docs/features/bridge/power_analysis.md).
  Retrieved April 23, 2026