# Technical Specifications & Master BOM

This document serves as the single source of truth for all hardware components, electrical connections, and power
requirements for the Smart Drawbridge.

---

## 1. Bill of Materials (BOM)

The bridge utilizes a modular set of components to balance precise movement with reliable safety sensing.

### 🧠 Control & Logic

| Part           | Quantity | Purpose                                                   |
|:---------------|:--------:|:----------------------------------------------------------|
| **ESP32-S3**   |    1     | Central controller running async Rust/Embassy.            |
| **SN74HC595N** |    1     | 8-bit Shift Register for LED expansion to save GPIO pins. |

### ⚙️ Actuators (Motion)

| Part                 | Quantity | Purpose                                              |
|:---------------------|:--------:|:-----------------------------------------------------|
| **28BYJ-48 Stepper** |    1     | Bridge deck lift mechanism (Precise positioning).    |
| **ULN2003 Driver**   |    1     | Current Darlington array to drive the stepper motor. |
| **SG90 Micro Servo** |    2     | North and South road barrier arms.                   |

### 👁️ Sensors (Feedback)

| Part                   | Quantity | Purpose                                              |
|:-----------------------|:--------:|:-----------------------------------------------------|
| **HC-SR04**            |    2     | Ultrasonic boat approach detection (East/West).      |
| **KY-032 IR Obstacle** |    1     | Under-bridge clearance safety interlock.             |
| **KY-040 Encoder**     |    1     | Rotary feedback for stall detection and positioning. |
| **KY-025 Reed Module** |    1     | Magnetic limit switch for "Home" (closed) position.  |
| **HX711 + Load Cells** |  1 set   | Weight-based bridge deck occupancy detection.        |

---

## 2. Pinout Master Table

All signals are referenced to the ESP32-S3 GPIO mapping.

| Module             | Signal             |      GPIO      | Logic Level | Power Rail  |
|:-------------------|:-------------------|:--------------:|:-----------:|:------------|
| **Stepper Motor**  | IN1, IN2, IN3, IN4 | 15, 16, 17, 18 |    3.3V     | 5V External |
| **Shift Register** | SER, SRCLK, RCLK   |    6, 7, 8     |    3.3V     | 3.3V VCC    |
| **Shift Register** | Output Enable (OE) |       9        |    3.3V     | 3.3V VCC    |
| **Boat Sonar (E)** | Trigger / Echo     |     21 / 4     |    3.3V*    | 5V / 3.3V   |
| **Boat Sonar (W)** | Trigger / Echo     |    38 / 39     |    3.3V*    | 5V / 3.3V   |
| **Servo Barriers** | PWM (Left / Right) |     5 / 10     |    3.3V     | 5V External |
| **HX711 (Weight)** | Data / Clock       |    41 / 42     |    3.3V     | 3.3V VCC    |
| **Encoder**        | A, B, Button       |    1, 2, 3     |    3.3V     | 3.3V VCC    |
| **IR Safety Beam** | Signal Output      |       11       |    3.3V     | 3.3V VCC    |
| **Reed Switch**    | Signal Output      |       40       |    3.3V     | 3.3V VCC    |
| **Alarm Buzzer**   | Control Pin        |       12       |    3.3V     | 5V External |

> **Note:** Sonar Echo lines require a voltage divider (1kΩ/2kΩ) to step 5V signals down to 3.3V for the MCU.

---

## 3. Power Architecture

To prevent brownouts and signal noise, the system uses isolated power rails.

### 3.1 Rail Distribution

Both power rails come from the power module.

- **Logic Rail (3.3 V):** Powers the high-precision sensors (HX711, Encoder, Reed).
- **Actuator Rail (5 V):** Powers the high-current loads including the Stepper, Servos, and Buzzer

### 3.2 Passive Safety Components

- **LED Protection:** 220 Ω - 330 Ω resistors on all `74HC595` outputs.
- **Decoupling:** 100uF–470uF capacitor across the 5 V rail to absorb motor spikes.
- **Common Ground:** All GND pins (MCU, 5V supply, Drivers) are tied together for a stable reference.

---

## 4. Shift Register Output Mapping (SN74HC595N)

| Output Pin | Signal         | Target Traffic  |
|:----------:|:---------------|:----------------|
|   **Q0**   | Red LED        | Road Traffic    |
|   **Q1**   | Green LED      | Road Traffic    |
|   **Q2**   | Yellow/Flasher | Road Traffic    |
|   **Q3**   | Red LED        | Water Traffic   |
|   **Q4**   | Green LED      | Water Traffic   |
|   **Q5**   | Status LED 1   | System Internal |
|   **Q6**   | Status LED 2   | System Internal |
|   **Q7**   | Fault LED      | System Internal |