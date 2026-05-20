use crate::encoder::Encoder;
use crate::hardware_test::wait_for_user;
use crate::sensors::ReedSensor;
use crate::traits::{EncoderTrait, ReedSensorTrait};
use anyhow::Result;
use embassy_time::Timer;
use log::info;

pub async fn test_sensor(sensor: &ReedSensor<'_>, encoder: &mut Encoder<'_>) -> Result<()> {
    wait_for_user(
        encoder,
        "TEST 9: Reed Switch (Magnet) - Press button to STOP",
    )
    .await?;
    loop {
        info!("  Reed closed (Magnet near): {}", sensor.is_closed());
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
