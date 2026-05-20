# Power Usage Analysis and Constraints

## 1. Power Supply Overview

The system is powered by a Keyestudio KS0534 Power Module. This module is designed to plug into a breadboard and provide
dual-rail output (3.3V and 5V).

- **Input Source Recommendation:** While the KS0534 supports 5V Type-C input, a 9V DC Power Adapter is used to ensure
  the AMS1117 regulators have sufficient voltage "headroom" (dropout voltage) to maintain a stable 5V output under motor
  load. This prevents the voltage "sags" that typically occur when providing exactly 5V to a linear regulator.
- **Module Output Limit:** 3.3V: 500mA(Max); 5V: 500mA(Max)
- **Regulation:** Uses linear regulators (AMS1117) which dissipate excess voltage as heat.

## 2. Theoretical Peak Consumption

To ensure the system remains stable, the "Worst Case Scenario" (all components active/stalled simultaneously) must be
calculated.

### 2.1 5V Actuator Rail (Peak Demand)

| Component        | Status       | Estimated Peak (mA) |
|------------------|--------------|---------------------|
| SG90 Servo (1)   | Stall/Inrush | ~600mA              |
| SG90 Servo (2)   | Stall/Inrush | ~600mA              |
| 28BYJ-48 Stepper | Active Step  | ~200mA              |
| HC-SR04 (x2)     | Pinging      | ~30mA               |
| Active Buzzer    | Alerting     | ~30mA               |
| Subtotal (Peak)  |              | ~1460mA             |

### 2.2 Logic Power (Source: Independent USB-C)

| Component | Status         | Estimated Peak (mA)           |
|-----------|----------------|-------------------------------|
| ESP32-S3  | RF/WiFi Active | ~240mA                        |
| Status    | Stable         | Independent of Actuator load. |

### 2.3 3.3V Breadboard Rail (Source: KS0534)

| Component               | Status      | Estimated Peak (mA)   |
|-------------------------|-------------|-----------------------|
| SN74HC595 + 8 LEDs      | All LEDs On | ~160mA (20mA per LED) |
| Sensors (IR, Reed, Enc) | Active      | ~50mA                 |
| HX711 + Load Cells      | Sampling    | ~5mA                  |
| Subtotal (Peak)         |             | ~215mA                |

## 3. Identified Constraints and Risks

### 3.1 Total Current Overflow

The 5V Actuator Rail theoretical peak (~1460mA) is 2.9 times the 500mA limit.

- **Risk:** While the actuators may stall or "stutter" due to current starvation, the ESP32-S3 will not reset because it
  is on a separate USB-C power domain.
- **Brownout Potential:** Only affects the 5V/3.3V breadboard rails; the MCU remains active to detect and log the
  failure.

This isolation allows the Rust Embassy executor to maintain the state of the FSM and perform a controlled 'Safe
Shutdown' or 'Fault' transition even if the peripheral power rail collapses due to an actuator stall.

### 3.2 Thermal Dissipation

Drawing near 500mA continuously will cause the AMS1117 regulators on the power module to become extremely hot. If they
overheat, they will enter thermal shutdown, cutting power to the bridge.

## 4. Mitigation Strategy (Software Interlocks)

Because the hardware cannot support a full simultaneous load, the Rust Embassy FSM is designed with Sequential
Activation Guards:

1. **Staggered Inrush:** Servos are moved one at a time using Timer::after().await to ensure their peak inrush currents
   do not overlap.
2. **Mutual Exclusion:** The software prevents the Stepper Motor from engaging until the LowerBarriers state (Servos) is
   fully complete and the servos have entered a low-power "holding" state.
3. **Brownout Detection:** The ESP32-S3 internal brownout detector is enabled to safely halt the FSM if voltage drops
   below a safe threshold.