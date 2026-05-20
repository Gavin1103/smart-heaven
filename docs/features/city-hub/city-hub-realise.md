# Realise: City Hub Bus Prototype Implementation

**Author:** Rocco Reus – Embedded & Robotics Engineer  
**Date:** April 4, 2026  
**Version:** 1.0

---

## 1. Context of the Evidence

This evidence represents the realization of a small-scale City Hub prototype.

The goal is to validate the core architectural concepts of the Smart City system—specifically the shared communication
bus—using available hardware (ESP32-C3) before moving to the final ESP32-S3 integration.

Specifically, this prototype focuses on:

- Shared bus communication (I2C)
- Multi-device addressing (MPU-6050 and OLED)
- Rust-based hardware interaction using the Embassy framework

---

## 2. Who Created the Evidence

- Author: Rocco Reus
- Role: Embedded & Robotics Engineer
- Contribution: Design and implementation of the City Hub bus prototype

---

## 3. Goal of the Evidence

The goal of this evidence is to demonstrate that:

- Multiple devices (Sensors and Displays) can share a single communication bus
- Rust (Embassy) can reliably scan, detect, and communicate with multiple devices on the same lines
- The logic and bus architecture are transferable between different ESP32 variants (C3 to S3)

---

## 4. Introduction

This prototype serves as a proof-of-concept for the City Hub architecture. Instead of connecting all devices directly
via GPIO, a shared I2C bus is used. This reduces pin usage and allows the system to scale as more "tiles" or modules are
added to the Smart City.

---

## 5. Implementation Overview

### Hardware Setup

The prototype consists of:

- ESP32-C3 (Used as a functional stand-in for the S3)
- I2C bus (Shared SDA / SCL lines)
- MPU-6050 (6-axis IMU)
- OLED Display (SSD1306 via I2C)

---

### Software Setup

The system is implemented in Rust using the Embassy framework.

Key features:

- Asynchronous I2C driver implementation
- Bus Scanner script: A custom routine that iterates through I2C addresses to identify connected hardware
- Real-time output: Serial logging of detected device addresses

---

## 6. Implementation Process

### Step 1 – Bus Setup

- SDA and SCL pins configured on the ESP32-C3
- Bus initialized within the Embassy async executor

---

### Step 2 – Device Integration

- The MPU-6050 and OLED were wired in parallel to the same bus
- Verified that both devices have unique hardcoded I2C addresses to avoid collisions

---

### Step 3 – Scanner Script Development

- Developed a Rust script to "ping" every possible address on the bus
- Logged successes to the console to verify hardware connectivity

---

## 7. Results

---

### 7.1 Bus Detection (Scanner)

#### 🎥 What am I watching?

This video shows the ESP32-C3 running the I2C scanner script. You can see the serial monitor identifying the specific
hex addresses for the MPU-6050 and the OLED display.

<video controls width="70%">
  <source src="../../assets/videos/rocco/city-hub-prototype.mp4" type="video/mp4">
</video>

---

#### Observed behavior

- MPU-6050 detected at its expected address (typically 0x68)
- OLED detected at its expected address (typically 0x3C)
- The script confirms that the shared bus is electrically sound and that the software can distinguish between multiple
  nodes

---

## 8. Validation of Design

The prototype confirms that:

- **I2C effectively reduces GPIO usage:** Two devices are handled using only two data pins
- **Hardware Agnostic Logic:** The I2C implementation on the C3 successfully validates the architecture intended for the
  S3
- **Rust Reliability:** The Embassy framework handles the shared bus without timing conflicts between the sensor and the
  display

---

## 9. Reflection

- Hardware Availability: Using the C3 was a successful pivot that allowed development to continue without waiting for S3
  stock
- Scalability: The ease of adding the OLED alongside the MPU-6050 proves that the City Hub can expand simply by adding
  addresses to the bus

---

## 10. Next Steps

- Migration: Port the code to the ESP32-S3 once available
- Data Passthrough: Move from just "detecting" the devices to displaying real-time MPU-6050 data on the OLED screen

---

## 11. Conclusion

The use of the ESP32-C3 provided a perfect testbed for the City Hub's communication layer. By successfully scanning and
identifying the MPU-6050 and OLED on a shared bus, we have validated that the system's core communication architecture
is ready for full-scale integration.