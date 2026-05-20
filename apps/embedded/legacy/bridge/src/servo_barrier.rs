use crate::traits::BarrierPairTrait;
use anyhow::Result;
use embassy_time::{Duration, Timer};
use esp_idf_hal::ledc::*;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BarrierPosition {
    #[allow(dead_code)]
    Open,
    #[allow(dead_code)]
    Closed,
    Stopped,
    Unknown,
}

#[derive(Clone, Copy)]
pub struct BarrierCalibration {
    pub open_duty: u32,
    pub closed_duty: u32,
}

impl BarrierCalibration {
    pub const fn new(open_duty: u32, closed_duty: u32) -> Self {
        Self {
            open_duty,
            closed_duty,
        }
    }
}

pub struct ServoBarrier<'d> {
    driver: LedcDriver<'d>,
    calibration: BarrierCalibration,
}

impl<'d> ServoBarrier<'d> {
    pub fn new(driver: LedcDriver<'d>, calibration: BarrierCalibration) -> Self {
        Self {
            driver,
            calibration,
        }
    }

    pub fn open(&mut self) -> Result<()> {
        self.driver.set_duty(self.calibration.open_duty)?;
        Ok(())
    }

    pub fn close(&mut self) -> Result<()> {
        self.driver.set_duty(self.calibration.closed_duty)?;
        Ok(())
    }

    pub fn stop(&mut self) -> Result<()> {
        self.driver.set_duty(0)?;
        Ok(())
    }
}

pub struct BarrierPair<'d> {
    pub left: ServoBarrier<'d>,
    pub right: ServoBarrier<'d>,
    current_pos: BarrierPosition,
}

impl<'d> BarrierPair<'d> {
    pub fn new(left: ServoBarrier<'d>, right: ServoBarrier<'d>) -> Self {
        Self {
            left,
            right,
            current_pos: BarrierPosition::Unknown,
        }
    }
}

impl<'d> BarrierPairTrait for BarrierPair<'d> {
    #[allow(dead_code)]
    async fn open(&mut self) -> Result<()> {
        if self.current_pos == BarrierPosition::Open {
            return Ok(());
        }

        // We open barriers one by one to avoid high peak current
        // that could cause a brownout on the power supply.
        self.left.open()?;
        Timer::after(Duration::from_millis(800)).await;
        self.left.stop()?;

        Timer::after(Duration::from_millis(200)).await;

        self.right.open()?;
        Timer::after(Duration::from_millis(800)).await;
        self.right.stop()?;

        self.current_pos = BarrierPosition::Open;
        Ok(())
    }

    #[allow(dead_code)]
    async fn close(&mut self) -> Result<()> {
        if self.current_pos == BarrierPosition::Closed {
            return Ok(());
        }

        // We close barriers one by one to avoid high peak current.
        self.left.close()?;
        Timer::after(Duration::from_millis(800)).await;
        self.left.stop()?;

        Timer::after(Duration::from_millis(200)).await;

        self.right.close()?;
        Timer::after(Duration::from_millis(800)).await;
        self.right.stop()?;

        self.current_pos = BarrierPosition::Closed;
        Ok(())
    }

    fn stop(&mut self) -> Result<()> {
        self.left.stop()?;
        self.right.stop()?;
        self.current_pos = BarrierPosition::Stopped;
        Ok(())
    }

    fn open_left(&mut self) -> Result<()> {
        self.left.open()
    }

    fn close_left(&mut self) -> Result<()> {
        self.left.close()
    }

    fn stop_left(&mut self) -> Result<()> {
        self.left.stop()
    }

    fn open_right(&mut self) -> Result<()> {
        self.right.open()
    }

    fn close_right(&mut self) -> Result<()> {
        self.right.close()
    }

    fn stop_right(&mut self) -> Result<()> {
        self.right.stop()
    }
}
