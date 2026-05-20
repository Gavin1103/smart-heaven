use crate::encoder::Encoder;
use crate::hardware_test::wait_for_user;
use crate::sensors::DistanceSensor;
use crate::traits::{DistanceSensorTrait, EncoderTrait};
use anyhow::Result;
use embassy_time::Timer;
use log::info;

pub async fn test_sensors(
    sensor1: &mut DistanceSensor<'_>,
    sensor2: &mut DistanceSensor<'_>,
    encoder: &mut Encoder<'_>,
) -> Result<()> {
    wait_for_user(
        encoder,
        "TEST 6: Ultrasonic Sensor 1 (East) - Press button to STOP",
    )
    .await?;
    loop {
        if let Some(d) = sensor1.measure_distance_mm().await? {
            info!("  Dist 1: {}mm", d);
        } else {
            info!("  Dist 1: Timeout");
        }
        Timer::after_millis(500).await;
        if encoder.is_pressed() {
            break;
        }
    }
    while encoder.is_pressed() {
        Timer::after_millis(50).await;
    } // Debounce

    wait_for_user(
        encoder,
        "TEST 7: Ultrasonic Sensor 2 (West) - Press button to STOP",
    )
    .await?;
    loop {
        if let Some(d) = sensor2.measure_distance_mm().await? {
            info!("  Dist 2: {}mm", d);
        } else {
            info!("  Dist 2: Timeout");
        }
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
