# Railway Barrier - Design

Based on the analysis and advice, the railway barrier system is designed as a local-first embedded system.  
The ESP32 is responsible for all real-time decision-making, while the backend is only used for monitoring and logging.

## System overview

The system consists of the following main parts:

- **ESP32:** central controller of the system
- **Two ultrasonic sensors:** detect an approaching train and detect when the train has passed
- **Servo motor:** opens and closes the barrier
- **LED lights:** provide visual warning signals
- **Active buzzer:** provides audible warning signals
- **Push button:** enables manual override
- **Backend:** stores events and shows system status

The design follows a local-first approach. This means that detection, decision-making, and barrier control all happen on
the ESP32. As a result, the barrier can continue to operate safely even when the backend or network connection is
unavailable.

Although the analysis showed that combining different detection methods can further improve reliability, this prototype
uses two ultrasonic sensors because they are practical, available in the kit, and easier to integrate into a small-scale
setup.

## System behavior

The railway barrier is designed as a state-based system.  
Using a state machine makes the behavior predictable, easier to debug, and less sensitive to unstable switching. This
state-based approach ensures controlled transitions and prevents unsafe or unstable behavior.

The system uses the following states:

- **Idle:** barrier is open and no train is detected
- **Approaching:** a train is detected by Sensor A
- **Closing:** warning signals are active and the barrier is moving down
- **Closed:** the barrier is fully closed while the train is passing
- **Opening:** the train has passed and the barrier is moving up
- **Error:** the system detected an invalid or unsafe situation

## State transitions

The expected state flow is:

- **Idle → Approaching:** Sensor A detects an approaching train
- **Approaching → Closing:** the minimum warning time has passed
- **Closing → Closed:** the barrier has fully closed
- **Closed → Opening:** Sensor B detects that the train has passed
- **Opening → Idle:** the barrier has fully opened
- **Any state → Error:** an unexpected or unsafe condition occurs

In the error state, the barrier must default to the safest possible position: closed.

## Detection design

To improve reliability, the design uses two detection points:

- **Sensor A:** placed before the crossing to detect an approaching train
- **Sensor B:** placed after the crossing to detect when the train has passed

This design is more reliable than using only one sensor, because the system can distinguish between a train arriving and
a train leaving. It also reduces ambiguity in the control logic.

### Sensor placement considerations

The placement of Sensor A depends on the speed of the train and the total time needed for warning and barrier
movement.  
For this prototype, the required detection distance can be estimated as:

`distance = train speed × (warning time + closing time + safety margin)`

Sensor B is placed after the crossing to detect when the train has fully passed, so the barrier only opens when the
crossing is clear.

For example, if the train moves at 10 cm/s and the system requires a total of 6 seconds (3 seconds warning time, 2
seconds barrier closing time, and 1 second safety margin), Sensor A should be placed approximately 60 cm before the
crossing.

This ensures that the barrier is fully closed before the train reaches the crossing.

## Barrier control design

The barrier is controlled by the SG90 servo motor.  
The servo moves between two fixed positions:

- **Open position:** barrier up
- **Closed position:** barrier down

The servo should not move instantly from one position to another. Instead, it should move gradually in small angle
steps. This creates smoother movement and reduces unstable behavior.

## Warning design

Before the barrier closes, the system should activate warning signals:

- **LED lights:** flash to warn users that the barrier is about to close
- **Active buzzer:** produces an audible warning signal

These warning signals remain active while the barrier is closing and while the crossing is blocked.

## Timing considerations

The system must ensure a minimum warning time between detecting an approaching train and closing the barrier. This delay
allows users to react safely before the barrier starts moving.

A minimum warning time of approximately 3–5 seconds is recommended to balance safety and usability. Too short warning
times may be unsafe, while too long warning times may reduce user trust in the system.

## Manual override design

The push button is used as a manual override input.  
When pressed, it allows the operator to interrupt the automatic process and manually request the barrier to open or
close, depending on the current system state.

Because the push button is not a physical toggle switch, the system uses software logic to interpret button presses and
handle manual actions safely within the state machine.

## Communication design

The ESP32 sends status updates and events to the backend via Wi-Fi.  
Examples of events are:

- train detected
- barrier closing
- barrier closed
- train passed
- barrier opening
- error detected

The backend is not part of the control loop. It only receives data for monitoring and logging. This ensures that the
railway barrier remains operational even if the backend is unavailable.

## Safety design decisions

The following safety decisions are included in the design:

- The barrier is controlled locally on the ESP32
- The barrier defaults to closed when an error occurs
- Sensor input is validated to reduce false positives
- State transitions are controlled through a defined state machine
- Manual override is always available
- Warning signals are activated before barrier movement begins

These choices support a safe and reliable system design.

## Final design choice

The final design consists of:

- ESP32 as the local controller
- Two ultrasonic sensors for train detection
- SG90 servo motor for barrier movement
- LED lights for visual warning
- Active buzzer for audible warning
- Push button for manual override
- Backend for monitoring and logging only

This design supports the functional, safety, and technical requirements identified in the analysis and follows the
advice formulated for this prototype.