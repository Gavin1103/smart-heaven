use crate::traits::BuzzerTrait;
use anyhow::Result;
use esp_idf_hal::gpio::*;

/// Driver for an active buzzer (e.g. KY-012).
/// Ref: https://sensorkit.joy-it.net/en/sensors/ky-012
pub struct Buzzer<'d> {
    pin: PinDriver<'d, AnyOutputPin, Output>,
}

impl<'d> Buzzer<'d> {
    pub fn new(mut pin: PinDriver<'d, AnyOutputPin, Output>) -> Result<Self> {
        pin.set_high()?; // Assuming active low, set high to turn off initially
        Ok(Self { pin })
    }
}

impl<'d> BuzzerTrait for Buzzer<'d> {
    fn set_on(&mut self, on: bool) -> Result<()> {
        if on {
            self.pin.set_high()?; // KY-012: High = ON
        } else {
            self.pin.set_low()?; // KY-012: Low = OFF
        }
        Ok(())
    }

    #[cfg(feature = "hil")]
    fn set_frequency(&mut self, _freq: u32) -> Result<()> {
        // KY-012 is a passive buzzer but this implementation seems to treat it as active (High=ON)
        // For an active buzzer, we can't really set frequency, just ON/OFF
        if _freq > 0 {
            self.set_on(true)
        } else {
            self.set_on(false)
        }
    }

    #[cfg(feature = "hil")]
    fn turn_off(&mut self) -> Result<()> {
        self.set_on(false)
    }
}
