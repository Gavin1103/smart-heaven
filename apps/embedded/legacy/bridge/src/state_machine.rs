#![allow(dead_code)]
use crate::config::*;
use embassy_time::{Duration, Instant};
use log::{error, info, warn};

/// Snapshot of all sensor inputs and system state used for state transitions.
#[derive(Debug, Clone, Copy)]
pub struct SharedSensors {
    pub boat_detected: bool,
    pub obstacle_detected: bool,
    pub reed_closed: bool,
    pub car_detected: bool,
    pub encoder_pos: i32,
    pub stepper_pos: i32,
    pub encoder_sw_pressed: bool,
    pub last_sensor_update: Instant,
}

impl Default for SharedSensors {
    fn default() -> Self {
        Self {
            boat_detected: false,
            obstacle_detected: false,
            reed_closed: false,
            car_detected: false,
            encoder_pos: 0,
            stepper_pos: 0,
            encoder_sw_pressed: false,
            last_sensor_update: Instant::now(),
        }
    }
}


/// Possible reasons for the bridge entering a Fault state.
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum FaultReason {
    HomingTimeout,
    OpeningTimeout,
    ClosingTimeout,
    InvalidStartupState,
    SafetyCheckFailed,
    SensorReadTimeout,
    /// Stepper commanded a full opening cycle but the reed sensor is still closed,
    /// meaning the bridge deck never physically left the closed position (stalled motor / too heavy).
    MotorStall,
}

/// The bridge state machine. Each state (except Startup/Fault/Idle) typically carries 
/// the 'Instant' it was entered to handle time-based transitions.
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum BridgeState {
    Startup,
    /// Moving towards the closed position until the reed sensor triggers.
    Homing(Instant),
    /// Bridge is closed and waiting for a boat.
    IdleClosed,
    /// A boat has been detected, waiting for confirmation duration.
    BoatDetected(Instant),
    /// Flashing lights to warn road traffic of impending closure.
    WarningRoadTraffic(Instant),
    /// Physically lowering the barriers.
    LowerBarriers(Instant),
    /// Barriers are down, ensuring no cars are trapped on the bridge (weight sensor).
    WaitBridgeClear(Instant),
    /// Road traffic stopped, bridge clear. Signaling to water traffic.
    WaterPrepRedGreen(Instant),
    /// Moving the bridge to the open position.
    Opening(Instant),
    /// Bridge is fully open.
    Open(Instant),
    /// Checking if any boats/obstacles are still in the way before closing.
    ClosingCheckClearance(Instant),
    /// Moving the bridge back to the closed position.
    Closing(Instant),
    /// Physically raising the barriers to allow road traffic.
    RaiseBarriers(Instant),
    /// System halted due to a safety issue or timeout.
    Fault(FaultReason),
}

impl BridgeState {
    pub fn next(
        &self,
        sensors: &SharedSensors,
        reed_trigger_pos: &mut Option<i32>,
        now: Instant,
    ) -> BridgeState {
        let mut next_state = *self;

        match *self {
            // Initial state: ensure we aren't starting in a physically impossible configuration.
            BridgeState::Startup => {
                if sensors.boat_detected && sensors.obstacle_detected {
                    error!("Invalid startup position: multiple sensors blocked!");
                    next_state = BridgeState::Fault(FaultReason::InvalidStartupState);
                } else {
                    info!("[STARTUP] -> Homing");
                    next_state = BridgeState::Homing(now);
                }
            }

            // Move towards home (closed). Once reed triggers, we move a bit further 
            // to ensure it's firmly seated against the support.
            BridgeState::Homing(start_time) => {
                if sensors.reed_closed {
                    if reed_trigger_pos.is_none() {
                        *reed_trigger_pos = Some(sensors.stepper_pos);
                        info!(
                            "[HOMING] Reed triggered at {}. Seating bridge for {} more steps...",
                            sensors.stepper_pos, HOMING_EXTRA_STEPS
                        );
                    }
                    if sensors.stepper_pos <= reed_trigger_pos.unwrap() - HOMING_EXTRA_STEPS {
                        info!(
                            "[HOMING] Reed closed and seated! Home reached after {}ms",
                            now.duration_since(start_time).as_millis()
                        );
                        next_state = BridgeState::IdleClosed;
                        *reed_trigger_pos = None;
                    }
                } else {
                    *reed_trigger_pos = None;
                    if now.duration_since(start_time) >= HOMING_TIMEOUT {
                        error!(
                            "[HOMING] Timeout after {}ms! reed_closed={}",
                            now.duration_since(start_time).as_millis(),
                            sensors.reed_closed
                        );
                        next_state = BridgeState::Fault(FaultReason::HomingTimeout);
                    }
                }
            }

            BridgeState::IdleClosed => {
                if sensors.boat_detected {
                    info!("Boat detected!");
                    next_state = BridgeState::BoatDetected(now);
                } else if !sensors.reed_closed {
                    warn!("[IDLE] Reed opened unexpectedly! Homing...");
                    next_state = BridgeState::Homing(now);
                }
            }

            BridgeState::BoatDetected(start_time) => {
                if !sensors.boat_detected {
                    info!("Boat disappeared, returning to Idle.");
                    next_state = BridgeState::IdleClosed;
                } else if now.duration_since(start_time) >= BOAT_DETECTION_CONFIRMATION {
                    next_state = BridgeState::WarningRoadTraffic(now);
                }
            }

            // --- NEW SAFETY SEQUENCE ---
            BridgeState::WarningRoadTraffic(start_time) => {
                if now.duration_since(start_time) >= WARNING_DURATION {
                    info!("Warning complete. Waiting for bridge to clear...");
                    next_state = BridgeState::WaitBridgeClear(now);
                }
            }

            BridgeState::WaitBridgeClear(start_time) => {
                if !sensors.car_detected {
                    info!("Bridge clear! Lowering barriers.");
                    next_state = BridgeState::LowerBarriers(now);
                } else if now.duration_since(start_time) >= Duration::from_secs(30) {
                    // TODO: Move this timeout to config.rs
                    error!("Bridge blocked by weight for too long!");
                    next_state = BridgeState::Fault(FaultReason::SafetyCheckFailed);
                }
            }

            BridgeState::LowerBarriers(start_time) => {
                if now.duration_since(start_time) >= BARRIER_MOVE_DURATION {
                    info!("Barriers closed. Preparing water lights.");
                    next_state = BridgeState::WaterPrepRedGreen(now);
                }
            }

            BridgeState::WaitBridgeClear(start_time) => {
                if !sensors.car_detected {
                    info!("Bridge clear! Preparing water lights.");
                    next_state = BridgeState::WaterPrepRedGreen(now);
                } else if now.duration_since(start_time) >= Duration::from_secs(30) {
                    // TODO: Move this timeout to config.rs
                    error!("Bridge blocked by weight for too long!");
                    next_state = BridgeState::Fault(FaultReason::SafetyCheckFailed);
                }
            }

            BridgeState::WaterPrepRedGreen(start_time) => {
                if now.duration_since(start_time) >= WATER_PREP_DURATION {
                    info!("Opening bridge...");
                    next_state = BridgeState::Opening(now);
                }
            }

            BridgeState::Opening(start_time) => {
                if now.duration_since(start_time) >= BRIDGE_TIMEOUT {
                    next_state = BridgeState::Fault(FaultReason::OpeningTimeout);
                } else if sensors.stepper_pos >= FULL_ROTATION {
                    if sensors.reed_closed {
                        // Reed still triggered after a full stepper cycle means the bridge
                        // deck never left the closed position — motor stalled (too heavy?).
                        error!(
                            "[OPENING] Stepper completed {} steps but reed is still closed! \
                             Bridge did not open. Motor stall or overload? stepper_pos={}",
                            FULL_ROTATION, sensors.stepper_pos
                        );
                        next_state = BridgeState::Fault(FaultReason::MotorStall);
                    } else {
                        info!("Bridge open! stepper_pos={}", sensors.stepper_pos);
                        next_state = BridgeState::Open(now);
                    }
                }
            }

            BridgeState::Open(start_time) => {
                // Only proceed to closing when the waterway is clear AND the minimum
                // wait has elapsed. If a boat/obstacle is still present we simply keep
                // waiting — no timer reset needed.
                if now.duration_since(start_time) >= BRIDGE_WAIT_DURATION
                    && !sensors.boat_detected
                    && !sensors.obstacle_detected
                {
                    info!("Checking clearance for closing...");
                    next_state = BridgeState::ClosingCheckClearance(now);
                }
            }

            BridgeState::ClosingCheckClearance(start_time) => {
                if sensors.boat_detected || sensors.obstacle_detected {
                    info!("[FSM] Boat/Obstacle during clearance check, re-opening wait.");
                    next_state = BridgeState::Open(now);
                } else if now.duration_since(start_time) >= CLEARANCE_CHECK_DURATION {
                    info!("Closing bridge...");
                    next_state = BridgeState::Closing(now);
                }
            }

            BridgeState::Closing(start_time) => {
                if sensors.obstacle_detected {
                    // Boat still crossing — abort close and wait again.
                    warn!("[CLOSING] IR blocked while closing, re-opening!");
                    next_state = BridgeState::Opening(now);
                } else if now.duration_since(start_time) >= BRIDGE_TIMEOUT {
                    next_state = BridgeState::Fault(FaultReason::ClosingTimeout);
                } else if sensors.reed_closed {
                    info!("Bridge seated! Raising barriers.");
                    next_state = BridgeState::RaiseBarriers(now);
                }
            }

            BridgeState::RaiseBarriers(start_time) => {
                if now.duration_since(start_time) >= BARRIER_MOVE_DURATION {
                    info!("Barriers raised. Idle.");
                    next_state = BridgeState::IdleClosed;
                }
            }

            BridgeState::Fault(_) => {
                if sensors.encoder_sw_pressed {
                    info!("Reset button pressed, homing...");
                    next_state = BridgeState::Homing(now);
                }
            }
        }

        next_state
    }
}
