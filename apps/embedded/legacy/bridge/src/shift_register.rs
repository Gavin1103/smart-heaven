use anyhow::Result;
use esp_idf_hal::gpio::*;

/// Driver for a 74HC595 shift register.
/// Ref: https://www.ti.com/lit/ds/symlink/sn74hc595.pdf
pub struct ShiftRegister<'d> {
    ser: PinDriver<'d, AnyOutputPin, Output>,
    srclk: PinDriver<'d, AnyOutputPin, Output>,
    rclk: PinDriver<'d, AnyOutputPin, Output>,
}

impl<'d> ShiftRegister<'d> {
    pub fn new(
        ser: PinDriver<'d, AnyOutputPin, Output>,
        srclk: PinDriver<'d, AnyOutputPin, Output>,
        rclk: PinDriver<'d, AnyOutputPin, Output>,
    ) -> Self {
        Self { ser, srclk, rclk }
    }

    /// Shifts 8 bits into the register and latches them to the outputs.
    pub fn write_byte(&mut self, data: u8) -> Result<()> {
        for i in (0..8).rev() {
            if (data & (1 << i)) != 0 {
                self.ser.set_high()?;
            } else {
                self.ser.set_low()?;
            }
            self.srclk.set_high()?;
            self.srclk.set_low()?;
        }
        self.rclk.set_high()?;
        self.rclk.set_low()?;
        Ok(())
    }
}
