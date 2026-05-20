# Railway Barrier - Analyze

## Goal

For this learning goal, I want to create a safe and reliable railway barrier.  
To achieve this, I researched what makes a railway barrier reliable and safe.  
Based on this research, I translated the findings into functional, technical, and safety requirements for a Smart City
railway barrier system.

## Functional requirements

- Detect when a train is approaching and when it has passed
- Automatically close and open the barrier
- Provide visual warning signals (LED)
- Provide audible warning signals (buzzer)
- Support manual override (buttons)
- Show system status (serial / backend)
- Log events (e.g. train detected, barrier closed)

(Crump, 2024)

## Reliability & safety

- Fail-safe: barrier closes on errors
- Stable behavior (no rapid switching)
- Balanced warning time (too much warning time → impatient users → distrust in the barrier)
- Manual control must always be possible

(Crump, 2024)

## Components that can be used

To explore possible solutions for a safe and reliable railway barrier, I analyzed which components from the kit could
potentially be used in the system. The final selection will be justified in the advice section.

- **Microcontroller (ESP32 S3/C3):**  
  The ESP32 acts as the main controller of the system. It processes sensor data, controls the barrier, and communicates
  with the backend via Wi-Fi. It supports real-time processing and allows the system to operate independent of the
  backend.

- **LED lights:**  
  LEDs are used to provide visual warning signals to indicate that the barrier is about to close or is closing. They can
  be controlled to flash in patterns, making warnings more noticeable for users.

- **Active and passive buzzer:**  
  Buzzers provide audible warning signals. An active buzzer produces sound with a simple signal, while a passive buzzer
  requires signal modulation to generate sound (Lambert, 2024). This makes active buzzers simpler to use, while passive
  buzzers allow more flexible sound patterns.

- **Push button:**  
  A push button can be used for manual override, allowing the user to open or close the barrier in case of an emergency.
  It only sends a signal when pressed, which requires software logic to handle state changes.

- **Stepper motor:**  
  A stepper motor moves in small steps and allows precise control over position, which can be used to control the
  barrier (De Bakker, 2025). It is suitable for applications that require precise positioning and controlled movement.

- **SG90 mini servo:**  
  A servo motor can rotate to a specific angle, making it suitable for controlling a barrier that moves between fixed
  positions (Pinout, 2023). It is easy to control using PWM signals and is commonly used in small embedded projects.

- **Ultrasonic sensor:**  
  The ultrasonic sensor measures distance using sound waves and can detect the presence of an approaching train (Sarraf,
  2026). It works by sending out a sound pulse and measuring the time it takes for the echo to return.

- **Laser / LDR (photoresistor):**  
  A laser combined with an LDR can be used as a beam-break sensor. When the laser beam is interrupted, the system can
  detect that an object (train) is passing (Agarwal, 2024). However, this setup is sensitive to ambient light and
  requires precise alignment.

---

## Detection methods analysis

To determine the most reliable way to detect an approaching train, multiple detection methods were analyzed and compared
based on reliability, complexity, and suitability for a small-scale Smart City prototype.

### Ultrasonic sensor

The ultrasonic sensor detects objects by measuring distance using sound waves (Sarraf, 2026).

- **Advantages:**
  - Simple to implement
  - Works without precise alignment
  - Suitable for detecting objects at short range

- **Disadvantages:**
  - Can detect unwanted objects (false positives)
  - Accuracy depends on angle and surface of the object
  - Limited reliability as a single source of detection

---

### Laser + LDR (beam-break sensor)

A laser combined with an LDR detects when a light beam is interrupted by an object (Agarwal, 2024).

- **Advantages:**
  - Very precise (binary detection: blocked / not blocked)
  - Reliable for detecting when a train passes a fixed point

- **Disadvantages:**
  - Requires precise alignment
  - Sensitive to ambient light conditions
  - Only detects at a single point (not distance-based)

---

### Track circuits (real-world railway systems)

In real railway systems, trains are often detected using **track circuits**.  
A track circuit works by sending an electrical current through the rails. When a train is present, its metal wheels and
axles complete the circuit, allowing the system to detect that a section of track is occupied (Crump, 2024).

- **Advantages:**
  - Very high reliability
  - Specifically designed for train detection
  - Widely used in real railway infrastructure
  - Fail-safe by design (loss of signal → system assumes track is occupied)

- **Disadvantages:**
  - Complex to implement
  - Requires direct integration with railway infrastructure
  - Not suitable for small-scale prototypes like this project

---

### Sound sensor

A sound sensor detects noise levels, which could indicate an approaching train.

- **Advantages:**
  - Can detect objects from a distance
  - Does not require physical alignment

- **Disadvantages:**
  - Highly unreliable due to environmental noise
  - Difficult to distinguish train sounds from other sounds

---

### Motion sensor (PIR)

A PIR sensor detects movement based on infrared radiation.

- **Advantages:**
  - Easy to use
  - Detects movement without distance measurement

- **Disadvantages:**
  - Not specific to trains
  - High chance of false positives (people, animals)

---

## Comparison

| Method            | Reliability | Complexity | Accuracy | Suitability |
|------------------|------------|------------|----------|------------|
| Ultrasonic       | Medium     | Low        | Medium   | High       |
| Laser + LDR      | High       | Medium     | High     | High       |
| Track circuits   | Very High  | High       | Very High| Low        |
| Sound sensor     | Low        | Low        | Low      | Low        |
| PIR sensor       | Low        | Low        | Low      | Low        |

---

## Key insight

- No single sensor provides fully reliable detection
- Reliability is achieved by combining multiple sensors (redundancy)
- Real-world systems use infrastructure-based detection, while prototypes rely on external sensors

---

## Technical requirements

The following technical requirements are derived from the functional and safety requirements described above (Crump,
2024).

- The system must continuously read and process sensor data to detect an approaching train with minimal delay
- The system must reliably detect a train under different conditions (e.g. distance, angle, or environmental noise)
- The system must debounce and validate sensor input to prevent false positives or noise
- The ESP32 must process all sensor data locally without relying on external systems
- The barrier must transition between states using a defined state machine (e.g. idle → closing → closed → opening)
- The servo/stepper motor must move gradually to prevent sudden or unstable movements
- The system must ensure a minimum warning time before closing the barrier
- The system must prevent rapid state switching (e.g. open/close loops)
- The system must default to a safe state (barrier closed) when an error occurs
- The system must remain operational without a backend connection
- The system must support manual override input at all times

## Key insight

- Real-time decision-making must happen locally on the ESP32 to ensure reliability and low latency
- The backend should not be part of the control loop, but only used for monitoring and logging

# References

Crump, W. (2024, January 16). *How railroad crossings work — Practical engineering*. Practical Engineering.  
https://practical.engineering/blog/2023/12/29/how-railroad-crossings-work

What Are Level Crossings and How to Drive Through Them Safely? (n.d.). Evans Halshaw.  
https://www.evanshalshaw.com/blog/how-do-level-crossings-work/?srsltid=AfmBOorh89qrFSM8WoNcMv_nnzlNL347oP1KEj5jxKlJY31G-b1yZ8Ek

Pinout. (2023, August 17). SG90 PDF Datasheet - Micro Servo Motor - Tower pro.
DatasheetCafe. https://www.datasheetcafe.com/sg90-pdf-23123/

De Bakker, B. (2025, 26 december). 28BYJ-48 Stepper Motor with ULN2003 Driver and Arduino Tutorial.
Makerguides.com. https://www.makerguides.com/28byj-48-stepper-motor-arduino-tutorial/

Light Sensor: Advantages and Disadvantages. (z.d.). RF Wireless
World. https://www.rfwireless-world.com/terminology/light-sensor-advantages-disadvantages

Agarwal, N. (2024, 11 juni). LDR - Advantages and Disadvantages (Limitations). Electronics For You – Official Site
ElectronicsForU.com. https://www.electronicsforu.com/technology-trends/learn-electronics/ldr-advantages-disadvantages

Sarraf, G. (2026, 5 januari). How Does an Ultrasonic Sensor Detect Distance: Technical Guide.
ThinkRobotics.com. https://thinkrobotics.com/blogs/learn/how-does-an-ultrasonic-sensor-detect-distance-technical-guide?srsltid=AfmBOop_nvv_3F4vSglWc2o8QEGeWWUJ6JJ-f5QzYiuG00vWHr2sdlnI

Lambert, G. (2024, 11 november). Active Buzzers vs Passive Buzzers. Circuit
Basics. https://www.circuitbasics.com/what-is-a-buzzer/