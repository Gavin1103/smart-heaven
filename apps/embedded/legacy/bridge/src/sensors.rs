use crate::traits::*;
use anyhow::Result;
use embassy_time::{Duration, Instant, Timer};
use esp_idf_hal::gpio::*;
use log::debug;

/// Busy-wait for the given number of microseconds without yielding to the async executor.
/// Used for time-critical bit-banging (e.g. HX711) where an await point would let another
/// task run and potentially violate the 60 µs inter-clock-pulse deadline.
#[inline(always)]
fn delay_us_blocking(us: u32) {
    // SAFETY: ets_delay_us is a simple busy-wait loop with no side effects.
    unsafe { esp_idf_sys::ets_delay_us(us) };
}

/// Ultrasonic distance sensor (HC-SR04).
/// Ref: https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf
pub struct DistanceSensor<'d> {
    trig: PinDriver<'d, AnyIOPin, Output>,
    echo: PinDriver<'d, AnyIOPin, Input>,
}

impl<'d> DistanceSensor<'d> {
    pub fn new(
        trig: PinDriver<'d, AnyIOPin, Output>,
        echo: PinDriver<'d, AnyIOPin, Input>,
    ) -> Self {
        Self { trig, echo }
    }
}

impl<'d> DistanceSensorTrait for DistanceSensor<'d> {
    async fn measure_distance_mm(&mut self) -> Result<Option<u32>> {
        // Attempt up to 3 times to get a valid reading (reduces noise/jitter)
        for attempt in 0..3 {
            // 1. Ensure trig is low and let it settle
            self.trig.set_low()?;
            Timer::after_micros(10).await;

            // 2. Send 12us trigger pulse (min 10us)
            self.trig.set_high()?;
            Timer::after_micros(12).await;
            self.trig.set_low()?;

            // 3. Wait for echo to go high (start of pulse)
            let t0 = Instant::now();
            let mut echo_started = false;
            while t0.elapsed() < Duration::from_millis(50) {
                if self.echo.is_high() {
                    echo_started = true;
                    break;
                }
                Timer::after_micros(50).await; // Poll start less frequently
            }

            if !echo_started {
                debug!(
                    "[HC-SR04] Timeout waiting for echo start (attempt {})",
                    attempt + 1
                );
                Timer::after_millis(10).await; // Settle time before retry
                continue;
            }

            // 4. Wait for echo to go low (end of pulse)
            let start = Instant::now();
            let mut echo_ended = false;
            while start.elapsed() < Duration::from_millis(50) {
                if self.echo.is_low() {
                    echo_ended = true;
                    break;
                }
                Timer::after_micros(20).await; // Poll end frequently for better precision
            }

            if !echo_ended {
                debug!(
                    "[HC-SR04] Timeout waiting for echo end (attempt {})",
                    attempt + 1
                );
                Timer::after_millis(10).await;
                continue;
            }

            let pulse_us = start.elapsed().as_micros();

            // 5. Basic sanity check (max range ~4m = ~23ms)
            // Speed of sound = 343 m/s = 0.343 mm/us.
            // Distance = (Time * Speed) / 2 (round trip)
            if pulse_us > 100 && pulse_us < 35000 {
                return Ok(Some((pulse_us as u32 * 343) / 2000));
            } else if attempt < 2 {
                debug!(
                    "[HC-SR04] Invalid pulse width: {}us (attempt {})",
                    pulse_us,
                    attempt + 1
                );
                Timer::after_millis(10).await;
                continue;
            }
        }
        debug!("[HC-SR04] All measurement attempts timed out or failed.");
        Ok(None)
    }
}

pub struct ObstacleSensor<'d> {
    pin: PinDriver<'d, AnyIOPin, Input>, // Changed to AnyIOPin
}

impl<'d> ObstacleSensor<'d> {
    pub fn new(pin: PinDriver<'d, AnyIOPin, Input>) -> Self {
        // Changed here too
        Self { pin }
    }
}

impl<'d> ObstacleSensorTrait for ObstacleSensor<'d> {
    fn is_blocked(&self) -> bool {
        self.pin.is_low()
    }
}


pub struct ReedSensor<'d> {
    pin: PinDriver<'d, AnyIOPin, Input>,
}

impl<'d> ReedSensor<'d> {
    pub fn new(pin: PinDriver<'d, AnyIOPin, Input>) -> Self {
        Self { pin }
    }
}

impl<'d> ReedSensorTrait for ReedSensor<'d> {
    fn is_closed(&self) -> bool {
        self.pin.is_low()
    }
}



/// 24-bit ADC for bridge weight measurement (load cell).
/// Ref: https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf
pub struct Hx711<'d> {
    data: PinDriver<'d, AnyInputPin, Input>,
    sclk: PinDriver<'d, AnyOutputPin, Output>,
}

impl<'d> Hx711<'d> {
    pub fn new(
        data: PinDriver<'d, AnyInputPin, Input>,
        mut sclk: PinDriver<'d, AnyOutputPin, Output>,
    ) -> Result<Self> {
        sclk.set_low()?;
        Ok(Self { data, sclk })
    }
}

impl<'d> WeightSensorTrait for Hx711<'d> {
    async fn read(&mut self) -> Result<i32> {
        let mut value: i32 = 0;
        // Wait for data to go low (ready)
        let t0 = Instant::now();
        while self.data.is_high() {
            if t0.elapsed() > Duration::from_millis(100) {
                return Ok(0);
            }
            Timer::after_micros(10).await;
        }

        // Clock out 24 bits. Use blocking delays — the HX711 will enter power-down
        // if SCLK stays HIGH for more than 60 µs, which can happen if we use
        // Timer::after_micros (an await point that lets other Embassy tasks run).
        for _ in 0..24 {
            self.sclk.set_high()?;
            delay_us_blocking(1);
            value <<= 1;
            if self.data.is_high() {
                value |= 1;
            }
            self.sclk.set_low()?;
            delay_us_blocking(1);
        }

        // 25th pulse sets gain to 128 for next read
        self.sclk.set_high()?;
        delay_us_blocking(1);
        self.sclk.set_low()?;

        // Sign extend 24-bit to 32-bit
        if (value & 0x800000) != 0 {
            value |= !0xffffff;
        }
        Ok(value)
    }
}
