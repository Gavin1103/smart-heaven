use crate::traits::StepperTrait;
use anyhow::Result;
use esp_idf_hal::gpio::*;

pub struct StepperPins<'d> {
    pub in1: PinDriver<'d, AnyOutputPin, Output>,
    pub in2: PinDriver<'d, AnyOutputPin, Output>,
    pub in3: PinDriver<'d, AnyOutputPin, Output>,
    pub in4: PinDriver<'d, AnyOutputPin, Output>,
}

impl<'d> StepperTrait for StepperPins<'d> {
    fn apply_step(&mut self, step: &[bool; 4]) -> Result<()> {
        if step[0] {
            self.in1.set_high()?
        } else {
            self.in1.set_low()?
        };
        if step[1] {
            self.in2.set_high()?
        } else {
            self.in2.set_low()?
        };
        if step[2] {
            self.in3.set_high()?
        } else {
            self.in3.set_low()?
        };
        if step[3] {
            self.in4.set_high()?
        } else {
            self.in4.set_low()?
        };
        Ok(())
    }

    fn stop(&mut self) -> Result<()> {
        self.apply_step(&[false, false, false, false])
    }
}

