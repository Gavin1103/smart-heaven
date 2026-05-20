# Smart Drawbridge (Ophaalbrug)

> A fully automated Dutch ophaalbrug (drawbridge) powered by Rust Embassy, featuring dual-side detection and
> IR safety interlocks to balance boat and road traffic.

---

## 🌉 System overview

The Smart Drawbridge is a scale model of traditional Dutch infrastructure, designed to demonstrate safe, asynchronous
automation in a smart city context. It manages a critical junction where a North-South road crosses an East-West canal.

The system uses an ESP32-S3 to coordinate real-time sensor data, nautical signaling, and precise motor control to ensure
that neither road nor water traffic is ever at risk.

## 🚀 Key Features

- Dutch Nautical Standards: Implements authentic "Red + Green" transition signaling for vessels.
- Dual-Layer Safety: Combines IR beam-break sensors for under-bridge clearance with ultrasonic approach detection.
- Precise Motion: Uses a stepper-driven lift mechanism and servo-controlled barrier arms for reliable mechanical
  operation.
- Fail-Safe Design: Automatically halts and re-opens if an obstruction is detected during closure.

## 📂 Project Navigation

Select a section below to dive into the technical specifics of the build:

| Section                                                | Description                                                                 |
|--------------------------------------------------------|-----------------------------------------------------------------------------|
| [Architecture](./bridge_architecture.md)               | The software "brain": Async tasks, FSM logic, and the Rust/Embassy stack.   |
| [Technical Specs](./bridge_components_list.md)         | Master Pinout table, 5V/3.3V power rails, and the full Bill of Materials.   |
| [Wiring & power](./bridge_wiring_and_power.md)         | Electrical diagrams, sensor modules, and high-current actuator isolation.   |
| [Design & Safety](./bridge_design.md)                  | Finite State Machine (FSM) diagrams and the core safety "guards".           |
| [Bridge occupancy dicision](./bridge_advise.md)        | Document to advise major on which bridge occupancy detection system to use. |
| [Safety & Timeing](./bridge_safety_and_timing_spec.md) | How to measure safety goals and prototype hardware limitations.             |
| [Research](./bridge_research.md)                       | Real-world Dutch infrastructure standards and design justifications.        |
| [Roadmap](./bridge_roadmap.md)                         | Planned phases: PCB integration, telemetry, override API, signaling fix.   |
| [PCB Design](./bridge_pcb_design.md)                   | Bridge Node PCB: MCP23017, PCA9685PW, level shifting, connector mapping.   |

## 🚦 The Demo Experience

The bridge operates on a fully autonomous cycle triggered by approaching vessels:

1. **Detection:** HC-SR04 sensors pick up a boat approach.
2. **Securing:** Road lights flash, buzzers sound, and barrier arms drop.
3. **Preparation:** Boat signals transition to Red+Green while the system verifies the deck is clear.
4. **Operation:** The deck lifts; boat signals turn Green for passage.
5. **Recovery:** Once the IR beam is clear, the deck lowers and road traffic resumes.

## 🛠 Status & Metadata

- **Owner:** Rocco Reus
- **Tile:** T2 (Canal / Road Crossing)
- **Build stage:** Prototyping (Rust/Embassy)
- **Last updated:** 2026-03-17