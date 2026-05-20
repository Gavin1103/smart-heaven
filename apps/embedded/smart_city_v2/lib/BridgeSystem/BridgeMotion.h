#ifndef BRIDGE_MOTION_H
#define BRIDGE_MOTION_H

#include <Arduino.h>
#include "city_config.h"
#include "MCP23017.h"
#include "PCA9685.h"

namespace bridge {

enum class BarrierPosition : uint8_t { Open, Closed, Unknown };

/// Drives the 28BYJ-48 stepper (via MCP23017 GPA0-3 -> ULN2003) and the two
/// SG90 servo barriers (via PCA9685 CH0 / CH1).
///
/// Power constraints (power_analysis.md):
///   - 5V rail: 500 mA budget; both servos stalling = ~1200 mA.
///   - Servos activate one at a time with a BARRIER_STAGGER_MS gap.
///   - Stepper is paused while barriers are in motion.
///   - Servo channels are set to full-off after motion completes (reduces idle draw).
///
/// Stepper steps are written as a 4-bit pattern to MCP23017 Port A (GPA0-3),
/// preserving the upper nibble (HX711 + shift register signals).
/// At 3 ms/step and ~200 us I2C overhead per step, the effective step rate is
/// ~290 steps/sec vs the original 333 steps/sec. This is within spec for 28BYJ-48.
///
/// Call begin() once in setup(). Call update() every loop() iteration.
class BridgeMotion {
public:
    BridgeMotion(MCP23017& mcp, PCA9685& pca);

    bool begin();
    void update();

    void moveBy(int steps);
    void setBarriers(bool open);
    void stopAll();

    bool isStepperMoving() const { return _stepperRemaining > 0; }
    bool isBarrierMoving()  const { return _barrierPhase != BarrierPhase::Idle; }

    // Calibration constants -- same values as before; exposed for hwtest
    static constexpr uint8_t L_SERVO_OPEN   = 180;
    static constexpr uint8_t L_SERVO_CLOSED = 90;
    static constexpr uint8_t R_SERVO_OPEN   = 90;
    static constexpr uint8_t R_SERVO_CLOSED = 180;

    static constexpr unsigned long BARRIER_MOVE_MS    = 1200UL;
    static constexpr unsigned long BARRIER_STAGGER_MS = 200UL;

    // Test-only: set servo angle directly on PCA9685 (bypasses FSM)
    void setLeftServoAngle(uint8_t degrees);
    void setRightServoAngle(uint8_t degrees);
    void detachLeftServo();
    void detachRightServo();

private:
    MCP23017& _mcp;
    PCA9685&  _pca;

    static constexpr unsigned long STEPPER_STEP_MS = 3UL;

    // 28BYJ-48 half-step sequence (8 phases, GPA0=IN1 ... GPA3=IN4)
    static const uint8_t HALF_STEP_SEQ[8];

    enum class BarrierPhase : uint8_t {
        Idle,
        MovingLeft,
        WaitingBetween,
        MovingRight,
    };

    BarrierPhase    _barrierPhase  = BarrierPhase::Idle;
    BarrierPosition _barrierPos    = BarrierPosition::Unknown;
    bool            _barrierTarget = false;
    unsigned long   _barrierTimer  = 0;

    int           _stepperRemaining = 0;
    int8_t        _stepperDir       = 1;
    uint8_t       _stepperIdx       = 0;
    unsigned long _stepperLastMs    = 0;

    void _applyStepperPattern(uint8_t idx);
    void _stopStepper();
};

} // namespace bridge

#endif // BRIDGE_MOTION_H
