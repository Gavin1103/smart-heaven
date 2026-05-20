# BridgeSystem

The bridge module is the first completed city component and acts as the **reference implementation** for how to structure a module in this project. Read this alongside the main [smart_city_v2 README](../../README.md).

---

## Overview

The bridge controls road/water traffic signals, physical barriers (servos), and a rotating deck (stepper motor) in response to boat detection. It runs entirely in its own FreeRTOS task.

---

## File Structure

```
BridgeSystem/
├── CityModule.h              # Abstract interface shared by all city modules
├── BridgeController.h/.cpp   # Top-level orchestrator + 14-state FSM
├── BridgeSignaling.h/.cpp    # Road/water LEDs + buzzer via 74HC595 shift register
├── BridgeMotion.h/.cpp       # 28BYJ-48 stepper + 2× SG90 servo barriers
└── BridgeSensors.h/.cpp      # HC-SR04 sonar ×2, KY-032 IR, KY-025 reed,
                               #   HX711 load cell, KY-040 rotary encoder
```

`BridgeController` owns the three subsystem objects and only calls semantic methods on them — it never touches GPIO directly. This is the pattern to follow.

---

## Pin Assignments

| Subsystem | Signal | GPIO |
|:----------|:-------|:-----|
| Signaling | SER (shift reg) | 6 |
| Signaling | SRCLK | 7 |
| Signaling | RCLK | 8 |
| Signaling | OE (active LOW) | 9 |
| Signaling | Buzzer | 12 |
| Motion | Servo Left | 5 |
| Motion | Servo Right | 10 |
| Motion | Stepper IN1–IN4 | 15, 16, 17, 18 |
| Sensors | Sonar East Trig/Echo | 1, 2 |
| Sensors | Sonar West Trig/Echo | 3, 4 |
| Sensors | KY-032 IR obstacle | 11 |
| Sensors | KY-025 Reed switch | 21 |
| Sensors | HX711 DT/SCK | 38, 39 |
| Sensors | KY-040 CLK/DT/SW | 40, 41, 42 |

---

## State Machine

```
Startup → Homing → IdleClosed
                       │
              (boat detected)
                       ↓
               BoatDetected        (1 s debounce)
                       ↓
           WarningRoadTraffic      (3 s flashing yellow)
                       ↓
            WaitBridgeClear        (road must be empty)
                       ↓
              LowerBarriers
                       ↓
           WaterPrepRedGreen       (2 s red+green hold)
                       ↓
                  Opening          (6400 stepper steps)
                       ↓
                    Open           (min 30 s + waterway clear)
                       ↓
          ClosingCheckClearance    (2 s final safety window)
                       ↓
                  Closing          (until reed triggered)
                       ↓
              RaiseBarriers → IdleClosed

Any state → Fault   (timeout / sensor contradiction / stall)
Fault → Homing      (encoder button press to recover)
```

### Timing constants

| Constant | Value | Meaning |
|:---------|:------|:--------|
| `BOAT_CONFIRM_MS` | 1 000 ms | Boat must persist before acting |
| `WARNING_DURATION_MS` | 3 000 ms | Road warning flash duration |
| `WATER_PREP_MS` | 2 000 ms | Red+green water signal hold |
| `BRIDGE_WAIT_MS` | 30 000 ms | Minimum time bridge stays open |
| `CLEARANCE_CHECK_MS` | 2 000 ms | Final safety pause before closing |
| `HOMING_TIMEOUT_MS` | 30 000 ms | Homing hard timeout |
| `BRIDGE_TIMEOUT_MS` | 40 000 ms | Opening/closing hard timeout |
| `FULL_ROTATION` | 6 400 steps | One full deck rotation |
| `HOMING_EXTRA_STEPS` | 200 steps | Overshoot to seat the bridge |

---

## Subsystem Details

### BridgeSignaling — 74HC595 shift register

All outputs are packed into one byte and written to the shift register on every change.

| Bit | Signal |
|:----|:-------|
| Q0 | Road Red |
| Q1 | Road Green |
| Q2 | Road Yellow |
| Q3 | Water Red |
| Q4 | Water Green |
| Q5 | Status LED 1 |
| Q6 | Status LED 2 |
| Q7 | Fault LED |

> Water traffic has no Yellow output — passing `Color::Yellow` is treated as Off.

### BridgeMotion — power interlock

The stepper and servos share a 5V/500 mA rail. `BridgeMotion` enforces mutual exclusion: the stepper will not step while `isBarrierMoving()` is true, and the controller never starts a motor move while barriers are in motion. If your module has multiple actuators on a shared rail, add a similar guard.

Servos are **detached** when not actively moving to avoid holding current. The 200 ms stagger between left and right servo (`BARRIER_STAGGER_MS`) prevents both stalling simultaneously.

### BridgeSensors — non-blocking sensor pattern

Every sensor subsystem is its own mini-FSM. The HC-SR04 sonar cycles through `Idle → WaitEchoStart → WaitEchoEnd → Cooldown` without blocking. All sensors require N consecutive agreeing readings before updating their confirmed state. Follow this same pattern for any new sensor you add.

The HX711 load cell uses a 30-sample tare on startup and EMA drift correction during operation. `tareLoadCell()` is called automatically each time the bridge returns to `IdleClosed`.

---

## Hardware Test Cases

Flash the `hwtest` build and open the serial monitor at 115200 baud.

| Key | Test | What it validates |
|:----|:-----|:------------------|
| `s` | Stepper | 500 steps forward + 500 back; PASS if encoder net offset ≤ ±5 |
| `b` | Barriers | Left then right servo open/close cycle |
| `l` | Lights | Cycles each shift register output Q0–Q7, then buzzer |
| `i` | Inputs | Prints reed / IR / weight states every 500 ms for 5 s |
| `v` | VarServo | Sweeps each servo 0→180° one degree at a time to find calibration angles |

To add a test for your own module, add a new value to the `ActiveTest` enum in `main_hwtest.cpp` and a matching `case` in the `loop()` FSM. Keep it non-blocking — use the same `millis()` + phase pattern the existing tests use.

---

## Known Issues

- **`_bus` / `_address` in `BridgeController`** — stored but unused. All hardware is GPIO-direct today. Reserved for future I2C peripheral migration.
- **Simultaneous water Red+Green** — `BridgeSignaling` has no API for two water channels at once; `setWaterLightsRedGreen()` in `BridgeController` currently only sets Red. Needs fixing in `BridgeSignaling`.
