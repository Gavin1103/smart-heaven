# Research: Dutch Bridge Infrastructure and Safety

In the Netherlands, movable bridges (*beweegbare bruggen*) are critical components of the national "Smart City" grid,
managed by regional authorities and **Rijkswaterstaat**. This project implements a scaled automation system that mirrors
the safety and signaling logic used in these real-world systems.

---

## 1. Management and Traffic Philosophy

Bridge software in the Netherlands must balance two competing priorities:

- **Waterway Vitality:** Ensuring vessels can pass without unnecessary delay.
- **Road Congestion:** Implementing "cooldown" periods between openings to prevent gridlock in urban
  North-South/East-West corridors.

**Design Justification:** Our system uses a `CLEARANCE_TIMEOUT` and boat approach detection to ensure the bridge stays
open only as long as necessary, returning priority to road traffic immediately after passage.

---

## 2. Nautical Light Signals

To maintain safety on Dutch waterways, signals follow strict inland maritime standards:

| Signal          | Meaning                                     | Project Implementation                                      |
|:----------------|:--------------------------------------------|:------------------------------------------------------------|
| **Single Red**  | Stop; passage prohibited.                   | Default state when the bridge is closed or closing.         |
| **Red + Green** | Prepare to sail; bridge is engaging motors. | Active during the `WaterPrepRedGreen` and `Opening` states. |
| **Green**       | Passage permitted; bridge is fully secured. | Active only when the stepper confirms `FULL_ROTATION`.      |

---

## 3. Safety Interlocks & Fail-Safes

Real-world Dutch bridges utilize "Interlock" systems to prevent mechanical movement while the deck or waterway is
occupied.

### 3.1 Under-Bridge Clearance

Just as Rijkswaterstaat uses sensors to prevent closing on a vessel, this project utilizes a **KY-032 IR Obstacle sensor
** mounted 2cm above the water line. If this beam is broken during the `Closing` phase, the FSM triggers an immediate
re-opening to the `OpenMonitoring` state.

### 3.2 Deck Occupancy

To ensure no vehicles or pedestrians are trapped on the movable section, real bridges use weight sensors or induction
loops. We mimic this using **half-bridge weight pads and the HX711** to verify the deck is clear before the `Opening`
sequence begins.

---

## 4. Technical Framework Choice: Rust & Embassy

The decision to use **Rust (Embassy)** on an **ESP32-S3** was driven by the need for "Timing-Safe" architecture.

- **Asynchronous Logic:** Allows the system to monitor safety sensors (like the IR Beam) in real-time even while driving
  high-precision stepper motors.
- **Memory Safety:** Prevents common software faults that could lead to "Lost Logic," ensuring the bridge always
  defaults to a safe "Bridge Closed / Road Open" state upon reboot.
