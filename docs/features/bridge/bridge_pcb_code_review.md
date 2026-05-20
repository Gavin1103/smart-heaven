# Bridge Code Review — `feature/bridge_pcb` Branch

**Date:** 2026-05-18
**Reviewer:** Code review (static analysis)
**Scope:** All firmware changes on `feature/bridge_pcb` vs `main`
**Files reviewed:**

- `lib/MCP23017/` — new I/O-expander driver
- `lib/PCA9685/` — new PWM-driver driver
- `lib/TelemetryManager/` — new telemetry task
- `lib/BridgeSystem/` — `BridgeController`, `BridgeMotion`, `BridgeSensors`, `BridgeSignaling`
- `include/city_config.h`, `include/telemetry_config.h`
- `src/main.cpp`, `src/main_hwtest.cpp`, `platformio.ini`

> Note: this is a static review. The branch was **not compiled** — PlatformIO build
> verification is still recommended before merge.

---

## 1. Overall Verdict

The branch is a **substantial, well-structured rework**: all bridge I/O moves from
direct ESP32 GPIO to the new Bridge Node PCB (MCP23017 I/O expander + PCA9685 PWM
driver), and a non-blocking telemetry path is added. The driver layering is clean,
the FSM logic itself is untouched, and the I2C bus is mutex-protected so the
cross-task access is safe.

However, **two of the migrated subsystems have timing problems that make their
sensor data unreliable** (HC-SR04 sonar and HX711 load cell). These are design-level
issues introduced by routing time-critical signals through an I2C expander, and they
should be resolved or explicitly feature-gated before this is treated as production
firmware. Several medium-severity robustness gaps are also listed below.

---

## 2. High-Severity Issues

### 2.1 — Sonar echo pulses are missed at the 100 Hz poll rate

`BridgeSensors::_updateSonar()` samples each echo pin **once per `update()` call**,
and `bridgeTask()` calls `update()` every 10 ms (100 Hz). The HC-SR04 echo pulse for
a boat at the `BOAT_DETECT_MM = 500` threshold is only:

```
500 mm × 2 (round trip) / 0.343 mm/µs ≈ 2 915 µs ≈ 2.9 ms
```

That is **shorter than one poll interval**. Consequences:

- The whole HIGH pulse can fall between two polls → `lastEchoHigh` never sees it →
  no measurement at all for that ping.
- Even when the rising edge *is* caught, the falling edge is sampled up to 10 ms
  later, so the measured `pulseUs` (and therefore `distMm`) is grossly inflated.

The old code polled the echo pin tightly inside a dedicated sonar FSM; the new code
cannot, because each sample is a full I2C `readPortB()` transaction. **Boat detection
is effectively unreliable as written.**

**Recommendation:** echo pulse-width measurement cannot be done by polling an I2C
expander at 100 Hz. Options: (a) keep the HC-SR04 *echo* lines on real ESP32 GPIO
with interrupt or `pulseIn`-style capture, (b) use a sonar module that returns
distance over I2C/serial, or (c) sample echo in a dedicated high-rate task. At
minimum, document the current limitation prominently.

### 2.2 — HX711 powers down during every bit-bang read

`BridgeSensors::_hx711ReadRaw()` toggles the HX711 `SCK` line through
`MCP23017::setBitA()`. Each `setBitA()` is a full I2C transaction (hundreds of µs at
400 kHz, plus mutex acquisition). `SCK` therefore stays HIGH **far beyond the HX711's
60 µs `PD_SCK` power-down threshold** on every clock bit.

Per the HX711 datasheet, holding `PD_SCK` HIGH > 60 µs powers the chip down and the
in-progress conversion is lost. The code's only mitigation is glitch detection that
discards `0`, `-1`, and `0x7FFFFF` — but a power-down mid-read yields **arbitrary**
corrupt values, not those three sentinels. The result is that `isWeightDetected()`
likely never returns a trustworthy reading.

This is acknowledged in `bridge_pcb_design.md §4.4` as a "deliberate trade-off", but
the trade-off as implemented **fails silently rather than degrading gracefully**: the
bridge would read "no car on deck" even when one is present — a safety-relevant false
negative if weight is used as a deck-clear interlock.

**Recommendation:** drive `HX711 SCK` (and ideally `DOUT`) from real ESP32 GPIO
rather than the expander, or, if the PCB cannot be changed, explicitly disable the
load-cell feature and have `isWeightDetected()` return a known/unknown state instead
of a misleading `false`.

---

## 3. Medium-Severity Issues

### 3.1 — Failed I2C reads silently report all inputs active

`MCP23017::readPortA()` / `readPortB()` ignore the `bool` return of
`I2CManager::readBytes()`; on failure the local `val` stays `0x00`. Every bridge
digital input is **active-low**, so a failed read is indistinguishable from
"obstacle blocking + reed closed + IR blocked + encoder pressed". A transient bus
glitch can therefore inject a spurious obstacle or closed-limit signal into the FSM.

**Recommendation:** propagate the read failure (return `bool` or use an out-param)
and have callers hold last-known-good values, or escalate to `Fault` after repeated
failures.

### 3.2 — `BridgeController::begin()` failure is discarded

`begin()` correctly returns `false` when the MCP23017 or PCA9685 is not found, but
`bridgeTask()` in `main.cpp` calls `bridgeNode.begin();` and **ignores the result**,
then enters the `update()` loop unconditionally. With no PCB attached the firmware
runs a blind control loop over a dead bus.

**Recommendation:** check the return value; on failure, retry or halt (and light the
fault LED if the signaling path is reachable) instead of looping `update()`.

### 3.3 — No WiFi reconnection

`connectWiFi()` attempts to connect once for 10 s in `setup()`. If it fails — or if
WiFi drops later — `sendPayload()` only checks `WiFi.status()` and drops every event.
Nothing ever retries, so telemetry is permanently lost after the first disconnect.

**Recommendation:** call `WiFi.setAutoReconnect(true)`, or add a periodic reconnect
attempt in the telemetry task.

### 3.4 — Sonar re-triggers every 20 ms (too fast)

The PCA9685 generates the HC-SR04 trigger pulses continuously at the chip's PWM
frequency, which is set to **50 Hz** for the servos — so the sonar fires every 20 ms.
The HC-SR04 datasheet recommends ≥ 60 ms between cycles so prior echoes decay; at
20 ms, ghost echoes from the previous ping can be read as a valid pulse. The
East/West stagger only addresses *cross-sensor* interference, not self-interference.

This is an architectural tension: the PCA9685 has a **single prescaler shared by all
16 channels**, so the sonar trigger rate cannot be lowered independently of the servo
frequency. The same constraint affects the buzzer (see 4.4).

**Recommendation:** decide whether trigger generation belongs on the PCA9685 at all;
gating triggers in software or moving them to a timer-driven GPIO would decouple the
sonar cycle from the 50 Hz servo requirement.

---

## 4. Low-Severity / Observations

### 4.1 — Actuator I2C writes are unchecked

`PCA9685::setChannel/setServoAngle/setSonarTrigger/setFullOff` and
`MCP23017::writePortA/setBitA` ignore the `I2CManager::writeBytes()` return value, so
a failed servo, buzzer or stepper command is silent. Acceptable for thin drivers, but
worth a deliberate decision.

### 4.2 — Encoder loses counts on fast rotation

`_updateEncoder()` processes at most one CLK edge per 100 Hz poll. Fine for stall
detection and occasional manual input, but fast manual turning will under-count.

### 4.3 — HX711 ready-timeout branch is effectively dead

`_updateHx711()` is gated by `now - lastPollMs < pollMs` (150 ms in `Reading`), so by
the time the `HX711_READY_TIMEOUT_MS` (100 ms) check runs, the condition is always
true. The timeout constant is redundant logic — clean up or rework.

### 4.4 — Buzzer driven at 50 Hz

`_setBuzzerPwm()` drives the buzzer channel at 50% duty at the PCA9685's 50 Hz. If
the buzzer is **passive**, 50 Hz is essentially inaudible — passive buzzers need a
kHz-range drive. If it is **active**, any HIGH suffices and this is fine. Confirm the
buzzer type; note the shared-prescaler constraint (4.x / 3.4) prevents a kHz tone
while the servos need 50 Hz.

### 4.5 — Shift-register / stepper timing (documented, acceptable)

LED updates cost ~24 I2C transactions (~5 ms) and the stepper effective rate drops
from ~333 to ~290 steps/s. Both are documented in code comments and within spec — no
action needed.

---

## 5. What Was Done Well

- **Clean driver layering** — `MCP23017` and `PCA9685` are thin, single-purpose
  drivers built on the existing mutex-protected `I2CManager`; cross-task I2C is safe.
- **Telemetry decoupling** — `TelemetryManager` uses a FreeRTOS queue + background
  task, so `enqueue()` is non-blocking and safe to call from the 100 Hz bridge task.
  `TelemetryPayload` correctly carries only `.rodata` string-literal pointers across
  the task boundary.
- **Constructor member-init order** — the `_mcp`/`_pca`-before-subsystems ordering is
  correct and clearly commented.
- **Port A upper-nibble preservation** — `BridgeMotion` and `BridgeSignaling`
  consistently read-modify-write the shared latch so stepper, HX711 and shift-register
  bits don't clobber each other.
- **No secrets committed** — `telemetry_config.h` ships placeholder credentials,
  overridable via `platformio.ini` build flags.
- **FSM untouched** — the state machine logic is unchanged; only the I/O layer moved.

---

## 6. Recommended Actions Before Merge

| Priority | Action                                                                | Status                                              |
|----------|-----------------------------------------------------------------------|-----------------------------------------------------|
| High     | Resolve or feature-gate the sonar echo timing (2.1)                   | Open — needs hardware/design decision               |
| High     | Resolve or feature-gate the HX711 power-down issue (2.2)              | Open — needs hardware/design decision               |
| Medium   | Propagate I2C read failures from `MCP23017` (3.1)                     | **Fixed** — reads return last-known-good on failure |
| Medium   | Handle `BridgeController::begin()` failure in `bridgeTask()` (3.2)    | **Fixed** — task halts with a fatal log             |
| Medium   | Add WiFi reconnection (3.3)                                           | **Fixed** — `WiFi.setAutoReconnect(true)` enabled   |
| Medium   | Re-evaluate sonar trigger rate vs the shared PCA9685 prescaler (3.4)  | Open — needs hardware/design decision               |
| Low      | Confirm buzzer type vs 50 Hz drive (4.4)                              | Open — needs hardware check                         |
| —        | Run a full PlatformIO build for both `main` and `hwtest` environments | Open                                                |

> **4.3 (HX711 ready-timeout):** re-examined and **left as-is** — the timeout is not
> actually dead code; it is load-bearing during the `Taring` phase (where `pollMs` is
> 10 ms, below the 100 ms timeout). Only the `Reading` phase makes it redundant.
