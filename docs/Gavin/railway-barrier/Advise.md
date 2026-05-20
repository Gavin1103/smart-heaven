# Railway Barrier - Advise

Based on the analysis, the following advice is formulated to create a safe and reliable railway barrier system.

## Recommended architecture

- ESP32 as the main controller for real-time decision-making
- Backend for monitoring and logging only
- Local-first design to ensure reliability and low latency

This architecture ensures that the system can operate independently of external systems, which is essential for
safety-critical behavior. This aligns with the analysis, where real-time processing and fail-safe behavior were identified
as key requirements.

---

## Hardware choices

### Servo motor (SG90) over stepper motor

For this prototype, the SG90 mini servo is the most suitable choice for controlling the barrier. The barrier only needs
to move between a limited number of fixed positions, making a servo more practical than a stepper motor.

A servo is easier to control, requires less setup, and introduces less complexity, which improves reliability. A stepper
motor offers more precise control, but this level of precision is not required for this use case.

---

### LED

LED lights are recommended for visual warnings. Flashing white lights can be used to signal users to stop, followed by
red flashing lights when the barrier is closing.

This provides clear and recognizable visual feedback and supports the functional requirement of providing warning signals.

---

### Push button

A push button is used to implement manual override in case of an emergency. Although a physical toggle switch could
provide a clearer user state, a push button combined with software logic can simulate toggle behavior effectively.

This approach reduces hardware complexity while maintaining full control functionality, while ensuring that manual
control is always available as required in the safety analysis.

---

### Active buzzer over passive buzzer

An active buzzer is recommended for audible warnings. It is simple to control and reliable, as it only requires a
digital signal to produce sound.

A passive buzzer would allow more complex sound patterns, but this is not necessary for this prototype and would
introduce unnecessary complexity.

---

## Detection strategy

### Primary detection: Ultrasonic sensor

Based on the detection methods analysis, the ultrasonic sensor is recommended as the primary detection method.

It provides a good balance between simplicity, reliability, and suitability for a small-scale prototype. It does not
require precise alignment and can detect objects at a distance, making it effective for detecting an approaching train.

---

### Secondary detection (optional): Laser + LDR

To increase reliability, a secondary detection method such as a laser/LDR beam-break sensor can be added.

From the analysis, it was concluded that no single sensor is fully reliable. By combining multiple sensors, the system
can validate detection results and reduce the risk of false positives or missed detections.

This introduces redundancy, which aligns with fail-safe principles used in real-world railway systems.

---

### Dual detection points

To improve system clarity and reliability, two detection points are recommended:

- Sensor A → detects approaching train → barrier closes
- Sensor B → detects that the train has passed → barrier opens

This reduces ambiguity and ensures correct state transitions.

---

### Real-world comparison

In real-world railway systems, track circuits provide the most reliable method for train detection, as they directly
detect whether a section of track is occupied.

However, implementing track circuits is not feasible in this prototype due to hardware limitations and the absence of
physical railway infrastructure.

Therefore, this system uses external sensors (such as ultrasonic and optional beam-break detection) to approximate
similar behavior. By combining multiple sensors, the system increases reliability and partially compensates for the lack
of infrastructure-based detection.

---

## Software approach

To ensure predictable behavior, easier debugging, and to prevent unstable switching, a state machine is recommended.

The system should include the following states:

- Idle
- Approaching
- Closing
- Closed
- Opening
- Error

This structure ensures controlled transitions and supports fail-safe behavior, as identified in the analysis.

---

## Communication

To ensure reliable communication with the backend:

- ESP32 communicates with the backend via Wi-Fi
- Direct API communication is sufficient for this prototype
- The backend is used for logging and monitoring only

The backend is not part of the control loop, ensuring that system operation is not affected by network issues. This
follows the local-first principle defined in the analysis.

---

## Safety strategy

- Fail-safe: the barrier defaults to a closed state in case of errors
- Sensor input must be validated to prevent noise and false triggers
- Timing constraints ensure sufficient warning time before closing
- The system must prevent rapid state switching
- Manual override must always be available

These measures ensure stable and safe system behavior under different conditions.

---

## Final advice (short)

Best solution:

- ESP32 + servo motor
- 2 Ultrasonic sensor
- Two detection points (approach + pass)
- State-based logic (finite state machine)
- Local-first control
- Backend for monitoring only

This combination provides a balance between simplicity, reliability, and safety, while aligning with real-world
fail-safe principles.