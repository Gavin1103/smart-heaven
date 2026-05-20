use crate::config::*;
use crate::traits::*;
use anyhow::Result;
use embassy_time::{Duration, Instant, Timer};
use log::info;

#[allow(dead_code)]
pub struct HilDevices<S, B, D, R, O, E, L, BUZ, W> {
    pub stepper: S,
    pub barriers: B,
    pub ds1: D,
    pub ds2: D,
    pub reed: R,
    pub obstacle: O,
    pub encoder: E,
    pub lights: L,
    pub buzzer: BUZ,
    pub hx711: W,
}

#[allow(dead_code)]
pub async fn run_hil_tests<S, B, D, R, O, E, L, BUZ, W>(
    devices: HilDevices<S, B, D, R, O, E, L, BUZ, W>,
) -> Result<()>
where
    S: StepperTrait,
    B: BarrierPairTrait,
    D: DistanceSensorTrait,
    R: ReedSensorTrait,
    O: ObstacleSensorTrait,
    E: EncoderTrait,
    L: TrafficLightsTrait,
    BUZ: BuzzerTrait,
    W: WeightSensorTrait,
{
    let HilDevices {
        mut stepper,
        mut barriers,
        mut ds1,
        mut ds2,
        reed,
        obstacle,
        encoder,
        mut lights,
        mut buzzer,
        mut hx711,
    } = devices;
    info!("--- Starting Hardware-In-The-Loop (HIL) Tests ---");

    // 1. Test Buzzer
    info!("[HIL] Testing Buzzer...");
    buzzer.set_frequency(1000)?;
    Timer::after(Duration::from_millis(500)).await;
    buzzer.turn_off()?;

    // 2. Test Lights
    info!("[HIL] Testing Traffic Lights (Cycle)...");
    lights.set_idle()?;
    Timer::after(Duration::from_millis(500)).await;
    lights.set_warning(true)?;
    Timer::after(Duration::from_millis(500)).await;
    lights.set_open(true)?;
    Timer::after(Duration::from_millis(500)).await;

    // 3. Test Sensors
    info!("[HIL] Testing Sensors (Real-time readings for 5s)...");
    let start = Instant::now();
    while start.elapsed() < Duration::from_secs(5) {
        let d1 = ds1.measure_distance_mm().await?;
        let d2 = ds2.measure_distance_mm().await?;
        let w = hx711.read().await?;
        info!(
            "Dist: {:?} / {:?}, Reed: {}, Obs: {}, Weight: {}, Enc: {}",
            d1,
            d2,
            reed.is_closed(),
            obstacle.is_blocked(),
            w,
            encoder.get_position()
        );
        Timer::after(Duration::from_millis(200)).await;
    }

    // 4. Test Barriers
    info!("[HIL] Testing Barriers (Open/Close)...");
    barriers.close().await?;
    Timer::after(Duration::from_millis(1000)).await;
    barriers.open().await?;

    // 5. Test Stepper
    info!("[HIL] Testing Stepper (100 steps)...");
    for i in 0..100 {
        stepper.apply_step(&STEP_SEQUENCE[i % 8])?;
        Timer::after(Duration::from_millis(5)).await;
    }
    stepper.stop()?;

    info!("--- HIL Tests Completed Successfully ---");
    Ok(())
}
