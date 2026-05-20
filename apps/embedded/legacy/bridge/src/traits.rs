#![allow(dead_code)]
use anyhow::Result;

/// Interface for ultrasonic distance sensors (e.g. HC-SR04).
pub trait DistanceSensorTrait {
    /// Measures the distance in millimeters. Returns None if measurement fails.
    async fn measure_distance_mm(&mut self) -> Result<Option<u32>>;
}

/// Interface for digital obstacle sensors (e.g. IR proximity sensor).
pub trait ObstacleSensorTrait {
    /// Returns true if an obstacle is detected (sensor is blocked).
    fn is_blocked(&self) -> bool;
}

/// Interface for reed sensors (magnetic switches).
pub trait ReedSensorTrait {
    /// Returns true if the magnet is near (switch is closed).
    fn is_closed(&self) -> bool;
}

/// Interface for weight sensors (load cells via HX711).
pub trait WeightSensorTrait {
    /// Reads the raw value from the ADC.
    async fn read(&mut self) -> Result<i32>;
}

/// Interface for a 4-wire stepper motor controller (e.g. ULN2003).
pub trait StepperTrait {
    /// Applies a single step pattern to the motor coils.
    fn apply_step(&mut self, step: &[bool; 4]) -> Result<()>;
    /// De-energizes all coils to prevent overheating.
    fn stop(&mut self) -> Result<()>;
}

pub trait TrafficLightsTrait {
    fn set_state(&mut self, mask: u8) -> Result<()>;
    fn set_idle(&mut self) -> Result<()>;
    fn set_warning(&mut self, flashing_on: bool) -> Result<()>;
    fn set_prep(&mut self) -> Result<()>;
    fn set_open(&mut self, flashing_on: bool) -> Result<()>;
}

pub trait BuzzerTrait {
    fn set_on(&mut self, on: bool) -> Result<()>;
    #[cfg(feature = "hil")]
    fn set_frequency(&mut self, freq: u32) -> Result<()>;
    #[cfg(feature = "hil")]
    fn turn_off(&mut self) -> Result<()>;
}

pub trait BarrierPairTrait {
    async fn open(&mut self) -> Result<()>;
    async fn close(&mut self) -> Result<()>;
    fn stop(&mut self) -> Result<()>;

    fn open_left(&mut self) -> Result<()>;
    fn close_left(&mut self) -> Result<()>;
    fn stop_left(&mut self) -> Result<()>;
    fn open_right(&mut self) -> Result<()>;
    fn close_right(&mut self) -> Result<()>;
    fn stop_right(&mut self) -> Result<()>;
}

pub trait EncoderTrait {
    fn update(&mut self) -> bool;
    fn get_position(&self) -> i32;
    fn reset(&mut self);
    fn is_pressed(&self) -> bool;
}
