#include "BridgeMotion.h"

namespace bridge {

// 28BYJ-48 half-step sequence -- 8 phases, lower nibble only (GPA0-GPA3 = IN1-IN4).
// Translated from HALF_STEP_SEQ in stepper.rs. Upper nibble kept 0 here;
// _applyStepperPattern() ORs in the preserved upper bits from the MCP23017 latch.
const uint8_t BridgeMotion::HALF_STEP_SEQ[8] = {
    0b00000001,  // IN1
    0b00000011,  // IN1+IN2
    0b00000010,  // IN2
    0b00000110,  // IN2+IN3
    0b00000100,  // IN3
    0b00001100,  // IN3+IN4
    0b00001000,  // IN4
    0b00001001,  // IN4+IN1
};

BridgeMotion::BridgeMotion(MCP23017& mcp, PCA9685& pca)
    : _mcp(mcp), _pca(pca) {}

bool BridgeMotion::begin() {
    _stopStepper();
    _pca.setFullOff(PCA_CH_SERVO_LEFT);
    _pca.setFullOff(PCA_CH_SERVO_RIGHT);
    _barrierPos   = BarrierPosition::Unknown;
    _barrierPhase = BarrierPhase::Idle;
    return true;
}

void BridgeMotion::update() {
    unsigned long now = millis();

    switch (_barrierPhase) {
        case BarrierPhase::Idle:
            break;

        case BarrierPhase::MovingLeft:
            if (now - _barrierTimer >= BARRIER_MOVE_MS) {
                // Detach left servo by disabling PCA9685 channel (reduces idle draw)
                _pca.setFullOff(PCA_CH_SERVO_LEFT);
                _barrierTimer = now;
                _barrierPhase = BarrierPhase::WaitingBetween;
            }
            break;

        case BarrierPhase::WaitingBetween:
            if (now - _barrierTimer >= BARRIER_STAGGER_MS) {
                uint8_t angle = _barrierTarget ? R_SERVO_OPEN : R_SERVO_CLOSED;
                _pca.setServoAngle(PCA_CH_SERVO_RIGHT, angle);
                _barrierTimer = now;
                _barrierPhase = BarrierPhase::MovingRight;
            }
            break;

        case BarrierPhase::MovingRight:
            if (now - _barrierTimer >= BARRIER_MOVE_MS) {
                _pca.setFullOff(PCA_CH_SERVO_RIGHT);
                _barrierPos   = _barrierTarget ? BarrierPosition::Open
                                               : BarrierPosition::Closed;
                _barrierPhase = BarrierPhase::Idle;
            }
            break;
    }

    // Stepper is blocked while barriers are moving (5V power budget guard)
    if (_barrierPhase != BarrierPhase::Idle) return;
    if (_stepperRemaining <= 0) return;

    if (now - _stepperLastMs >= STEPPER_STEP_MS) {
        _stepperLastMs = now;

        if (_stepperDir > 0) _stepperIdx = (_stepperIdx + 1) % 8;
        else                 _stepperIdx = (_stepperIdx + 7) % 8;

        _applyStepperPattern(_stepperIdx);
        _stepperRemaining--;

        if (_stepperRemaining == 0) _stopStepper();
    }
}

void BridgeMotion::moveBy(int steps) {
    if (steps == 0) return;
    _stepperDir       = (steps > 0) ? 1 : -1;
    _stepperRemaining = (steps > 0) ? steps : -steps;
    _stepperLastMs    = millis();
}

void BridgeMotion::setBarriers(bool open) {
    if (_barrierPhase != BarrierPhase::Idle) return;

    BarrierPosition target = open ? BarrierPosition::Open : BarrierPosition::Closed;
    if (_barrierPos == target) return;

    _barrierTarget = open;
    uint8_t angle  = open ? L_SERVO_OPEN : L_SERVO_CLOSED;
    _pca.setServoAngle(PCA_CH_SERVO_LEFT, angle);
    _barrierTimer = millis();
    _barrierPhase = BarrierPhase::MovingLeft;
}

void BridgeMotion::stopAll() {
    _stepperRemaining = 0;
    _stopStepper();
    _pca.setFullOff(PCA_CH_SERVO_LEFT);
    _pca.setFullOff(PCA_CH_SERVO_RIGHT);
    _barrierPhase = BarrierPhase::Idle;
}

void BridgeMotion::setLeftServoAngle(uint8_t degrees) {
    _pca.setServoAngle(PCA_CH_SERVO_LEFT, degrees);
}

void BridgeMotion::setRightServoAngle(uint8_t degrees) {
    _pca.setServoAngle(PCA_CH_SERVO_RIGHT, degrees);
}

void BridgeMotion::detachLeftServo() {
    _pca.setFullOff(PCA_CH_SERVO_LEFT);
}

void BridgeMotion::detachRightServo() {
    _pca.setFullOff(PCA_CH_SERVO_RIGHT);
}

// Write the 4-bit stepper coil pattern to MCP23017 Port A (GPA0-GPA3).
// The upper nibble (HX711 + shift register bits) is preserved from the latch.
void BridgeMotion::_applyStepperPattern(uint8_t idx) {
    uint8_t portA = (_mcp.latchA() & 0xF0) | HALF_STEP_SEQ[idx];
    _mcp.writePortA(portA);
}

// De-energise all stepper coils by clearing GPA0-GPA3.
// Mirrors StepperTrait::stop() in stepper.rs.
void BridgeMotion::_stopStepper() {
    uint8_t portA = _mcp.latchA() & 0xF0;  // preserve upper nibble
    _mcp.writePortA(portA);
}

} // namespace bridge
