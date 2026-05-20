# Bridge Firmware — How It Was Built

The bridge is the first completed module in `smart_city_v2` and serves as the **reference implementation** for how all city modules should be structured. This page explains the design decisions behind the code — not just what it does, but why it works the way it does.

For the full technical spec (pin tables, timing constants, hardware test keys), see the [BridgeSystem README](https://gitlab.com/city-smart-heaven/city-sim-learning-group/-/blob/main/apps/embedded/smart_city_v2/lib/BridgeSystem/README.md) in the repo.

---

## Why a 14-state FSM?

The bridge has a strict physical sequence: you cannot open a bridge while road traffic is still moving through, and you cannot close it while a boat is still underneath. A simple `if/else` chain can't enforce these dependencies safely, and it becomes impossible to reason about edge cases.

The FSM makes every valid transition explicit. Each state has one job, a defined timeout, and a clear set of exit conditions. If anything goes wrong (sensor contradiction, motor stall, timeout), any state can jump directly to `Fault` — and `Fault` has a single, manual recovery path (encoder button → `Homing`). This makes the failure mode predictable, which matters for a physical demo.

```
Startup → Homing → IdleClosed
                       │
              (boat detected, 1 s confirmed)
                       ↓
               BoatDetected
                       ↓
           WarningRoadTraffic   (3 s flashing yellow)
                       ↓
            WaitBridgeClear     (road sensors must clear)
                       ↓
              LowerBarriers
                       ↓
           WaterPrepRedGreen    (2 s Dutch nautical signal)
                       ↓
                  Opening       (6 400 stepper steps)
                       ↓
                    Open        (min 30 s + waterway clear)
                       ↓
          ClosingCheckClearance (2 s final safety window)
                       ↓
                  Closing       (until reed switch triggers)
                       ↓
              RaiseBarriers → IdleClosed

Any state → Fault   (timeout / sensor contradiction / stall)
Fault → Homing      (encoder button to recover)
```

---

## Layered trust — why so many sensors?

No single sensor is trusted on its own. This is intentional:

- **HC-SR04 sonar (×2, east + west)** — detects boat approach. Two sensors because boats can come from either direction.
- **KY-032 IR obstacle** — confirms the road under the bridge is clear before the deck moves. Sonar alone can't see directly beneath the structure.
- **KY-025 reed switch** — physical confirmation that the bridge is fully closed. The stepper counts steps, but steps ≠ physical position if there's a slip or stall.
- **KY-040 rotary encoder** — confirms the stepper is actually moving. A motor can receive signals without turning; the encoder catches stalls that would otherwise be invisible.
- **HX711 load cell** — weight detection as a secondary occupancy check before closing. A car stopped on the bridge wouldn't always block the IR beam, but it would register on the weight sensor.

Each sensor layer catches a different failure mode. Together they implement what the architecture docs call **Layered Trust** — the bridge can only move when all relevant sensors agree it's safe to do so.

---

## Why the subsystem split?

The code is divided into four classes: `BridgeController`, `BridgeSignaling`, `BridgeMotion`, and `BridgeSensors`. This split was deliberate:

- **`BridgeController`** knows the FSM and nothing else. It calls semantic methods: `setRoadRed()`, `isBoatDetected()`, `openDeck()`. It never touches GPIO.
- **`BridgeSignaling`**, **`BridgeMotion`**, **`BridgeSensors`** own GPIO. They implement the semantic API the controller needs.

The result: changing a pin assignment is a one-line edit in a leaf class. Adding a new sensor doesn't touch the FSM. The controller reads like a state diagram, not a wiring harness. This pattern is required for all `smart_city_v2` modules.

---

## Non-blocking sensors — why not just `pulseIn()`?

`pulseIn()` and `delay()` block the FreeRTOS task. With a 100 Hz update loop (10 ms budget), a 30 ms `pulseIn()` call for the HC-SR04 would immediately miss its deadline and starve the rest of the bridge logic.

Instead, every sensor runs as its own mini-FSM cycling through `Idle → WaitEchoStart → WaitEchoEnd → Cooldown` using `micros()` timestamps. All sensors require **N consecutive agreeing readings** before updating their confirmed state — this debounces noise without adding any blocking delay.

The HX711 load cell uses a 30-sample tare on startup and an exponential moving average (EMA) during operation to filter drift. `tareLoadCell()` is called automatically each time the bridge returns to `IdleClosed`, keeping the baseline fresh.

---

## Power interlock — stepper + servos on a shared rail

The stepper motor and both servo motors share a single 5 V / 500 mA rail. Running all three simultaneously exceeds the rail's current budget and causes brownouts. `BridgeMotion` enforces mutual exclusion: the stepper will not step while `isBarrierMoving()` is true, and the FSM never starts a barrier move while the stepper is running.

Servos are also **detached** (PWM signal removed) when they're not actively moving. This prevents holding current from draining the rail while the bridge is sitting idle. A 200 ms stagger between the left and right servo (`BARRIER_STAGGER_MS`) prevents both from stalling simultaneously on the same inrush spike.

If your module has multiple actuators on a shared power rail, add a similar guard.

---

## Hardware tests — why a separate build?

The `hwtest` build (`src/main_hwtest.cpp`) is a standalone interactive wiring validator flashed before the full production firmware. Open Serial at 115 200 baud and press a key:

| Key | Test |
|:----|:-----|
| `s` | Stepper — 500 steps forward + back; PASS if encoder offset ≤ ±5 |
| `b` | Barriers — left then right servo open/close cycle |
| `l` | Lights — cycles all 8 shift register outputs + buzzer |
| `i` | Inputs — prints reed / IR / weight states every 500 ms for 5 s |
| `v` | VarServo — sweeps each servo 0 → 180° for angle calibration |

This exists because wiring faults (reversed motor wires, wrong I2C address, floating input) are fast to find with a targeted test and slow to debug inside the full FSM. The hwtest build has been used at every hardware iteration of the bridge; it's how we confirmed the encoder/stepper relationship before trusting the stall detection.
