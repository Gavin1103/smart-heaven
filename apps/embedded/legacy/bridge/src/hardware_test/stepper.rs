use crate::config::STEP_SEQUENCE;
use crate::encoder::Encoder;
use crate::hardware_test::wait_for_user;
use crate::servo_barrier::BarrierPair;
use crate::stepper::StepperPins;
use crate::traits::{BarrierPairTrait, StepperTrait};
use anyhow::Result;
use embassy_time::Timer;
use log::info;

pub async fn test_stepper(
    stepper: &mut StepperPins<'_>,
    barriers: &mut BarrierPair<'_>,
    encoder: &mut Encoder<'_>,
) -> Result<()> {
    wait_for_user(encoder, "TEST 5: Stepper Motor (Will open, then close)").await?;
    barriers.stop()?; // Double check servos are off

    let loops = 800; // Tune this value for the desired opening range

    info!("  Opening bridge...");
    for _ in 0..loops {
        for step in STEP_SEQUENCE.iter().rev() {
            stepper.apply_step(step)?;
            Timer::after_millis(5).await;
        }
    }

    Timer::after_secs(1).await;

    info!("  Closing bridge...");
    for _ in 0..loops {
        for step in STEP_SEQUENCE.iter() {
            stepper.apply_step(step)?;
            Timer::after_millis(5).await;
        }
    }

    stepper.stop()?; // Isolate power
    Ok(())
}
