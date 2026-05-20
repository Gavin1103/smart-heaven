use crate::buzzer::Buzzer;
use crate::encoder::Encoder;
use crate::hardware_test::wait_for_user;
use crate::lights::{TrafficLights, ROAD_GREEN, ROAD_RED};
use crate::sensors::Hx711;
use crate::traits::{BuzzerTrait, EncoderTrait, TrafficLightsTrait, WeightSensorTrait};
use anyhow::Result;
use embassy_time::Timer;
use log::info;

pub async fn test_load_cell(
    hx711: &mut Hx711<'static>,
    lights: &mut TrafficLights<'_>,
    buzzer: &mut Buzzer<'_>,
    encoder: &mut Encoder<'_>,
) -> Result<()> {
    wait_for_user(encoder, "TEST 11: HX711 - Taring... (STAY STILL)").await?;

    // 1. Initial Tare
    let mut baseline: f32 = 0.0;
    let mut samples = 0;
    while samples < 30 {
        // More samples for a rock-solid start
        if let Ok(val) = hx711.read().await {
            if val != 0 && val != -1 && val != 8388607 {
                baseline += val as f32;
                samples += 1;
            }
        }
        Timer::after_millis(10).await;
    }
    baseline /= 30.0;
    info!("Bridge Ready. Baseline set to: {}", baseline);

    // --- CALIBRATION CONSTANTS ---
    const CAR_THRESHOLD: f32 = 90.0; // Higher threshold to ignore the heavy drift
    const SCALE_FACTOR: f32 = 0.76;
    let mut car_counter = 0;

    loop {
        if let Ok(val) = hx711.read().await {
            // Ignore glitched readings
            if val != 0 && val != -1 && val != 8388607 {
                let current_raw = val as f32;
                let diff = (current_raw - baseline).abs();
                let grams = diff / SCALE_FACTOR;

                if grams > CAR_THRESHOLD {
                    car_counter += 1;
                    info!(
                        "  [CAR ATTEMPT] Grams: {:.1} | Counter: {}",
                        grams, car_counter
                    );
                } else {
                    car_counter = 0;

                    // --- THE DRIFT KILLER ---
                    // If weight is low (< 90g), we assume it's just drift.
                    // We pull the baseline toward the current reading very aggressively (20% change)
                    // to keep the "Grams" at 0.0.
                    baseline = (baseline * 0.8) + (current_raw * 0.2);

                    info!("  Bridge Empty: 0.0g (Stable)");
                }

                // Trigger lights/buzzer
                if car_counter >= 3 {
                    info!("  >>> RED LIGHT: CAR DETECTED! <<<");
                    let _ = lights.set_state(ROAD_RED);
                    let _ = buzzer.set_on(true);
                } else {
                    let _ = lights.set_state(ROAD_GREEN);
                    let _ = buzzer.set_on(false);
                }
            }
        }

        Timer::after_millis(150).await;
        if encoder.is_pressed() {
            break;
        }
    }
    while encoder.is_pressed() {
        Timer::after_millis(50).await;
    }
    Ok(())
}
