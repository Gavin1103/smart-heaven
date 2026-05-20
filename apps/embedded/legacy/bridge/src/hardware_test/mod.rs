use crate::buzzer::Buzzer;
use crate::encoder::Encoder;
use crate::lights::TrafficLights;
use crate::sensors::{DistanceSensor, Hx711, ObstacleSensor, ReedSensor};
use crate::stepper::StepperPins;
use crate::traits::EncoderTrait;
use anyhow::Result;
use embassy_time::Timer;
use log::info;

pub mod buzzer;
pub mod lights;
pub mod barriers;
pub mod stepper;
pub mod distance_sensors;
pub mod obstacle_sensor;
pub mod reed_sensor;
pub mod load_cell;

pub struct HardwareDevices<'d> {
    pub stepper: StepperPins<'d>,
    pub barriers: crate::servo_barrier::BarrierPair<'d>,
    pub sensor1: DistanceSensor<'d>,
    pub sensor2: DistanceSensor<'d>,
    pub reed: ReedSensor<'d>,
    pub obstacle: ObstacleSensor<'d>,
    pub encoder: Encoder<'d>,
    pub lights: TrafficLights<'d>,
    pub buzzer: Buzzer<'d>,
    pub hx711: Hx711<'d>,
}

pub struct HardwareTestConfig {
    pub test_buzzer: bool,
    pub test_lights: bool,
    pub test_barriers: bool,
    pub test_stepper: bool,
    pub test_distance_sensors: bool,
    pub test_obstacle_sensor: bool,
    pub test_reed_sensor: bool,
    pub test_load_cell: bool,
}

impl Default for HardwareTestConfig {
    fn default() -> Self {
        Self {
            test_buzzer: true,
            test_lights: true,
            test_barriers: true,
            test_stepper: true,
            test_distance_sensors: true,
            test_obstacle_sensor: true,
            test_reed_sensor: true,
            test_load_cell: true,
        }
    }
}

pub async fn wait_for_user(encoder: &mut Encoder<'_>, prompt: &str) -> Result<()> {
    info!("--------------------------------------------------");
    info!("{}", prompt);
    info!("  >>> Press encoder button to START this test <<<");
    info!("--------------------------------------------------");

    // Wait for press
    loop {
        if encoder.is_pressed() {
            break;
        }
        Timer::after_millis(50).await;
    }
    // Wait for release so we don't accidentally skip the next test
    while encoder.is_pressed() {
        Timer::after_millis(50).await;
    }
    info!("Starting...");
    Ok(())
}

pub async fn run_hardware_tests(
    devices: HardwareDevices<'static>,
) -> Result<()> {
    let mut devices = devices;
    
    // --- Tweak these to turn on/off tests ---
    let config = HardwareTestConfig {
        test_buzzer: true,
        test_lights: true,
        test_barriers: true,
        test_stepper: true,
        test_distance_sensors: true,
        test_obstacle_sensor: true,
        test_reed_sensor: true,
        test_load_cell: true,
    };

    info!("=== ENTERING INTERACTIVE HARDWARE TEST MODE ===");

    if config.test_buzzer {
        buzzer::test_buzzer(&mut devices.buzzer, &mut devices.encoder).await?;
    }

    if config.test_lights {
        lights::test_lights(&mut devices.lights, &mut devices.encoder).await?;
    }

    if config.test_barriers {
        barriers::test_barriers(&mut devices.barriers, &mut devices.encoder).await?;
    }

    if config.test_stepper {
        stepper::test_stepper(&mut devices.stepper, &mut devices.barriers, &mut devices.encoder).await?;
    }

    if config.test_distance_sensors {
        distance_sensors::test_sensors(&mut devices.sensor1, &mut devices.sensor2, &mut devices.encoder).await?;
    }

    if config.test_obstacle_sensor {
        obstacle_sensor::test_sensor(&mut devices.obstacle, &mut devices.encoder).await?;
    }

    if config.test_reed_sensor {
        reed_sensor::test_sensor(&mut devices.reed, &mut devices.encoder).await?;
    }

    if config.test_load_cell {
        load_cell::test_load_cell(&mut devices.hx711, &mut devices.lights, &mut devices.buzzer, &mut devices.encoder).await?;
    }

    info!("=== ALL HARDWARE TESTS COMPLETED SUCCESSFULLY ===");
    Ok(())
}
