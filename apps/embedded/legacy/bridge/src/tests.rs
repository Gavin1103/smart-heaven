#![allow(dead_code)]
#![allow(unused_imports)]
use crate::config::HOMING_EXTRA_STEPS;
use crate::config::BOAT_DETECTION_CONFIRMATION;
use crate::config::WARNING_DURATION;
use crate::config::BARRIER_MOVE_DURATION;
use crate::config::WATER_PREP_DURATION;
use crate::mock::MockDistanceSensor;
use crate::mock::MockReedSensor;
use crate::state_machine::{BridgeState, SharedSensors, FaultReason};
use crate::traits::ReedSensorTrait;
use embassy_time::Instant;

#[test]
fn test_startup_logic() {
    let state = BridgeState::Startup;
    let mut sensors = SharedSensors::default();
    let mut trigger_pos = None;
    let now = Instant::now();

    // Safe startup
    let next = state.next(&sensors, &mut trigger_pos, now);
    match next {
        BridgeState::Homing(_) => (),
        _ => panic!("Expected Homing, got {:?}", next),
    }

    // Unsafe startup (both boat and obstacle)
    sensors.boat_detected = true;
    sensors.obstacle_detected = true;
    let next = state.next(&sensors, &mut trigger_pos, now);
    match next {
        BridgeState::Fault(FaultReason::InvalidStartupState) => (),
        _ => panic!("Expected Fault, got {:?}", next),
    }
}

#[test]
fn test_homing_sequence() {
    let now = Instant::now();
    let mut state = BridgeState::Homing(now);
    let mut sensors = SharedSensors::default();
    let mut trigger_pos = None;

    // Still homing if reed not closed
    state = state.next(&sensors, &mut trigger_pos, now);
    match state {
        BridgeState::Homing(_) => (),
        _ => panic!("Expected Homing, got {:?}", state),
    }

    // Reed closed, should set trigger_pos
    sensors.reed_closed = true;
    sensors.stepper_pos = 1000;
    state = state.next(&sensors, &mut trigger_pos, now);
    assert_eq!(trigger_pos, Some(1000));

    // Not seated yet
    state = state.next(&sensors, &mut trigger_pos, now);
    match state {
        BridgeState::Homing(_) => (),
        _ => panic!("Expected Homing, got {:?}", state),
    }

    // Seated! (stepper moved HOMING_EXTRA_STEPS more)
    sensors.stepper_pos = 1000 - HOMING_EXTRA_STEPS;
    state = state.next(&sensors, &mut trigger_pos, now);
    assert_eq!(state, BridgeState::IdleClosed);
}

#[test]
fn test_boat_to_opening_flow() {
    let now = Instant::now();
    let mut state = BridgeState::IdleClosed;
    let mut sensors = SharedSensors {
        reed_closed: true,
        ..Default::default()
    };
    let mut trigger_pos = None;

    // Boat detected
    sensors.boat_detected = true;
    state = state.next(&sensors, &mut trigger_pos, now);
    match state {
        BridgeState::BoatDetected(_) => (),
        _ => panic!("Expected BoatDetected, got {:?}", state),
    }

    // Wait for confirmation
    let future_now = now + BOAT_DETECTION_CONFIRMATION;
    state = state.next(&sensors, &mut trigger_pos, future_now);
    match state {
        BridgeState::WarningRoadTraffic(_) => (),
        _ => panic!("Expected WarningRoadTraffic, got {:?}", state),
    }

    // Wait for warning duration
    let future_now = future_now + WARNING_DURATION;
    state = state.next(&sensors, &mut trigger_pos, future_now);
    match state {
        BridgeState::LowerBarriers(_) => (),
        _ => panic!("Expected LowerBarriers, got {:?}", state),
    }

    // Wait for barrier move
    let future_now = future_now + BARRIER_MOVE_DURATION;
    state = state.next(&sensors, &mut trigger_pos, future_now);
    match state {
        BridgeState::WaitBridgeClear(_) => (),
        _ => panic!("Expected WaitBridgeClear, got {:?}", state),
    }

    // Bridge clear of cars
    sensors.car_detected = false;
    state = state.next(&sensors, &mut trigger_pos, future_now);
    match state {
        BridgeState::WaterPrepRedGreen(_) => (),
        _ => panic!("Expected WaterPrepRedGreen, got {:?}", state),
    }

    // Open bridge
    let future_now = future_now + WATER_PREP_DURATION;
    state = state.next(&sensors, &mut trigger_pos, future_now);
    match state {
        BridgeState::Opening(_) => (),
        _ => panic!("Expected Opening, got {:?}", state),
    }
}

#[test]
fn test_obstacle_reopens_while_closing() {
    let now = Instant::now();
    let mut state = BridgeState::Closing(now);
    let mut sensors = SharedSensors::default();
    let mut trigger_pos = None;

    sensors.obstacle_detected = true;
    state = state.next(&sensors, &mut trigger_pos, now);
    match state {
        BridgeState::Opening(_) => (),
        _ => panic!("Expected Opening, got {:?}", state),
    }
}

#[test]
fn test_mock_sensors() {
    // This tests that our mocks work correctly
    let _mock_ds = MockDistanceSensor { value: Some(500) };
    let mock_reed = MockReedSensor { closed: true };

    assert!(mock_reed.is_closed());
}
