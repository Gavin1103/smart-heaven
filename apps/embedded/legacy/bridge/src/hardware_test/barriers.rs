use crate::encoder::Encoder;
use crate::hardware_test::wait_for_user;
use crate::servo_barrier::BarrierPair;
use crate::traits::BarrierPairTrait;
use anyhow::Result;
use embassy_time::Timer;

pub async fn test_barriers(barriers: &mut BarrierPair<'_>, encoder: &mut Encoder<'_>) -> Result<()> {
    wait_for_user(encoder, "TEST 3: LEFT Servo (Will open)").await?;
    barriers.open_left()?;
    wait_for_user(encoder, "TEST 3: LEFT Servo (Will close)").await?;
    Timer::after_secs(1).await;
    barriers.close_left()?;
    wait_for_user(encoder, "TEST 3: LEFT Servo (Will turn off.)").await?;
    Timer::after_secs(1).await;
    barriers.stop_left()?; // Isolate power

    wait_for_user(encoder, "TEST 4: RIGHT Servo (Will open)").await?;
    barriers.open_right()?;
    wait_for_user(encoder, "TEST 4: RIGHT Servo (Will close)").await?;
    Timer::after_secs(1).await;
    barriers.close_right()?;
    wait_for_user(encoder, "TEST 4: RIGHT Servo (Will turn off.)").await?;
    Timer::after_secs(1).await;
    barriers.stop_right()?; // Isolate power
    Ok(())
}
