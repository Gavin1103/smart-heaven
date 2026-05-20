#![cfg(all(not(feature = "hil"), not(feature = "hardware_tests")))]
use anyhow::Result;
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::mutex::Mutex;
use embassy_time::{Duration, Instant, Timer};
use log::{debug, error, info, warn};

use crate::config::*;
use crate::state_machine::{BridgeState, FaultReason, SharedSensors};

impl Default for SharedState {
    fn default() -> Self {
        Self {
            sensors: Mutex::new(SharedSensors::default()),
            bridge_state: Mutex::new(BridgeState::Startup),
        }
    }
}

pub struct SharedState {
    pub sensors: Mutex<CriticalSectionRawMutex, SharedSensors>,
    pub bridge_state: Mutex<CriticalSectionRawMutex, BridgeState>,
}

use crate::traits::*;

/// Continuous task to sample all sensors and update the shared state.
pub async fn sensor_task<R, O, D1, D2, W>(
    reed: R,
    obstacle: O,
    mut ds1: D1,
    mut ds2: D2,
    mut hx711: W,
    shared: &SharedState,
) -> Result<()>
where
    R: ReedSensorTrait,
    O: ObstacleSensorTrait,
    D1: DistanceSensorTrait,
    D2: DistanceSensorTrait,
    W: WeightSensorTrait,
{
    {
        let mut sensors = shared.sensors.lock().await;
        sensors.last_sensor_update = Instant::now();
    }

    // --- WEIGHT SENSOR CALIBRATION ---
    // Calibrate the bridge's empty weight (baseline) only when the bridge is idle and closed.
    let mut baseline: f32 = 0.0;

    // TODO: Move these to config.rs
    let mut car_detect_counter = 0;     // NEW
    let mut car_clear_counter = 0;      // NEW
    let mut car_detected_state = false; // NEW
    const CAR_THRESHOLD: f32 = 90.0;
    const SCALE_FACTOR: f32 = 0.76;

    let mut baseline_calibrated = false;
    let mut last_fsm_state = BridgeState::Startup;

    let mut reed_samples: u8 = 0;
    let mut boat_samples: u8 = 0;
    let mut obs_samples: u8 = 0;
    let mut loop_counter: u32 = 0;
    // HC-SR04 draws a noticeable current spike per measurement.
    // In IdleClosed we only need ~0.5 Hz to detect an approaching boat,
    // so skip most ticks to stay within the 500 mA power budget.
    // Each loop tick is ~50 ms, so 20 ticks ≈ 1 s between measurements.
    const HCSR04_IDLE_SKIP: u32 = 20;
    let mut hcsr04_tick: u32 = 0;
    loop {
        loop_counter += 1;
        if loop_counter % 20 == 0 {
            info!("[SENSOR LOOP] iteration {}", loop_counter);
        }

        let current_state = { *shared.bridge_state.lock().await };

        // --- NEW: FORCE RECALIBRATION ON RE-SEATING ---
        if matches!(current_state, BridgeState::IdleClosed) && !matches!(last_fsm_state, BridgeState::IdleClosed) {
            info!("[HX711] Bridge returned to Idle. Forcing new baseline calibration to account for mechanical seating.");
            baseline_calibrated = false;

            // Reset the hysteresis counters so stale data doesn't carry over
            car_detect_counter = 0;
            car_clear_counter = 0;
            car_detected_state = false;
        }
        last_fsm_state = current_state;

        if !baseline_calibrated && matches!(current_state, BridgeState::IdleClosed) {
            info!("[HX711] Bridge closed. Starting baseline calibration...");
            let mut samples = 0;
            let calibration_start = Instant::now();
            while samples < 30 {
                // Timeout after 5 seconds to prevent infinite blocking
                if calibration_start.elapsed() > Duration::from_secs(5) {
                    warn!("[HX711] Calibration timeout! Using default baseline.");
                    baseline = 0.0;
                    break;
                }
                if let Ok(val) = hx711.read().await {
                    // 8388607 is the maximum positive 24-bit value for HX711 (0x7FFFFF).
                    // We filter it out along with 0 and -1 as potential invalid/stuck readings.
                    if val != 0 && val != -1 && val != 8388607 {
                        baseline += val as f32;
                        samples += 1;
                    }
                }
                // Keep sensor timestamp alive during calibration
                {
                    let mut sensors = shared.sensors.lock().await;
                    sensors.last_sensor_update = Instant::now();
                }
                Timer::after(Duration::from_millis(10)).await;
            }
            if samples > 0 {
                baseline /= samples as f32;
                info!(
                    "[HX711] Baseline calibrated: {:.0} ({} samples)",
                    baseline, samples
                );
            } else {
                warn!(
                    "[HX711] Calibration failed — 0 valid samples collected. \
                     HX711 may not be connected or powered (data pin stuck HIGH?). \
                     car_detected will always be false."
                );
            }
            baseline_calibrated = true;
        }

        let current_reed = reed.is_closed();
        if current_reed {
            reed_samples = reed_samples.saturating_add(1);
        } else {
            reed_samples = 0;
        }
        let reed_is_closed = reed_samples >= 2;

        // --- POWER BUDGET: HC-SR04 and IR obstacle are mutually exclusive ---
        //
        // bridge_open_states  → only sample IR obstacle (boat crossing detection)
        // approach states     → only fire HC-SR04 (boat approach detection)
        //
        // This prevents both sensors drawing current at the same time under the
        // 500 mA power constraint.
        let bridge_open = matches!(
            current_state,
            BridgeState::Open(_) | BridgeState::ClosingCheckClearance(_) | BridgeState::Closing(_)
        );
        let is_moving = matches!(
            current_state,
            BridgeState::Homing(_)
                | BridgeState::Opening(_)
                | BridgeState::LowerBarriers(_)
                | BridgeState::RaiseBarriers(_)
        );

        // IR obstacle: only active while bridge is open.
        // Samples decay to 0 when not in open states so stale readings don't persist.
        if bridge_open {
            let obs_blocked_this_cycle = obstacle.is_blocked();
            if obs_blocked_this_cycle {
                obs_samples = obs_samples.saturating_add(1);
            } else {
                obs_samples = obs_samples.saturating_sub(1);
            }
            // Log every 20 ticks (~1 s) so we can verify the IR is being read and see samples accumulate.
            if loop_counter % 20 == 0 {
                info!(
                    "[IR] raw={}, samples={}, blocked={}",
                    obs_blocked_this_cycle,
                    obs_samples,
                    obs_samples >= 2
                );
            }
        } else {
            obs_samples = obs_samples.saturating_sub(1);
        }
        let obs_blocked = obs_samples >= 2;

        // HC-SR04: only active while bridge is closed/approaching.
        // Throttled to ~1 Hz in IdleClosed to keep current spikes minimal.
        hcsr04_tick = hcsr04_tick.wrapping_add(1);
        let in_idle = matches!(current_state, BridgeState::IdleClosed | BridgeState::BoatDetected(_));
        let run_hcsr04 = !is_moving && !bridge_open
            && (!in_idle || hcsr04_tick % HCSR04_IDLE_SKIP == 0);

        if run_hcsr04 {
            let dist1 = ds1.measure_distance_mm().await?;
            let dist2 = ds2.measure_distance_mm().await?;

            if loop_counter % 20 == 0 {
                info!(
                    "[SENSOR] ds1={}, ds2={}, obs={}",
                    dist1.map_or("None".into(), |d| format!("{}mm", d)),
                    dist2.map_or("None".into(), |d| format!("{}mm", d)),
                    obs_blocked
                );
            }

            let boat_this_cycle = dist1
                .map(|d| d > 0 && d < BOAT_DETECTION_DISTANCE_MM)
                .unwrap_or(false)
                || dist2
                    .map(|d| d > 0 && d < BOAT_DETECTION_DISTANCE_MM)
                    .unwrap_or(false);

            if boat_this_cycle {
                boat_samples = boat_samples.saturating_add(1);
            } else {
                boat_samples = boat_samples.saturating_sub(1);
            }
        } else if is_moving {
            // Actively moving — decay so stale detections don't persist.
            boat_samples = boat_samples.saturating_sub(1);
        }
        // When skipping HC-SR04 due to throttle or bridge_open, preserve the last
        // sample count so two consecutive real measurements can reach the threshold.

        let boat_detected = boat_samples >= 2;

        if baseline_calibrated {
            match hx711.read().await {
                Ok(val) if val == 0 => {
                    if loop_counter % 20 == 0 {
                        warn!("[HX711] Timeout — data pin stayed HIGH. Check power/wiring.");
                    }
                }
                Ok(val) if val == -1 || val == 8388607 => {
                    if loop_counter % 20 == 0 {
                        warn!("[HX711] Invalid/saturated reading: {}", val);
                    }
                }
                Ok(val) => {
                    let current_raw = val as f32;
                    let diff = (current_raw - baseline).abs();
                    let grams = diff / SCALE_FACTOR;

                    // 1. Increment the correct counter based on the current reading
                    if grams > CAR_THRESHOLD {
                        car_detect_counter += 1;
                        car_clear_counter = 0; // Reset clear counter
                    } else {
                        car_clear_counter += 1;
                        car_detect_counter = 0; // Reset detect counter
                    }

                    // 2. Apply Hysteresis to change the official state
                    if car_detect_counter >= 3 {
                        car_detected_state = true;
                    } else if car_clear_counter >= 10 { // Require ~500ms of consistently empty readings
                        car_detected_state = false;

                        // ONLY apply the Drift Killer if we are absolutely sure the bridge is empty
                        if grams < 20.0 {
                            baseline = (baseline * 0.99) + (current_raw * 0.01);
                        }
                    }

                    // Log AFTER updating
                    if loop_counter % 20 == 0 {
                        info!(
                            "[HX711] raw={}, baseline={:.0}, diff={:.0}, grams={:.1}, det_cnt={}, clr_cnt={}, car={}",
                            val, baseline, diff, grams, car_detect_counter, car_clear_counter, car_detected_state
                        );
                    }
                }
                Err(e) => {
                    if loop_counter % 20 == 0 {
                        warn!("[HX711] Read error: {}", e);
                    }
                }
            }
        }

        {
            let mut sensors = shared.sensors.lock().await;

            // Debug: log IR obstacle sensor on state change
            if obs_blocked != sensors.obstacle_detected {
                info!(
                    "[IR] obstacle_detected changed: {} -> {} (raw samples={})",
                    sensors.obstacle_detected, obs_blocked, obs_samples
                );
            }

            // Debug: log boat detection on state change
            if boat_detected != sensors.boat_detected {
                info!(
                    "[BOAT] boat_detected changed: {} -> {} (raw samples={})",
                    sensors.boat_detected, boat_detected, boat_samples
                );
            }

            // Debug: log car/weight detection on state change
            if car_detected_state != sensors.car_detected {
                info!(
                    "[HX711] car_detected changed: {} -> {}",
                    sensors.car_detected, car_detected_state
                );
            }

            sensors.boat_detected = boat_detected;
            sensors.obstacle_detected = obs_blocked;
            sensors.reed_closed = reed_is_closed;
            sensors.car_detected = car_detected_state; // <-- Update this line
            sensors.last_sensor_update = Instant::now();
        }

        Timer::after(Duration::from_millis(50)).await;
    }
}

/// Moves actuators (stepper, barriers) based on the current system state.
pub async fn motion_task<S, B, E>(
    mut stepper: S,
    mut barriers: B,
    mut encoder: E,
    shared: &SharedState,
) -> Result<()>
where
    S: StepperTrait,
    B: BarrierPairTrait,
    E: EncoderTrait,
{
    let mut seq_idx: usize = 0;
    let mut last_state = BridgeState::Startup;
    let mut step_counter: u32 = 0;

    loop {
        encoder.update();
        let enc_pos = encoder.get_position();

        let state = { *shared.bridge_state.lock().await };

        if state != last_state {
            info!("[MOTION] State changed to: {:?}", state);
            // Handle state transitions for internal vars
            if matches!(last_state, BridgeState::Homing(_))
                && matches!(state, BridgeState::IdleClosed)
            {
                encoder.reset();
                let mut s = shared.sensors.lock().await;
                s.stepper_pos = 0;
            }
            if matches!(last_state, BridgeState::Closing(_))
                && matches!(state, BridgeState::RaiseBarriers(_))
            {
                encoder.reset();
                let mut s = shared.sensors.lock().await;
                s.stepper_pos = 0;
            }

            // Open barriers once on entering IdleClosed / BoatDetected, not every tick.
            // This prevents the servo from fighting for position continuously while
            // the HC-SR04 is trying to fire, which can cause power-related measurement failures.
            let entering_idle = matches!(
                state,
                BridgeState::IdleClosed | BridgeState::BoatDetected(_)
            ) && !matches!(
                last_state,
                BridgeState::IdleClosed | BridgeState::BoatDetected(_)
            );
            if entering_idle {
                barriers.open().await?;
            }

            last_state = state;
        }

        match state {
            BridgeState::Homing(_) => {
                stepper.apply_step(&STEP_SEQUENCE[seq_idx])?;
                seq_idx = (seq_idx + 1) % 8; // FORWARD = closing
                let mut s = shared.sensors.lock().await;
                s.stepper_pos -= 1;
                step_counter += 1;
                if step_counter % 1000 == 0 {
                    info!(
                        "[MOTION] Homing: {} steps, stepper_pos={}",
                        step_counter, s.stepper_pos
                    );
                }
                barriers.stop()?;
            }
            BridgeState::Opening(_) => {
                stepper.apply_step(&STEP_SEQUENCE[seq_idx])?;
                seq_idx = if seq_idx == 0 { 7 } else { seq_idx - 1 }; // REVERSED = opening
                let mut s = shared.sensors.lock().await;
                s.stepper_pos += 1;
                step_counter += 1;
                if step_counter % 1000 == 0 {
                    info!(
                        "[MOTION] Opening: {} steps, stepper_pos={}",
                        step_counter, s.stepper_pos
                    );
                }
                barriers.stop()?;
            }
            BridgeState::Closing(_) => {
                stepper.apply_step(&STEP_SEQUENCE[seq_idx])?;
                seq_idx = (seq_idx + 1) % 8; // FORWARD = closing
                let mut s = shared.sensors.lock().await;
                s.stepper_pos -= 1;
                barriers.stop()?;
            }
            BridgeState::LowerBarriers(_) => {
                stepper.stop()?;
                barriers.close().await?;
            }
            BridgeState::RaiseBarriers(_) => {
                stepper.stop()?;
                barriers.open().await?;
            }
            BridgeState::IdleClosed | BridgeState::BoatDetected(_) => {
                // Barriers are opened once on state entry (see state-change block above).
                stepper.stop()?;
            }
            _ => {
                stepper.stop()?;
                barriers.stop()?;
            }
        }

        {
            let mut s = shared.sensors.lock().await;
            s.encoder_pos = enc_pos;
            s.encoder_sw_pressed = encoder.is_pressed();
        }

        Timer::after(STEP_INTERVAL).await;
    }
}

pub async fn light_task<L>(mut lights: L, shared: &SharedState) -> Result<()>
where
    L: TrafficLightsTrait,
{
    loop {
        let state = { *shared.bridge_state.lock().await };
        let should_be_on = (Instant::now().as_millis() / FLASH_TICK_MS) % 2 == 0;
        let fast_flash = (Instant::now().as_millis() / (FLASH_TICK_MS / 2)) % 2 == 0;

        match state {
            BridgeState::Startup | BridgeState::IdleClosed | BridgeState::BoatDetected(_) => {
                lights.set_idle()?;
            }
            BridgeState::Homing(_)
            | BridgeState::WarningRoadTraffic(_)
            | BridgeState::LowerBarriers(_)
            | BridgeState::WaitBridgeClear(_)
            | BridgeState::Opening(_)
            | BridgeState::ClosingCheckClearance(_)
            | BridgeState::Closing(_)
            | BridgeState::RaiseBarriers(_) => {
                lights.set_warning(should_be_on)?;
            }
            BridgeState::Fault(_) => {
                lights.set_warning(fast_flash)?;
            }
            BridgeState::WaterPrepRedGreen(_) => {
                lights.set_prep()?;
            }
            BridgeState::Open(_) => {
                lights.set_open(should_be_on)?;
            }
        }

        Timer::after(Duration::from_millis(100)).await;
    }
}

pub async fn buzzer_task<B>(mut buzzer: B, shared: &SharedState) -> Result<()>
where
    B: BuzzerTrait,
{
    loop {
        let state = { *shared.bridge_state.lock().await };
        let should_be_on = (Instant::now().as_millis() / FLASH_TICK_MS) % 2 == 0;

        match state {
            BridgeState::WarningRoadTraffic(_)
            | BridgeState::LowerBarriers(_)
            | BridgeState::WaitBridgeClear(_) => {
                buzzer.set_on(should_be_on)?;
            }
            BridgeState::Fault(_) => {
                buzzer.set_on(true)?;
                Timer::after(Duration::from_millis(50)).await;
                buzzer.set_on(false)?;
                Timer::after(Duration::from_millis(50)).await;
                buzzer.set_on(true)?;
                Timer::after(Duration::from_millis(50)).await;
                buzzer.set_on(false)?;
                Timer::after(Duration::from_millis(300)).await;
            }
            _ => {
                buzzer.set_on(false)?;
            }
        }

        Timer::after(Duration::from_millis(100)).await;
    }
}

/// Periodically triggers the state machine transition logic and updates the shared state.
pub async fn coordinator_task(shared: &SharedState) -> Result<()> {
    let mut last_logged_state: Option<BridgeState> = None;
    let mut log_counter: u32 = 0;
    let mut reed_trigger_pos: Option<i32> = None;

    loop {
        let (sensors, current_state) = {
            let s = shared.sensors.lock().await;
            let st = shared.bridge_state.lock().await;
            (*s, *st)
        };

        if last_logged_state != Some(current_state) {
            info!("[FSM] State changed to: {:?}", current_state);
            last_logged_state = Some(current_state);
        }

        log_counter += 1;
        if log_counter >= 20 {
            log_counter = 0;
            debug!(
                "[SENSORS] boat={}, obstacle={}, reed={}, car={}, enc={}, step={}, last_update={}ms ago",
                sensors.boat_detected,
                sensors.obstacle_detected,
                sensors.reed_closed,
                sensors.car_detected,
                sensors.encoder_pos,
                sensors.stepper_pos,
                sensors.last_sensor_update.elapsed().as_millis()
            );
        }

        let next_state = if !matches!(current_state, BridgeState::Startup | BridgeState::Homing(_))
            && sensors.last_sensor_update.elapsed() > SENSOR_READ_TIMEOUT
            && !matches!(
                current_state,
                BridgeState::Fault(FaultReason::SensorReadTimeout)
            ) {
            error!(
                "SENSOR READ TIMEOUT! last_update was {}ms ago. Entering fault state.",
                sensors.last_sensor_update.elapsed().as_millis()
            );
            BridgeState::Fault(FaultReason::SensorReadTimeout)
        } else {
            current_state.next(&sensors, &mut reed_trigger_pos, Instant::now())
        };

        if next_state != current_state {
            let mut st = shared.bridge_state.lock().await;
            *st = next_state;
        }

        Timer::after(Duration::from_millis(50)).await;
    }
}
