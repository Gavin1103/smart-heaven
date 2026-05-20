use crate::traits::EncoderTrait;
use esp_idf_hal::gpio::*;
#[allow(dead_code)]
/// Driver for a rotary encoder (e.g. KY-040).
/// Ref: https://sensorkit.joy-it.net/en/sensors/ky-040
pub struct Encoder<'d> {
    clk: PinDriver<'d, AnyIOPin, Input>,
    dt: PinDriver<'d, AnyIOPin, Input>,
    sw: PinDriver<'d, AnyIOPin, Input>,
    last_clk: bool,
    position: i32,
}

impl<'d> Encoder<'d> {
    pub fn new(
        clk: PinDriver<'d, AnyIOPin, Input>,
        dt: PinDriver<'d, AnyIOPin, Input>,
        sw: PinDriver<'d, AnyIOPin, Input>,
    ) -> Self {
        let last_clk = clk.is_high();
        Self {
            clk,
            dt,
            sw,
            last_clk,
            position: 0,
        }
    }
}

impl<'d> EncoderTrait for Encoder<'d> {
    #[allow(dead_code)]
    /// Polling-based update of the encoder position. 
    /// Should be called frequently (e.g. in a fast loop) to avoid missing steps.
    fn update(&mut self) -> bool {
        let current_clk = self.clk.is_high();
        let mut changed = false;
        if current_clk != self.last_clk {
            if self.dt.is_high() != current_clk {
                self.position += 1;
            } else {
                self.position -= 1;
            }
            changed = true;
        }
        self.last_clk = current_clk;
        changed
    }

    #[allow(dead_code)]
    fn get_position(&self) -> i32 {
        self.position
    }

    #[allow(dead_code)]
    fn reset(&mut self) {
        self.position = 0;
    }

    fn is_pressed(&self) -> bool {
        self.sw.is_low()
    }
}
