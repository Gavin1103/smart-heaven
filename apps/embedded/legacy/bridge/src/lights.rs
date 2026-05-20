use crate::shift_register::ShiftRegister;
use crate::traits::TrafficLightsTrait;
use anyhow::Result;

pub struct TrafficLights<'d> {
    register: ShiftRegister<'d>,
}

// Bitmask for the 74HC595 shift register outputs. 
// Match the physical wiring of LEDs to the register.
pub const ROAD_RED: u8 = 1 << 0;
pub const ROAD_GREEN: u8 = 1 << 1;
pub const ROAD_WARNING: u8 = 1 << 2;
pub const BOAT_RED: u8 = 1 << 3;
pub const BOAT_GREEN: u8 = 1 << 4;

impl<'d> TrafficLights<'d> {
    pub fn new(register: ShiftRegister<'d>) -> Self {
        Self { register }
    }
}

impl<'d> TrafficLightsTrait for TrafficLights<'d> {
    fn set_state(&mut self, mask: u8) -> Result<()> {
        self.register.write_byte(mask)
    }

    #[allow(dead_code)]
    fn set_idle(&mut self) -> Result<()> {
        self.set_state(ROAD_GREEN | BOAT_RED)
    }

    #[allow(dead_code)]
    fn set_warning(&mut self, flashing_on: bool) -> Result<()> {
        let mut mask = BOAT_RED;
        if flashing_on {
            mask |= ROAD_WARNING;
        } else {
            mask |= ROAD_RED;
        }
        self.set_state(mask)
    }

    #[allow(dead_code)]
    fn set_prep(&mut self) -> Result<()> {
        self.set_state(ROAD_RED | BOAT_RED | BOAT_GREEN)
    }

    #[allow(dead_code)]
    fn set_open(&mut self, flashing_on: bool) -> Result<()> {
        let mut mask = ROAD_RED;
        if flashing_on {
            mask |= BOAT_GREEN;
        }
        self.set_state(mask)
    }
}
