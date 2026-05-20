use crate::encoder::Encoder;
use crate::hardware_test::wait_for_user;
use crate::lights::{TrafficLights, ROAD_GREEN, ROAD_RED, ROAD_WARNING, BOAT_GREEN, BOAT_RED};
use crate::traits::TrafficLightsTrait;
use anyhow::Result;
use embassy_time::Timer;

pub async fn test_lights(lights: &mut TrafficLights<'_>, encoder: &mut Encoder<'_>) -> Result<()> {
    wait_for_user(encoder, "TEST 2: Traffic Lights (Will cycle through all states)").await?;
    lights.set_state(ROAD_RED)?; Timer::after_secs(1).await;
    lights.set_state(ROAD_GREEN)?; Timer::after_secs(1).await;
    lights.set_state(ROAD_WARNING)?; Timer::after_secs(1).await;
    lights.set_state(BOAT_RED)?; Timer::after_secs(1).await;
    lights.set_state(BOAT_GREEN)?; Timer::after_secs(1).await;
    lights.set_state(0)?; // Turn off
    Ok(())
}
