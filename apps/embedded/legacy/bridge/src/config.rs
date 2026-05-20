#![allow(dead_code)]
use embassy_time::Duration;

// --- MOTION & STEPPER CONSTANTS ---
pub const FULL_ROTATION: i32 = 6400; // Steps for a complete bridge movement (90 degrees or full rotation depends on gearing)

// ULN2003 / 28BYJ-48 half-step sequence
pub const STEP_SEQUENCE: [[bool; 4]; 8] = [
    [true, false, false, false],
    [true, true, false, false],
    [false, true, false, false],
    [false, true, true, false],
    [false, false, true, false],
    [false, false, true, true],
    [false, false, false, true],
    [true, false, false, true],
];

// --- DURATIONS & TIMING ---
pub const FLASH_TICK_MS: u64 = 250; // Milliseconds between traffic light flashes
pub const BARRIER_MOVE_DURATION: Duration = Duration::from_millis(2000);
pub const WARNING_DURATION: Duration = Duration::from_secs(3);
pub const BRIDGE_WAIT_DURATION: Duration = Duration::from_secs(30);
pub const STEP_INTERVAL: Duration = Duration::from_millis(5); // Minimum time between stepper steps

// --- SENSOR CONSTANTS ---
pub const BOAT_DETECTION_DISTANCE_MM: u32 = 100; // Trigger distance for ultrasonic sensors
pub const BOAT_DETECTION_CONFIRMATION: Duration = Duration::from_millis(1000); // Time boat must be detected before acting
pub const WATER_PREP_DURATION: Duration = Duration::from_millis(2000);
pub const CLEARANCE_CHECK_DURATION: Duration = Duration::from_millis(2000);
pub const BRIDGE_TIMEOUT: Duration = Duration::from_secs(40);
pub const HOMING_TIMEOUT: Duration = Duration::from_secs(30);
pub const SENSOR_READ_TIMEOUT: Duration = Duration::from_secs(2);

pub const HOMING_EXTRA_STEPS: i32 = 200; // Extra steps after reed triggers to fully seat the bridge
