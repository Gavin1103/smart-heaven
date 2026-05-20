#![allow(dead_code)]
use crate::traits::*;
use anyhow::Result;

pub struct MockDistanceSensor {
    pub value: Option<u32>,
}
impl DistanceSensorTrait for MockDistanceSensor {
    async fn measure_distance_mm(&mut self) -> Result<Option<u32>> {
        Ok(self.value)
    }
}

pub struct MockReedSensor {
    pub closed: bool,
}
impl ReedSensorTrait for MockReedSensor {
    fn is_closed(&self) -> bool {
        self.closed
    }
}

pub struct MockObstacleSensor {
    pub blocked: bool,
}
impl ObstacleSensorTrait for MockObstacleSensor {
    fn is_blocked(&self) -> bool {
        self.blocked
    }
}

pub struct MockWeightSensor {
    pub value: i32,
}
impl WeightSensorTrait for MockWeightSensor {
    async fn read(&mut self) -> Result<i32> {
        Ok(self.value)
    }
}

pub struct MockStepper {}
impl StepperTrait for MockStepper {
    fn apply_step(&mut self, _step: &[bool; 4]) -> Result<()> {
        Ok(())
    }
    fn stop(&mut self) -> Result<()> {
        Ok(())
    }
}

pub struct MockTrafficLights {
    pub state: u8,
}
impl TrafficLightsTrait for MockTrafficLights {
    fn set_state(&mut self, mask: u8) -> Result<()> {
        self.state = mask;
        Ok(())
    }
    fn set_idle(&mut self) -> Result<()> {
        Ok(())
    }
    fn set_warning(&mut self, _flashing_on: bool) -> Result<()> {
        Ok(())
    }
    fn set_prep(&mut self) -> Result<()> {
        Ok(())
    }
    fn set_open(&mut self, _flashing_on: bool) -> Result<()> {
        Ok(())
    }
}

pub struct MockBuzzer {
    pub on: bool,
}
impl BuzzerTrait for MockBuzzer {
    fn set_on(&mut self, on: bool) -> Result<()> {
        self.on = on;
        Ok(())
    }
    #[cfg(feature = "hil")]
    fn set_frequency(&mut self, _freq: u32) -> Result<()> {
        Ok(())
    }
    #[cfg(feature = "hil")]
    fn turn_off(&mut self) -> Result<()> {
        self.on = false;
        Ok(())
    }
}

pub struct MockBarrierPair {
    pub open_called: bool,
    pub close_called: bool,
}
impl BarrierPairTrait for MockBarrierPair {
    async fn open(&mut self) -> Result<()> {
        self.open_called = true;
        Ok(())
    }
    async fn close(&mut self) -> Result<()> {
        self.close_called = true;
        Ok(())
    }
    fn stop(&mut self) -> Result<()> {
        Ok(())
    }
    fn open_left(&mut self) -> Result<()> {
        Ok(())
    }
    fn close_left(&mut self) -> Result<()> {
        Ok(())
    }
    fn stop_left(&mut self) -> Result<()> {
        Ok(())
    }
    fn open_right(&mut self) -> Result<()> {
        Ok(())
    }
    fn close_right(&mut self) -> Result<()> {
        Ok(())
    }
    fn stop_right(&mut self) -> Result<()> {
        Ok(())
    }
}

pub struct MockEncoder {
    pub pressed: bool,
    pub position: i32,
}
impl EncoderTrait for MockEncoder {
    fn update(&mut self) -> bool {
        false
    }
    fn get_position(&self) -> i32 {
        self.position
    }
    fn reset(&mut self) {
        self.position = 0;
    }
    fn is_pressed(&self) -> bool {
        self.pressed
    }
}
