// =============================================================================
// main_hwtest.cpp — Hardware Validation Sketch
// =============================================================================
// Interactive serial menu to verify every pin of the Bridge Node PCB before
// running production firmware. Uses the same BridgeMotion, BridgeSensors, and
// BridgeSignaling production drivers so results reflect real wiring.
//
// Upload : pio run -e hwtest -t upload
// Monitor: pio device monitor -e hwtest
//
// Commands (send via Serial Monitor):
//   s  — Stepper Test   : 500 steps fwd then 500 back; prints encoder live
//   b  — Barrier Test   : open barriers, wait 1 s, close; servos detach after
//   l  — Lights/Buzzer  : cycle Q0-Q7 one at a time, then beep buzzer
//   i  — Input Monitor  : print reed / IR / weight states for 5 seconds
//   v  — VarServo       : sweep LEFT then RIGHT 0->180 degrees
// =============================================================================

#include <Arduino.h>
#include "I2CManager.h"
#include "MCP23017.h"
#include "PCA9685.h"
#include "BridgeMotion.h"
#include "BridgeSensors.h"
#include "BridgeSignaling.h"
#include "city_config.h"

using namespace bridge;

// ---- PCB peripherals -------------------------------------------------------- //
static I2CManager    _i2c(I2C_SDA_PIN, I2C_SCL_PIN, 400000);
static MCP23017      _mcp(_i2c, ADDR_BRIDGE_MCP_BASE);
static PCA9685       _pca(_i2c, ADDR_BRIDGE_PCA_BASE);

// ---- Subsystem instances ---------------------------------------------------- //
static BridgeMotion    _motion(_mcp, _pca);
static BridgeSensors   _sensors(_mcp, _pca);
static BridgeSignaling _signaling(_mcp, _pca);

// ---- Top-level test selector ------------------------------------------------ //
enum class ActiveTest : uint8_t { None, Stepper, Barrier, Lights, InputMonitor, VarServo };
static ActiveTest _active = ActiveTest::None;

// ===========================================================================
// 's' — Stepper Test
// ===========================================================================
enum class StepperPhase : uint8_t { Forward, Back, Done };
static StepperPhase  _stepperPhase   = StepperPhase::Done;
static unsigned long _stepperPrintMs = 0;

static void startStepperTest() {
    _active       = ActiveTest::Stepper;
    _stepperPhase = StepperPhase::Forward;
    _sensors.resetEncoderPosition();
    Serial.println(F("\n[STEPPER] START — moving 500 steps FORWARD"));
    _motion.moveBy(500);
}

static void updateStepperTest() {
    if (_motion.isStepperMoving() && (millis() - _stepperPrintMs >= 200UL)) {
        _stepperPrintMs = millis();
        Serial.print(F("  enc="));
        Serial.println(_sensors.getEncoderPosition());
    }

    if (_stepperPhase == StepperPhase::Forward && !_motion.isStepperMoving()) {
        int32_t pos = _sensors.getEncoderPosition();
        Serial.print(F("[STEPPER] FORWARD done. Encoder="));
        Serial.println(pos);
        if (pos == 0) {
            Serial.println(F("  WARN: encoder still 0 — check MCP23017 GPB1/GPB2."));
        }
        _stepperPhase = StepperPhase::Back;
        Serial.println(F("[STEPPER] Moving 500 steps BACK..."));
        _motion.moveBy(-500);

    } else if (_stepperPhase == StepperPhase::Back && !_motion.isStepperMoving()) {
        int32_t pos = _sensors.getEncoderPosition();
        Serial.print(F("[STEPPER] BACK done. Final encoder="));
        Serial.println(pos);
        if (pos >= -5 && pos <= 5) {
            Serial.println(F("[STEPPER] PASS (net offset within +/-5 counts)."));
        } else {
            Serial.print(F("[STEPPER] NOTE: offset "));
            Serial.print(pos);
            Serial.println(F(" — stepper may have skipped steps."));
        }
        _stepperPhase = StepperPhase::Done;
        _active       = ActiveTest::None;
    }
}

// ===========================================================================
// 'b' — Barrier Test
// ===========================================================================
// Tests each servo in full isolation — open -> close -> detach — before
// touching the second servo (5V 500 mA rail limit: two stalled servos = ~1200 mA).
//
// Servos are driven through PCA9685 via BridgeMotion helper methods:
//   setLeftServoAngle() / detachLeftServo()
//   setRightServoAngle() / detachRightServo()
//
// Sequence:
//   LEFT  -> L_SERVO_OPEN -> BARRIER_MOVE_MS -> L_SERVO_CLOSED -> BARRIER_MOVE_MS -> detach
//   [BARRIER_STAGGER_MS settle]
//   RIGHT -> R_SERVO_OPEN -> BARRIER_MOVE_MS -> R_SERVO_CLOSED -> BARRIER_MOVE_MS -> detach
// ===========================================================================

enum class BarrierTestPhase : uint8_t {
    LeftOpening,
    LeftClosing,
    LeftDetach,
    RightOpening,
    RightClosing,
    Done,
};

static BarrierTestPhase _barrierPhase = BarrierTestPhase::Done;
static unsigned long    _barrierTimer = 0;

static void startBarrierTest() {
    _active       = ActiveTest::Barrier;
    _barrierPhase = BarrierTestPhase::LeftOpening;
    _barrierTimer = millis();

    _motion.setLeftServoAngle(BridgeMotion::L_SERVO_OPEN);
    Serial.print(F("\n[BARRIER] LEFT servo — opening ("));
    Serial.print(BridgeMotion::L_SERVO_OPEN);
    Serial.println(F("deg)..."));
}

static void updateBarrierTest() {
    unsigned long elapsed = millis() - _barrierTimer;

    switch (_barrierPhase) {
        case BarrierTestPhase::LeftOpening:
            if (elapsed >= BridgeMotion::BARRIER_MOVE_MS) {
                Serial.print(F("[BARRIER] LEFT open done. Closing ("));
                Serial.print(BridgeMotion::L_SERVO_CLOSED);
                Serial.println(F("deg)..."));
                _motion.setLeftServoAngle(BridgeMotion::L_SERVO_CLOSED);
                _barrierPhase = BarrierTestPhase::LeftClosing;
                _barrierTimer = millis();
            }
            break;

        case BarrierTestPhase::LeftClosing:
            if (elapsed >= BridgeMotion::BARRIER_MOVE_MS) {
                _motion.detachLeftServo();
                Serial.println(F("[BARRIER] LEFT close done. Detached. Settling..."));
                _barrierPhase = BarrierTestPhase::LeftDetach;
                _barrierTimer = millis();
            }
            break;

        case BarrierTestPhase::LeftDetach:
            if (elapsed >= BridgeMotion::BARRIER_STAGGER_MS) {
                _motion.setRightServoAngle(BridgeMotion::R_SERVO_OPEN);
                Serial.print(F("[BARRIER] RIGHT servo — opening ("));
                Serial.print(BridgeMotion::R_SERVO_OPEN);
                Serial.println(F("deg)..."));
                _barrierPhase = BarrierTestPhase::RightOpening;
                _barrierTimer = millis();
            }
            break;

        case BarrierTestPhase::RightOpening:
            if (elapsed >= BridgeMotion::BARRIER_MOVE_MS) {
                Serial.print(F("[BARRIER] RIGHT open done. Closing ("));
                Serial.print(BridgeMotion::R_SERVO_CLOSED);
                Serial.println(F("deg)..."));
                _motion.setRightServoAngle(BridgeMotion::R_SERVO_CLOSED);
                _barrierPhase = BarrierTestPhase::RightClosing;
                _barrierTimer = millis();
            }
            break;

        case BarrierTestPhase::RightClosing:
            if (elapsed >= BridgeMotion::BARRIER_MOVE_MS) {
                _motion.detachRightServo();
                Serial.println(F("[BARRIER] RIGHT close done. Detached. PASS."));
                _barrierPhase = BarrierTestPhase::Done;
                _active       = ActiveTest::None;
            }
            break;

        case BarrierTestPhase::Done:
            break;
    }
}

// ===========================================================================
// 'l' — Lights / Buzzer Test
// ===========================================================================
static constexpr unsigned long LIGHT_STEP_MS = 300UL;
static constexpr unsigned long BUZZER_MS     = 500UL;

static uint8_t       _lightStep      = 0;
static unsigned long _lightStepStart = 0;

static const char* const LIGHT_NAMES[8] = {
    "Q0 Road Red", "Q1 Road Green", "Q2 Road Yellow",
    "Q3 Water Red", "Q4 Water Green",
    "Q5 Status-1",  "Q6 Status-2",  "Q7 Fault"
};

static void _clearAllOutputs() {
    _signaling.setRoadTraffic(Color::Off);
    _signaling.setWaterTraffic(Color::Off);
    _signaling.setStatus(false, false);
    _signaling.setFault(false);
    _signaling.stopAlarm();
    _signaling.setBuzzer(false);
}

static void _applyLightStep(uint8_t step) {
    _clearAllOutputs();
    switch (step) {
        case 0: _signaling.setRoadTraffic(Color::Red);    break;
        case 1: _signaling.setRoadTraffic(Color::Green);  break;
        case 2: _signaling.setRoadTraffic(Color::Yellow); break;
        case 3: _signaling.setWaterTraffic(Color::Red);   break;
        case 4: _signaling.setWaterTraffic(Color::Green); break;
        case 5: _signaling.setStatus(true,  false);       break;
        case 6: _signaling.setStatus(false, true);        break;
        case 7: _signaling.setFault(true);                break;
        default: break;
    }
}

static void startLightsTest() {
    _active         = ActiveTest::Lights;
    _lightStep      = 0;
    _lightStepStart = millis();
    _applyLightStep(0);
    Serial.print(F("\n[LIGHTS] "));
    Serial.println(LIGHT_NAMES[0]);
}

static void updateLightsTest() {
    unsigned long elapsed = millis() - _lightStepStart;

    if (_lightStep < 8) {
        if (elapsed >= LIGHT_STEP_MS) {
            _lightStep++;
            _lightStepStart = millis();

            if (_lightStep < 8) {
                _applyLightStep(_lightStep);
                Serial.print(F("[LIGHTS] "));
                Serial.println(LIGHT_NAMES[_lightStep]);
            } else {
                _clearAllOutputs();
                _signaling.setBuzzer(true);
                Serial.println(F("[BUZZER] BEEP on"));
            }
        }
    } else {
        if (elapsed >= BUZZER_MS) {
            _signaling.setBuzzer(false);
            Serial.println(F("[BUZZER] BEEP off. PASS."));
            _clearAllOutputs();
            _active = ActiveTest::None;
        }
    }
}

// ===========================================================================
// 'v' — Variable Servo Sweep
// ===========================================================================
// Sweeps LEFT then RIGHT 0->180 one degree at a time via PCA9685.
// Single-servo-at-a-time rule preserved (5V rail safety).
// ===========================================================================
static constexpr unsigned long VS_STEP_MS = 20UL;

enum class VarServoPhase : uint8_t { LeftSweep, LeftSettle, RightSweep, Done };

static VarServoPhase _vsPhase     = VarServoPhase::Done;
static uint8_t       _vsAngle     = 0;
static unsigned long _vsStepTimer = 0;

static void startVarServoTest() {
    _active      = ActiveTest::VarServo;
    _vsPhase     = VarServoPhase::LeftSweep;
    _vsAngle     = 0;
    _vsStepTimer = millis();

    _motion.setLeftServoAngle(0);
    Serial.println(F("\n[VARSERVO] LEFT servo sweep 0->180deg"));
    Serial.println(F("  angle=0"));
}

static void updateVarServoTest() {
    unsigned long elapsed = millis() - _vsStepTimer;

    switch (_vsPhase) {

        case VarServoPhase::LeftSweep:
            if (elapsed >= VS_STEP_MS) {
                _vsStepTimer = millis();
                _vsAngle++;
                if (_vsAngle <= 180) {
                    _motion.setLeftServoAngle(_vsAngle);
                    Serial.print(F("  L angle="));
                    Serial.println(_vsAngle);
                } else {
                    _motion.detachLeftServo();
                    Serial.println(F("[VARSERVO] LEFT done. Detached. Settling..."));
                    _vsPhase     = VarServoPhase::LeftSettle;
                    _vsStepTimer = millis();
                }
            }
            break;

        case VarServoPhase::LeftSettle:
            if (elapsed >= BridgeMotion::BARRIER_STAGGER_MS) {
                _vsAngle = 0;
                _motion.setRightServoAngle(0);
                Serial.println(F("[VARSERVO] RIGHT servo sweep 0->180deg"));
                Serial.println(F("  angle=0"));
                _vsPhase     = VarServoPhase::RightSweep;
                _vsStepTimer = millis();
            }
            break;

        case VarServoPhase::RightSweep:
            if (elapsed >= VS_STEP_MS) {
                _vsStepTimer = millis();
                _vsAngle++;
                if (_vsAngle <= 180) {
                    _motion.setRightServoAngle(_vsAngle);
                    Serial.print(F("  R angle="));
                    Serial.println(_vsAngle);
                } else {
                    _motion.detachRightServo();
                    Serial.println(F("[VARSERVO] RIGHT done. Detached. PASS."));
                    _vsPhase = VarServoPhase::Done;
                    _active  = ActiveTest::None;
                }
            }
            break;

        case VarServoPhase::Done:
            break;
    }
}

// ===========================================================================
// 'i' — Input Monitor
// ===========================================================================
static constexpr unsigned long INPUT_DURATION_MS   = 5000UL;
static constexpr unsigned long INPUT_PRINT_RATE_MS =  500UL;

static unsigned long _inputEnd       = 0;
static unsigned long _inputLastPrint = 0;

static void startInputMonitor() {
    _active         = ActiveTest::InputMonitor;
    _inputEnd       = millis() + INPUT_DURATION_MS;
    _inputLastPrint = 0;
    Serial.println(F("\n[INPUT] Monitoring for 5 s — trigger sensors now."));
    Serial.println(F("  [reed         | ir            | weight       ]"));
}

static void updateInputMonitor() {
    if (millis() >= _inputEnd) {
        Serial.println(F("[INPUT] Done."));
        _active = ActiveTest::None;
        return;
    }

    if (millis() - _inputLastPrint >= INPUT_PRINT_RATE_MS) {
        _inputLastPrint = millis();
        Serial.print(F("  reed="));
        Serial.print(_sensors.isReedClosed()       ? F("CLOSED  ") : F("open    "));
        Serial.print(F(" ir="));
        Serial.print(_sensors.isObstacleBlocking() ? F("BLOCKED ") : F("clear   "));
        Serial.print(F(" weight="));
        Serial.println(_sensors.isWeightDetected() ? F("DETECTED") : F("none    "));
    }
}

// ===========================================================================
// Menu
// ===========================================================================
static void printMenu() {
    Serial.println(F("\n=== Bridge HW Validation ==="));
    Serial.println(F("  s  Stepper  — 500 fwd + 500 back, encoder live"));
    Serial.println(F("  b  Barriers — left: open->close->detach, then right: open->close->detach"));
    Serial.println(F("  l  Lights   — cycle Q0-Q7 then buzzer beep"));
    Serial.println(F("  i  Inputs   — reed / IR / weight monitor 5 s"));
    Serial.println(F("  v  VarServo — sweep L then R 0->180deg, prints each degree"));
    Serial.println(F("============================\n"));
}

// ===========================================================================
// Arduino entry points
// ===========================================================================
void setup() {
    Serial.begin(115200);
    delay(1500); // wait for USB CDC to enumerate on ESP32-S3

    bool i2cOk = _i2c.begin();
    bool mcpOk = _mcp.begin(MCP_A_IODIR, MCP_B_IODIR, MCP_A_PULLUP, MCP_B_PULLUP);
    bool pcaOk = _pca.begin(50.0f);

    Serial.println(F("\n[HW VALIDATION] I2C bus init:"));
    Serial.print(F("  I2CManager      : "));
    Serial.println(i2cOk ? F("OK") : F("FAIL"));
    Serial.print(F("  MCP23017 (0x20) : "));
    Serial.println(mcpOk ? F("OK") : F("FAIL — check I2C wiring / address jumpers"));
    Serial.print(F("  PCA9685  (0x40) : "));
    Serial.println(pcaOk ? F("OK") : F("FAIL — check I2C wiring / address jumpers"));

    bool motionOk    = _motion.begin();
    bool sensorsOk   = _sensors.begin();
    bool signalingOk = _signaling.begin();

    Serial.println(F("[HW VALIDATION] Subsystems:"));
    Serial.print(F("  BridgeMotion    : "));
    Serial.println(motionOk    ? F("OK") : F("FAIL"));
    Serial.print(F("  BridgeSensors   : "));
    Serial.println(sensorsOk   ? F("OK") : F("FAIL"));
    Serial.print(F("  BridgeSignaling : "));
    Serial.println(signalingOk ? F("OK") : F("FAIL"));

    printMenu();
}

void loop() {
    _motion.update();
    _sensors.update();
    _signaling.update();

    if (Serial.available()) {
        char cmd = static_cast<char>(Serial.read());

        if (cmd == '\r' || cmd == '\n') return;

        if (_active != ActiveTest::None) {
            Serial.println(F("[!] Test in progress — wait for it to finish."));
            return;
        }

        switch (cmd) {
            case 's': startStepperTest();  break;
            case 'b': startBarrierTest();  break;
            case 'l': startLightsTest();   break;
            case 'i': startInputMonitor(); break;
            case 'v': startVarServoTest(); break;
            default:
                Serial.print(F("[!] Unknown command: "));
                Serial.println(cmd);
                printMenu();
                break;
        }
    }

    switch (_active) {
        case ActiveTest::Stepper:      updateStepperTest();  break;
        case ActiveTest::Barrier:      updateBarrierTest();  break;
        case ActiveTest::Lights:       updateLightsTest();   break;
        case ActiveTest::InputMonitor: updateInputMonitor(); break;
        case ActiveTest::VarServo:     updateVarServoTest(); break;
        case ActiveTest::None:         break;
    }
}
