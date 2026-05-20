# Bridge Code Review — Post-Refactoring Analysis

**Date:** 2026-04-02  
**Scope:** All uncommitted changes in `embedded/bridge/`

---

## 1. Overall Verdict

**The refactoring does NOT break existing behavioral logic.**  
All hardware-driving code (stepper, sensors, lights, buzzer, barriers, encoder) was moved from inherent `impl` blocks into trait `impl` blocks with **identical logic preserved**. The state machine (`state_machine.rs`) and task orchestration (`tasks.rs`) are functionally equivalent to the previous version.

`cargo clippy --all-targets --all-features -- -D warnings` **passes cleanly**.  
`cargo build` **succeeds**.

---

## 2. Issues Found

### 2.1 — `harness = true` in `Cargo.toml` (Medium Priority)

```toml
[[bin]]
name = "learning-group"
harness = true # use the built-in cargo test harness
```

This setting causes `cargo test` to attempt flashing the binary to the ESP32 via `espflash`, which fails without hardware connected. For an ESP-IDF project, this should be `harness = false` unless you intentionally want on-device test execution. With `harness = true`, `#[test]` functions in `tests.rs` are compiled but can only run when the board is plugged in.

**Recommendation:** If you want host-side unit tests (e.g., state machine logic), consider extracting them into a separate `#[cfg(test)]` module that can be compiled for the host target, or set `harness = false` and run tests manually on-device.

---

### 2.2 — Buzzer: Duplicate `set_on` Method (Low Priority — Not a Bug)

```rust
// buzzer.rs
impl<'d> Buzzer<'d> {
    pub fn set_on(&mut self, on: bool) -> Result<()> { ... } // inherent method
}

impl<'d> BuzzerTrait for Buzzer<'d> {
    fn set_on(&mut self, on: bool) -> Result<()> {
        self.set_on(on) // calls the inherent method (Rust resolves inherent over trait)
    }
}
```

This works because Rust's method resolution picks inherent methods over trait methods, so `self.set_on(on)` inside the trait impl calls the inherent `set_on`, not itself. However, it's **confusing and fragile** — if someone removes the inherent method, this becomes infinite recursion.

**Recommendation:** Remove the inherent `set_on` and put the logic directly in the trait impl, or rename one of them for clarity.

---

### 2.3 — `hardware_test.rs` Bypasses Trait Abstraction (Low Priority)

`hardware_test.rs` directly accesses `barriers.left` and `barriers.right` (the individual `ServoBarrier` fields), bypassing the `BarrierPairTrait` abstraction:

```rust
barriers.left.open()?;
barriers.left.close()?;
barriers.left.stop()?;
barriers.right.open()?;
// ...
```

This means `hardware_test.rs` is coupled to the concrete `BarrierPair` struct and won't work with a mock/alternative implementation.

**Recommendation:** Either add individual barrier methods to `BarrierPairTrait` (e.g., `open_left`, `close_right`), or accept this coupling since hardware tests are inherently hardware-specific.

---

### 2.4 — Incomplete Mock Coverage (Low Priority)

`mock.rs` only defines `MockDistanceSensor` and `MockReedSensor`. The following traits have **no mock implementations**:

- `ObstacleSensorTrait`
- `WeightSensorTrait`
- `StepperTrait`
- `TrafficLightsTrait`
- `BuzzerTrait`
- `BarrierPairTrait`
- `EncoderTrait`

The existing unit tests (`tests.rs`) test only the state machine (which doesn't need hardware mocks), so this isn't blocking anything right now.

**Recommendation:** Add mocks for all traits if you plan to write integration-level unit tests for `tasks.rs` (sensor_task, motion_task, etc.) without hardware.

---

### 2.5 — Excessive `#[allow(dead_code)]` Annotations (Low Priority)

Many trait methods and structs are annotated with `#[allow(dead_code)]`:

- Entire `BuzzerTrait` trait
- `BarrierPairTrait::open()`, `close()`
- `EncoderTrait::update()`, `get_position()`, `reset()`
- Multiple `TrafficLightsTrait` methods
- `SharedSensors` fields

These are used at runtime through the tasks system, but since `tasks.rs` is gated behind `#[cfg(all(not(feature = "hil"), not(feature = "hardware_tests")))]`, the compiler sees them as unused when compiling with those features or in test mode.

**Recommendation:** Instead of per-item `#[allow(dead_code)]`, consider a module-level `#![allow(dead_code)]` on `traits.rs`, or restructure the cfg gates so the compiler sees the full usage chain.

---

### 2.6 — Empty `impl` Block for `StepperPins` (Trivial)

```rust
// stepper.rs, line 42
impl<'d> StepperPins<'d> {}
```

This empty inherent impl block serves no purpose.

**Recommendation:** Remove it.

---

### 2.7 — Typo in `hardware_test.rs` (Trivial)

```rust
info!("  Opening bridge...b");  // line 101 — trailing "b"
```

**Recommendation:** Remove the stray `b`.

---

## 3. State Machine Behavioral Analysis

The state machine transition logic in `state_machine.rs` is **correct and well-structured**. Key observations:

| Transition | Behavior | Status |
|---|---|---|
| Startup → Homing | Triggers unless both boat + obstacle detected | ✅ Correct |
| Homing → IdleClosed | Reed triggers, then waits for `HOMING_EXTRA_STEPS` | ✅ Correct |
| Homing timeout | Faults after `HOMING_TIMEOUT` (30s) | ✅ Correct |
| IdleClosed → BoatDetected | On boat detection | ✅ Correct |
| BoatDetected → WarningRoadTraffic | After `BOAT_DETECTION_CONFIRMATION` (1s) | ✅ Correct |
| BoatDetected → IdleClosed | If boat disappears before confirmation | ✅ Correct |
| Full opening sequence | Warning → LowerBarriers → WaitBridgeClear → WaterPrep → Opening | ✅ Correct |
| Obstacle during Closing | Re-opens bridge (safety) | ✅ Correct |
| Closing → RaiseBarriers | On reed_closed (bridge seated) | ✅ Correct |
| Fault → Homing | On encoder button press (manual reset) | ✅ Correct |

**One subtle behavior to be aware of:** In `coordinator_task`, `reed_trigger_pos` is a local variable. If the coordinator task restarts (which it doesn't currently, but theoretically), this state would be lost. This is fine for the current design.

---

## 4. Task Orchestration Analysis

The task system (`tasks.rs`) correctly:

- Debounces sensor readings (reed, obstacle, boat — all require 2+ consecutive samples)
- Skips ultrasonic readings during motor movement (power constraint awareness)
- Calibrates the HX711 weight sensor baseline only after reaching `IdleClosed`
- Implements drift compensation for the weight sensor
- Resets stepper position on Homing→IdleClosed and Closing→RaiseBarriers transitions
- Monitors sensor liveness with `SENSOR_READ_TIMEOUT` watchdog

**No behavioral regressions detected.**

---

## 5. Summary of Recommendations

| #   | Issue                                       | Priority | Action                                       |
|-----|---------------------------------------------|----------|----------------------------------------------|
| 2.1 | `harness = true` prevents host-side testing | Medium   | Set to `false` or extract host-testable code |
| 2.2 | Buzzer duplicate `set_on`                   | Low      | Consolidate into trait impl only             |
| 2.3 | hardware_test bypasses trait abstraction    | Low      | Accept or extend trait                       |
| 2.4 | Incomplete mock coverage                    | Low      | Add mocks when needed for new tests          |
| 2.5 | Excessive `#[allow(dead_code)]`             | Low      | Use module-level allow or fix cfg gates      |
| 2.6 | Empty `StepperPins` impl block              | Trivial  | Remove                                       |
| 2.7 | Typo "Opening bridge...b"                   | Trivial  | Fix typo                                     |


## 6. AI Analysis

This code review was AI-generated based on the following AI model: Claude 4.5 Opus.
Additional human code review comments are added through the gitlab interface.