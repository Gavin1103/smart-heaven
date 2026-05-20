#ifndef BRIDGE_CONTROLLER_H
#define BRIDGE_CONTROLLER_H

#include "CityModule.h"
#include "I2CManager.h"
#include "MCP23017.h"
#include "PCA9685.h"
#include "BridgeSignaling.h"
#include "BridgeMotion.h"
#include "BridgeSensors.h"
#include "TelemetryManager.h"
#include "city_config.h"

namespace bridge {

class BridgeController : public CityModule {
public:
    // address = bridge node logical ID (0x10 = bridge 1, 0x11 = bridge 2, ...).
    // IC addresses on the PCB are derived automatically from this value.
    // telemetry is optional -- pass nullptr to disable remote logging.
    BridgeController(I2CManager& bus, uint8_t address,
                     telemetry::TelemetryManager* telemetry = nullptr);

    bool begin() override;
    void update() override;
    const char* getName() override { return "BridgeNode"; }

private:
    I2CManager& _bus;
    uint8_t     _address;

    // Bridge Node PCB peripherals -- must be declared before subsystems so that
    // C++ member initialisation order places them first in the constructor.
    MCP23017 _mcp;
    PCA9685  _pca;

    // Hardware subsystems -- all I/O now routed through _mcp and _pca.
    bridge::BridgeSignaling _signaling;
    bridge::BridgeMotion    _motion;
    bridge::BridgeSensors   _sensors;

    telemetry::TelemetryManager* _telemetry;

    // Derive MCP23017 I2C address from node address (see city_config.h)
    static uint8_t _mcpAddr(uint8_t nodeAddr) {
        return static_cast<uint8_t>(ADDR_BRIDGE_MCP_BASE +
               (nodeAddr - ADDR_BRIDGE_MIN));
    }
    // Derive PCA9685PW I2C address from node address
    static uint8_t _pcaAddr(uint8_t nodeAddr) {
        return static_cast<uint8_t>(ADDR_BRIDGE_PCA_BASE +
               (nodeAddr - ADDR_BRIDGE_MIN));
    }

    // -------------------------------------------------------------------------
    // Timing constants (translated from legacy/bridge/src/config.rs)
    // -------------------------------------------------------------------------
    static constexpr unsigned long HOMING_TIMEOUT_MS    = 30000UL;
    static constexpr unsigned long BOAT_CONFIRM_MS      = 1000UL;
    static constexpr unsigned long WARNING_DURATION_MS  = 3000UL;
    static constexpr unsigned long WAIT_CLEAR_MS        = 30000UL;
    static constexpr unsigned long BARRIER_MOVE_MS      = 2000UL;
    static constexpr unsigned long WATER_PREP_MS        = 2000UL;
    static constexpr unsigned long BRIDGE_TIMEOUT_MS    = 40000UL;
    static constexpr unsigned long CLEARANCE_CHECK_MS   = 2000UL;
    static constexpr unsigned long BRIDGE_WAIT_MS       = 30000UL;
    static constexpr int           HOMING_EXTRA_STEPS   = 200;
    static constexpr int           FULL_ROTATION        = 6400;

    // -------------------------------------------------------------------------
    // Fault reasons
    // -------------------------------------------------------------------------
    enum class FaultReason {
        HomingTimeout,
        OpeningTimeout,
        ClosingTimeout,
        InvalidStartupState,
        SafetyCheckFailed,
        MotorStall,
    };

    // -------------------------------------------------------------------------
    // FSM states
    // -------------------------------------------------------------------------
    enum class State {
        Startup,
        Homing,
        IdleClosed,
        BoatDetected,
        WarningRoadTraffic,
        WaitBridgeClear,
        LowerBarriers,
        WaterPrepRedGreen,
        Opening,
        Open,
        ClosingCheckClearance,
        Closing,
        RaiseBarriers,
        Fault,
    };

    State       _currentState  = State::Startup;
    FaultReason _faultReason   = FaultReason::InvalidStartupState;
    unsigned long _stateTimer  = 0;

    bool _reedTriggerPosSet = false;
    int  _reedTriggerPos    = 0;

    bool _boatDetected     = false;
    bool _obstacleDetected = false;
    bool _reedClosed       = false;
    bool _carDetected      = false;
    int  _stepperPos       = 0;
    bool _encoderSwPressed = false;

    void transitionTo(State newState);
    static const char* stateToString(State s);
    static const char* faultReasonToString(FaultReason reason);
    static telemetry::EventCode toEventCode(State from, State to);

    bool readBoatSensor();
    bool readObstacleSensor();
    bool readReedSensor();
    bool readCarDetected();
    int  readStepperPosition();
    bool readEncoderSwitch();

    void startHomingMotor();
    void startOpeningMotor();
    void startClosingMotor();
    void stopMotor();
    void lowerBarriers();
    void raiseBarriers();
    void stopBarriers();
    void setTrafficLightsIdle();
    void setTrafficLightsWarning(bool flashOn);
    void setTrafficLightsStop();
    void setWaterLightsRedGreen();
    void setWaterLightsGreen();
    void setBuzzer(bool on);
};

}

#endif
