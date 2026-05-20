#include "BridgeController.h"

namespace bridge {

BridgeController::BridgeController(I2CManager& bus, uint8_t address,
                                   telemetry::TelemetryManager* telemetry)
    : _bus(bus),
      _address(address),
      _mcp(bus, _mcpAddr(address)),   // must precede subsystems in decl order
      _pca(bus, _pcaAddr(address)),   // must precede subsystems in decl order
      _signaling(_mcp, _pca),
      _motion(_mcp, _pca),
      _sensors(_mcp, _pca),
      _telemetry(telemetry) {}

bool BridgeController::begin() {
    Serial.printf("[Bridge] Node 0x%02X -- MCP=0x%02X PCA=0x%02X\n",
                  _address, _mcpAddr(_address), _pcaAddr(_address));

    if (!_mcp.begin(MCP_A_IODIR, MCP_B_IODIR, MCP_A_PULLUP, MCP_B_PULLUP)) {
        Serial.println("[Bridge] ERROR: MCP23017 not found -- check I2C wiring and address jumpers");
        return false;
    }
    if (!_pca.begin(50.0f)) {
        Serial.println("[Bridge] ERROR: PCA9685PW not found -- check I2C wiring and address jumpers");
        return false;
    }

    _signaling.begin();
    _motion.begin();
    _sensors.begin();
    transitionTo(State::Startup);
    return true;
}

// -----------------------------------------------------------------------------
// stateToString — human-readable label for Serial logs
// -----------------------------------------------------------------------------
const char* BridgeController::stateToString(State s) {
    switch (s) {
        case State::Startup:               return "Startup";
        case State::Homing:                return "Homing";
        case State::IdleClosed:            return "IdleClosed";
        case State::BoatDetected:          return "BoatDetected";
        case State::WarningRoadTraffic:    return "WarningRoadTraffic";
        case State::WaitBridgeClear:       return "WaitBridgeClear";
        case State::LowerBarriers:         return "LowerBarriers";
        case State::WaterPrepRedGreen:     return "WaterPrepRedGreen";
        case State::Opening:               return "Opening";
        case State::Open:                  return "Open";
        case State::ClosingCheckClearance: return "ClosingCheckClearance";
        case State::Closing:               return "Closing";
        case State::RaiseBarriers:         return "RaiseBarriers";
        case State::Fault:                 return "Fault";
        default:                           return "Unknown";
    }
}

// -----------------------------------------------------------------------------
// faultReasonToString — human-readable fault label for logs and telemetry
// -----------------------------------------------------------------------------
const char* BridgeController::faultReasonToString(FaultReason reason) {
    switch (reason) {
        case FaultReason::HomingTimeout:       return "HomingTimeout";
        case FaultReason::OpeningTimeout:      return "OpeningTimeout";
        case FaultReason::ClosingTimeout:      return "ClosingTimeout";
        case FaultReason::InvalidStartupState: return "InvalidStartupState";
        case FaultReason::SafetyCheckFailed:   return "SafetyCheckFailed";
        case FaultReason::MotorStall:          return "MotorStall";
        default:                               return "Unknown";
    }
}

// -----------------------------------------------------------------------------
// toEventCode — maps (from, to) state pairs to the most specific EventCode so
// the backend can filter events without parsing state-name strings.
// -----------------------------------------------------------------------------
telemetry::EventCode BridgeController::toEventCode(State from, State to) {
    if (to   == State::Fault)                                          return telemetry::EventCode::FaultEntered;
    if (from == State::Fault)                                          return telemetry::EventCode::FaultRecovered;
    if (from == State::IdleClosed  && to == State::BoatDetected)       return telemetry::EventCode::BoatDetected;
    if (from == State::BoatDetected && to == State::IdleClosed)        return telemetry::EventCode::BoatCleared;
    if (to   == State::Open)                                           return telemetry::EventCode::BridgeOpened;
    if (from == State::Closing     && to == State::RaiseBarriers)      return telemetry::EventCode::BridgeClosed;
    if (from == State::Closing     && to == State::Opening)            return telemetry::EventCode::ClosingAborted;
    if (from == State::Homing      && to == State::IdleClosed)         return telemetry::EventCode::HomingComplete;
    if (from == State::Startup     && to == State::Homing)             return telemetry::EventCode::StartupValidated;
    return telemetry::EventCode::StateTransition;
}

// -----------------------------------------------------------------------------
// transitionTo — records entry time so every state can use millis()-based timing
// -----------------------------------------------------------------------------
void BridgeController::transitionTo(State newState) {
    Serial.printf("[Bridge] %s -> %s  (t=%lu ms)\n",
                  stateToString(_currentState),
                  stateToString(newState),
                  millis());

    // Enqueue telemetry before updating _currentState so fromState is correct.
    if (_telemetry) {
        telemetry::TelemetryPayload payload{};
        payload.eventCode   = toEventCode(_currentState, newState);
        payload.fromState   = stateToString(_currentState);
        payload.toState     = stateToString(newState);
        payload.faultReason = (newState == State::Fault)
                                  ? faultReasonToString(_faultReason)
                                  : nullptr;
        payload.sensors     = {_boatDetected, _obstacleDetected,
                                _carDetected,  _reedClosed, _stepperPos};
        payload.uptimeMs    = millis();
        _telemetry->enqueue(payload);
    }

    _currentState = newState;
    _stateTimer   = millis();

    // Tare the load cell every time we reach IdleClosed so the weight
    // baseline is always fresh (bridge deck empty, bridge seated).
    if (newState == State::IdleClosed) {
        _sensors.tareLoadCell();
    }
}

// -----------------------------------------------------------------------------
// update — called every loop tick; must NEVER block
// Translated from BridgeState::next() in legacy/bridge/src/state_machine.rs
// -----------------------------------------------------------------------------
void BridgeController::update() {
    // Drive all hardware subsystems first so sensor readings are current.
    _signaling.update();
    _motion.update();
    _sensors.update();

    // Refresh sensor cache once per tick.
    _boatDetected     = readBoatSensor();
    _obstacleDetected = readObstacleSensor();
    _reedClosed       = readReedSensor();
    _carDetected      = readCarDetected();
    _stepperPos       = readStepperPosition();
    _encoderSwPressed = readEncoderSwitch();

    unsigned long elapsed = millis() - _stateTimer;

    switch (_currentState) {

        // ------------------------------------------------------------------ //
        // Startup — validate that sensors aren't in a contradictory state     //
        // ------------------------------------------------------------------ //
        case State::Startup:
            if (_boatDetected && _obstacleDetected) {
                // Multiple sensors blocked simultaneously is physically impossible
                _faultReason = FaultReason::InvalidStartupState;
                transitionTo(State::Fault);
            } else {
                _reedTriggerPosSet = false;
                transitionTo(State::Homing);
                startHomingMotor();
            }
            break;

        // ------------------------------------------------------------------ //
        // Homing — drive toward closed position until reed triggers, then     //
        // seat the bridge HOMING_EXTRA_STEPS further to ensure firm contact   //
        // ------------------------------------------------------------------ //
        case State::Homing:
            if (_reedClosed) {
                if (!_reedTriggerPosSet) {
                    _reedTriggerPos    = _stepperPos;
                    _reedTriggerPosSet = true;
                    Serial.printf("[Homing] Reed triggered at pos=%d — seating %d extra steps\n",
                                  _reedTriggerPos, HOMING_EXTRA_STEPS);
                }
                // Motor moves in the POSITIVE direction, so position increases.
                // Wait until we have seated HOMING_EXTRA_STEPS beyond the trigger point.
                int target = _reedTriggerPos + HOMING_EXTRA_STEPS;
                Serial.printf("[Homing] pos=%d  target=%d  elapsed=%lu ms\n",
                              _stepperPos, target, elapsed);
                if (_stepperPos >= target) {
                    Serial.printf("[Homing] Seated at pos=%d — homing complete\n", _stepperPos);
                    stopMotor();
                    _reedTriggerPosSet = false;
                    transitionTo(State::IdleClosed);
                    setTrafficLightsIdle();
                }
            } else {
                // Reed not yet triggered — reset sub-state and check timeout
                _reedTriggerPosSet = false;
                Serial.printf("[Homing] Searching... pos=%d  elapsed=%lu ms\n",
                              _stepperPos, elapsed);
                if (elapsed >= HOMING_TIMEOUT_MS) {
                    Serial.printf("[Homing] TIMEOUT after %lu ms — entering Fault\n", elapsed);
                    stopMotor();
                    _faultReason = FaultReason::HomingTimeout;
                    transitionTo(State::Fault);
                }
            }
            break;

        // ------------------------------------------------------------------ //
        // IdleClosed — bridge seated, waiting for a boat                      //
        // ------------------------------------------------------------------ //
        case State::IdleClosed:
            if (_boatDetected) {
                transitionTo(State::BoatDetected);
            } else if (!_reedClosed) {
                // Reed opened unexpectedly (e.g. vibration) — re-home
                _reedTriggerPosSet = false;
                transitionTo(State::Homing);
                startHomingMotor();
            }
            break;

        // ------------------------------------------------------------------ //
        // BoatDetected — debounce: boat must stay detected for BOAT_CONFIRM_MS//
        // ------------------------------------------------------------------ //
        case State::BoatDetected:
            if (!_boatDetected) {
                // Boat disappeared before confirmation window — abort
                transitionTo(State::IdleClosed);
            } else if (elapsed >= BOAT_CONFIRM_MS) {
                transitionTo(State::WarningRoadTraffic);
                setTrafficLightsWarning(true);
                setBuzzer(true);
            }
            break;

        // ------------------------------------------------------------------ //
        // WarningRoadTraffic — flash lights to alert road users               //
        // ------------------------------------------------------------------ //
        case State::WarningRoadTraffic:
            // Flash traffic lights at 500 ms intervals each tick.
            setTrafficLightsWarning((elapsed / 500UL) % 2 == 0);
            if (elapsed >= WARNING_DURATION_MS) {
                setBuzzer(false);
                setTrafficLightsStop();
                transitionTo(State::WaitBridgeClear);
            }
            break;

        // ------------------------------------------------------------------ //
        // WaitBridgeClear — ensure no car is on the bridge before lowering    //
        // barriers. Timeout after 30 s → Fault.                              //
        // ------------------------------------------------------------------ //
        case State::WaitBridgeClear:
            if (!_carDetected) {
                transitionTo(State::LowerBarriers);
                lowerBarriers();
            } else if (elapsed >= WAIT_CLEAR_MS) {
                _faultReason = FaultReason::SafetyCheckFailed;
                transitionTo(State::Fault);
            }
            break;

        // ------------------------------------------------------------------ //
        // LowerBarriers — wait for BridgeMotion to complete the sequence      //
        // ------------------------------------------------------------------ //
        case State::LowerBarriers:
            if (!_motion.isBarrierMoving()) {
                transitionTo(State::WaterPrepRedGreen);
                setWaterLightsRedGreen();
            }
            break;

        // ------------------------------------------------------------------ //
        // WaterPrepRedGreen — signal water traffic to prepare (red+green)     //
        // ------------------------------------------------------------------ //
        case State::WaterPrepRedGreen:
            if (elapsed >= WATER_PREP_MS) {
                transitionTo(State::Opening);
                startOpeningMotor();
            }
            break;

        // ------------------------------------------------------------------ //
        // Opening — motor drives bridge to open position                       //
        // Fault if timeout or if reed still closed after full rotation         //
        // (motor stall / overload)                                            //
        // ------------------------------------------------------------------ //
        case State::Opening:
            if (elapsed >= BRIDGE_TIMEOUT_MS) {
                stopMotor();
                _faultReason = FaultReason::OpeningTimeout;
                transitionTo(State::Fault);
            } else if (_stepperPos >= FULL_ROTATION) {
                if (_reedClosed) {
                    // Full rotation completed but bridge never left closed pos
                    stopMotor();
                    _faultReason = FaultReason::MotorStall;
                    transitionTo(State::Fault);
                } else {
                    stopMotor();
                    transitionTo(State::Open);
                    setWaterLightsGreen();
                }
            }
            break;

        // ------------------------------------------------------------------ //
        // Open — bridge is fully open; wait for waterway to clear AND for the //
        // minimum hold time before initiating closing sequence                //
        // ------------------------------------------------------------------ //
        case State::Open:
            if (elapsed >= BRIDGE_WAIT_MS && !_boatDetected && !_obstacleDetected) {
                transitionTo(State::ClosingCheckClearance);
            }
            break;

        // ------------------------------------------------------------------ //
        // ClosingCheckClearance — final safety window before closing           //
        // ------------------------------------------------------------------ //
        case State::ClosingCheckClearance:
            if (_boatDetected || _obstacleDetected) {
                // Something entered the waterway — go back to full open wait
                transitionTo(State::Open);
            } else if (elapsed >= CLEARANCE_CHECK_MS) {
                transitionTo(State::Closing);
                startClosingMotor();
            }
            break;

        // ------------------------------------------------------------------ //
        // Closing — motor drives bridge back to closed position                //
        // Abort and re-open if IR blocked during close                        //
        // ------------------------------------------------------------------ //
        case State::Closing:
            if (_obstacleDetected) {
                // Boat/obstacle crossed while closing — reverse
                stopMotor();
                transitionTo(State::Opening);
                startOpeningMotor();
            } else if (elapsed >= BRIDGE_TIMEOUT_MS) {
                stopMotor();
                _faultReason = FaultReason::ClosingTimeout;
                transitionTo(State::Fault);
            } else if (_reedClosed) {
                stopMotor();
                transitionTo(State::RaiseBarriers);
                raiseBarriers();
            }
            break;

        // ------------------------------------------------------------------ //
        // RaiseBarriers — wait for BridgeMotion to complete the sequence      //
        // ------------------------------------------------------------------ //
        case State::RaiseBarriers:
            if (!_motion.isBarrierMoving()) {
                transitionTo(State::IdleClosed);
                setTrafficLightsIdle();
            }
            break;

        // ------------------------------------------------------------------ //
        // Fault — halted; press encoder button to reset and re-home            //
        // ------------------------------------------------------------------ //
        case State::Fault:
            if (_encoderSwPressed) {
                _reedTriggerPosSet = false;
                transitionTo(State::Homing);
                startHomingMotor();
            }
            break;
    }
}

// =============================================================================
// SENSOR READS
// Delegate to BridgeSensors; cached into _boatDetected etc. at top of update().
// =============================================================================

bool BridgeController::readBoatSensor() {
    // Boat detected from either approach direction.
    return _sensors.isBoatDetectedEast() || _sensors.isBoatDetectedWest();
}

bool BridgeController::readObstacleSensor() {
    return _sensors.isObstacleBlocking();
}

bool BridgeController::readReedSensor() {
    return _sensors.isReedClosed();
}

bool BridgeController::readCarDetected() {
    return _sensors.isWeightDetected();
}

int BridgeController::readStepperPosition() {
    // Encoder position is reset at the start of each homing/opening move,
    // providing a relative step count for the current motion segment.
    return static_cast<int>(_sensors.getEncoderPosition());
}

bool BridgeController::readEncoderSwitch() {
    return _sensors.isEncoderPressed();
}

// =============================================================================
// ACTUATORS — MOTOR
// =============================================================================

void BridgeController::startHomingMotor() {
    // Safety interlock: do not start stepper while barriers are in motion
    // (power budget constraint, see docs/features/bridge/power_analysis.md).
    if (_motion.isBarrierMoving()) return;
    _motion.stopAll();                                      // cancel any in-flight move
    _sensors.resetEncoderPosition();                        // fresh position baseline
    // Queue a forward move large enough to reach home from any position.
    // The Homing state calls stopMotor() once reed + HOMING_EXTRA_STEPS are satisfied.
    _motion.moveBy(-(FULL_ROTATION + HOMING_EXTRA_STEPS + 500));
}

void BridgeController::startOpeningMotor() {
    if (_motion.isBarrierMoving()) return;
    _motion.stopAll();                    // cancel any in-flight move (e.g. Closing reversal)
    _sensors.resetEncoderPosition();      // count steps from zero so Opening can check >= FULL_ROTATION
    _motion.moveBy(FULL_ROTATION + 500);  // slightly over-queue; stopped at FULL_ROTATION by state machine
}

void BridgeController::startClosingMotor() {
    if (_motion.isBarrierMoving()) return;
    _motion.stopAll();
    // Closing does not rely on encoder position; it stops on reed trigger.
    _motion.moveBy(-(FULL_ROTATION + 500));
}

void BridgeController::stopMotor() {
    _motion.stopAll();
}

// =============================================================================
// ACTUATORS — BARRIERS
// =============================================================================

void BridgeController::lowerBarriers() {
    // false = closed/lowered (road blocked for boat passage).
    // BridgeMotion handles the staggered Sequential Activation Guard internally.
    _motion.setBarriers(false);
}

void BridgeController::raiseBarriers() {
    // true = open/raised (road re-opened after bridge closes).
    _motion.setBarriers(true);
}

void BridgeController::stopBarriers() {
    // BridgeMotion manages its own barrier completion sequence.
    // LowerBarriers / RaiseBarriers states now check isBarrierMoving() directly,
    // so this explicit stop is no longer needed and is intentionally a no-op.
}

// =============================================================================
// ACTUATORS — SIGNALING
// =============================================================================

void BridgeController::setTrafficLightsIdle() {
    // Bridge is closed and seated — road traffic may proceed.
    _signaling.setRoadTraffic(bridge::Color::Green);
}

void BridgeController::setTrafficLightsWarning(bool flashOn) {
    // flashOn toggles Yellow on/off to create the flashing warning pattern.
    _signaling.setRoadTraffic(flashOn ? bridge::Color::Yellow : bridge::Color::Off);
}

void BridgeController::setTrafficLightsStop() {
    // Bridge sequence in progress — road traffic must stop.
    _signaling.setRoadTraffic(bridge::Color::Red);
}

void BridgeController::setWaterLightsRedGreen() {
    // Preparation signal: bridge is about to open (Red = wait, approach slowly).
    // Full Red+Green simultaneous output requires a future BridgeSignaling API
    // extension (two independent water-light channels).
    _signaling.setWaterTraffic(bridge::Color::Red);
}

void BridgeController::setWaterLightsGreen() {
    // Bridge is fully open — waterway is clear to proceed.
    _signaling.setWaterTraffic(bridge::Color::Green);
}

void BridgeController::setBuzzer(bool on) {
    // Use the non-blocking alarm pattern from BridgeSignaling rather than
    // a raw on/off, so update() can drive the 500 ms toggle automatically.
    if (on) {
        _signaling.triggerAlarm();
    } else {
        _signaling.stopAlarm();
    }
}

}