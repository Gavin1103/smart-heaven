mod buzzer;
mod config;
mod encoder;
#[cfg(feature = "hardware_tests")]
mod hardware_test;
#[cfg(feature = "hil")]
mod hil_tests;
mod lights;
#[cfg(test)]
mod mock;
mod sensors;
mod servo_barrier;
mod shift_register;
mod state_machine;
mod stepper;
#[cfg(test)]
mod tests;
mod traits;

#[cfg(all(not(feature = "hil"), not(feature = "hardware_tests")))]
mod tasks;

use anyhow::Result;
#[cfg(all(not(feature = "hil"), not(feature = "hardware_tests")))]
use embassy_sync::mutex::Mutex;
use esp_idf_hal::gpio::*;
use esp_idf_hal::ledc::config::TimerConfig;
use esp_idf_hal::ledc::*;
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_hal::prelude::*;
use esp_idf_svc::hal::task::block_on;
#[cfg(all(not(feature = "hil"), not(feature = "hardware_tests")))]
use futures::join;
#[cfg(all(not(feature = "hil"), not(feature = "hardware_tests")))]
use log::info;

use crate::buzzer::Buzzer;
use crate::encoder::Encoder;
use crate::lights::TrafficLights;
use crate::sensors::{DistanceSensor, ObstacleSensor, ReedSensor};
use crate::servo_barrier::ServoBarrier;
use crate::shift_register::ShiftRegister;
#[cfg(all(not(feature = "hil"), not(feature = "hardware_tests")))]
use crate::state_machine::{BridgeState, SharedSensors};
use crate::stepper::StepperPins;
#[cfg(all(not(feature = "hil"), not(feature = "hardware_tests")))]
use crate::tasks::*;


/// The entry point of the application. Initializes the ESP32 log and starts the bridge task.
fn main() {
    esp_idf_svc::sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();

    block_on(async {
        if let Err(e) = run_bridge().await {
            log::error!("Bridge error: {:?}", e);
        }
    });
}

/// Main orchestration function that initializes hardware, state, and launches concurrent tasks.
async fn run_bridge() -> Result<()> {
    let peripherals = Peripherals::take().unwrap();

    // --- PIN INITIALIZATION ---
    // Hardware initialization is quite verbose here.
    // Consider moving this to a dedicated 'Hardware' struct or module.
    
    // Shift Register Output Enable (OE) - Active Low, drive LOW to enable outputs
    let mut oe = PinDriver::output(peripherals.pins.gpio9)?;
    oe.set_low()?;

    let stepper = StepperPins {
        in1: PinDriver::output(peripherals.pins.gpio15.downgrade_output())?,
        in2: PinDriver::output(peripherals.pins.gpio16.downgrade_output())?,
        in3: PinDriver::output(peripherals.pins.gpio17.downgrade_output())?,
        in4: PinDriver::output(peripherals.pins.gpio18.downgrade_output())?,
    };

    let shift_reg = ShiftRegister::new(
        PinDriver::output(peripherals.pins.gpio6.downgrade_output())?,
        PinDriver::output(peripherals.pins.gpio7.downgrade_output())?,
        PinDriver::output(peripherals.pins.gpio8.downgrade_output())?,
    );
    let lights = TrafficLights::new(shift_reg);

    let distance_sensor1 = DistanceSensor::new(
        PinDriver::output(peripherals.pins.gpio21.downgrade())?,
        PinDriver::input(peripherals.pins.gpio4.downgrade())?,
    );

    let distance_sensor2 = DistanceSensor::new(
        PinDriver::output(peripherals.pins.gpio38.downgrade())?,
        PinDriver::input(peripherals.pins.gpio39.downgrade())?,
    );

    let mut reed_pin = PinDriver::input(peripherals.pins.gpio40.downgrade())?;
    reed_pin.set_pull(Pull::Up)?;
    let reed = ReedSensor::new(reed_pin);

    let mut obs_pin = PinDriver::input(peripherals.pins.gpio11.downgrade())?; // Changed downgrade_input() to downgrade()
    obs_pin.set_pull(esp_idf_hal::gpio::Pull::Up)?;
    let obstacle = ObstacleSensor::new(obs_pin);

    let buzzer = Buzzer::new(PinDriver::output(
        peripherals.pins.gpio12.downgrade_output(),
    )?)?;

    let mut sw_pin = PinDriver::input(peripherals.pins.gpio3.downgrade())?; // Changed
    sw_pin.set_pull(esp_idf_hal::gpio::Pull::Up)?;

    let encoder = Encoder::new(
        PinDriver::input(peripherals.pins.gpio1.downgrade())?, // Changed
        PinDriver::input(peripherals.pins.gpio2.downgrade())?, // Changed
        sw_pin,
    );

    // --- SERVO INITIALIZATION ---
    use esp_idf_hal::ledc::Resolution;
    let timer_driver = LedcTimerDriver::new(
        peripherals.ledc.timer0,
        &TimerConfig::new()
            .frequency(50_u32.Hz())
            .resolution(Resolution::Bits14),
    )?;

    let barriers = crate::servo_barrier::BarrierPair::new(
        ServoBarrier::new(
            LedcDriver::new(
                peripherals.ledc.channel0,
                &timer_driver,
                peripherals.pins.gpio5,
            )?,
            crate::servo_barrier::BarrierCalibration::new(820, 1400),
        ),
        ServoBarrier::new(
            LedcDriver::new(
                peripherals.ledc.channel1,
                &timer_driver,
                peripherals.pins.gpio10,
            )?,
            crate::servo_barrier::BarrierCalibration::new(500, 1300),
        ),
    );

    let hx711 = crate::sensors::Hx711::new(
        PinDriver::input(peripherals.pins.gpio41.downgrade_input())?,
        PinDriver::output(peripherals.pins.gpio42.downgrade_output())?,
    )?;

    #[cfg(feature = "hardware_tests")]
    {
        // Special mode for testing individual hardware components.
        crate::hardware_test::run_hardware_tests(crate::hardware_test::HardwareDevices {
            stepper,
            barriers,
            sensor1: distance_sensor1,
            sensor2: distance_sensor2,
            reed,
            obstacle,
            encoder,
            lights,
            buzzer,
            hx711,
        })
        .await?;
        Ok(())
    }

    #[cfg(not(feature = "hardware_tests"))]
    {
        #[cfg(feature = "hil")]
        {
            // Hardware-in-the-Loop (HIL) tests for automated verification.
            crate::hil_tests::run_hil_tests(crate::hil_tests::HilDevices {
                stepper,
                barriers,
                ds1: distance_sensor1,
                ds2: distance_sensor2,
                reed,
                obstacle,
                encoder,
                lights,
                buzzer,
                hx711,
            })
            .await?;
            Ok(())
        }

        #[cfg(not(feature = "hil"))]
        {
            // --- SHARED STATE ---
            // Contains the state of all sensors and the current bridge FSM state,
            // protected by Mutexes for safe access across tasks.
            let shared = SharedState {
                sensors: Mutex::new(SharedSensors::default()),
                bridge_state: Mutex::new(BridgeState::Startup),
            };

            info!("Bridge initialized. Starting tasks...");

            // --- LAUNCH TASKS ---
            // Run all bridge control tasks concurrently.
            let (res_s, res_m, res_l, res_b, res_c) = join!(
                sensor_task(
                    reed,
                    obstacle,
                    distance_sensor1,
                    distance_sensor2,
                    hx711,
                    &shared
                ),
                motion_task(stepper, barriers, encoder, &shared),
                light_task(lights, &shared),
                buzzer_task(buzzer, &shared),
                coordinator_task(&shared),
            );

            // If any task fails, report it
            res_s?;
            res_m?;
            res_l?;
            res_b?;
            res_c?;

            Ok(())
        }
    }
}
