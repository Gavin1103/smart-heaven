use crate::buzzer::Buzzer;
use crate::encoder::Encoder;
use crate::hardware_test::wait_for_user;
use crate::traits::BuzzerTrait;
use anyhow::Result;
use embassy_time::Timer;

pub async fn test_buzzer(buzzer: &mut Buzzer<'_>, encoder: &mut Encoder<'_>) -> Result<()> {
    wait_for_user(encoder, "TEST 1: Buzzer (Will chirp for 500ms)").await?;
    buzzer.set_on(true)?;
    Timer::after_millis(500).await;
    buzzer.set_on(false)?;
    Ok(())
}
