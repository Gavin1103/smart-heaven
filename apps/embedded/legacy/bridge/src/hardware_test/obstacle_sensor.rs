use crate::encoder::Encoder;
use crate::hardware_test::wait_for_user;
use crate::sensors::ObstacleSensor;
use crate::traits::{EncoderTrait, ObstacleSensorTrait};
use anyhow::Result;
use embassy_time::Timer;
use log::info;

pub async fn test_sensor(sensor: &ObstacleSensor<'_>, encoder: &mut Encoder<'_>) -> Result<()> {
    wait_for_user(
        encoder,
        "TEST 8: Obstacle Laser (IR) - Press button to STOP",
    )
    .await?;
    loop {
        info!("  Obstacle blocked: {}", sensor.is_blocked());
        Timer::after_millis(500).await;
        if encoder.is_pressed() {
            break;
        }
    }
    while encoder.is_pressed() {
        Timer::after_millis(50).await;
    }
    Ok(())
}
