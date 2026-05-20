# Smart Drawbridge (Ophaalbrug) – Architecture

## 1. Architectural Philosophy: Layered Trust

The bridge logic is built on the principle of **Layered Trust**, ensuring that no action is taken based on a single
sensor reading alone.

1. **Commanded Movement:** The stepper is instructed to move.
2. **Measured Movement:** The **KY-040 Encoder** confirms that physical movement actually occurred to detect stalls or
   slips.
3. **Physical Endpoint:** The **KY-025 Reed Switch** provides a hard physical confirmation that the bridge is closed.
4. **Occupancy Verification:** **HX711 Weight Pads** and **IR Obstacle sensors** provide a final safety check before any
   motion begins.

---

## 2. Software Structure (Rust/Embassy)

The system is divided into modular, asynchronous tasks to ensure timing safety and responsiveness.

- **Coordinator Task:** The central Finite State Machine (FSM) that makes high-level transition decisions.
- **Sensor Task:** Polls the ultrasonic, reed, IR, and weight sensors to update a shared system state.
- **Motion Task:** Handles the low-level stepping logic for the bridge deck and PWM for barrier servos.
- **Signaling Task:** Manages LED patterns via the shift register and controls the audible buzzer.

---

## 3. Subsystem Interaction

- **Logic Expansion:** To conserve GPIO, a **SN74HC595N Shift Register** handles all LED outputs. The FSM requests
  logical states (e.g., `boat_go()`) rather than manipulating raw bits.
- **Safety Interlocks:** Any task can trigger a **Fault State** if a timeout is reached or a movement mismatch is
  detected by the encoder.

> For specific pin mappings and electrical details, see the **[Technical Specs](./bridge_components_list.md)**.