#ifndef BRIDGE_SENSORS_H
#define BRIDGE_SENSORS_H

#include <Arduino.h>
#include "city_config.h"
#include "MCP23017.h"
#include "PCA9685.h"

namespace bridge {

/// Aggregates all sensor subsystems for the Smart Drawbridge via the Bridge Node PCB.
///
/// Hardware interface (Phase 1 — PCB):
///   MCP23017 Port B reads: encoder (GPB1-3), reed (GPB4), IR (GPB5), sonar echo (GPB6-7)
///   MCP23017 Port A reads/writes: HX711 bit-bang (GPA4=DT in, GPA5=SCK out)
///   PCA9685 CH3/CH4: HC-SR04 trigger pulses at 50 Hz (continuous, staggered)
///
/// All digital inputs are active-low with MCP23017 pull-ups enabled.
///
/// HX711 timing note: bit-banging 24 bits through MCP23017 I2C takes ~11 ms and
/// causes SCK to stay HIGH for >60 µs per bit (exceeding the HX711 60 µs power-down
/// threshold). This is a deliberate design trade-off — reads are taken at 1 Hz and
/// any corrupt readings are discarded via glitch detection. See bridge_pcb_design.md §4.4.
///
/// Call begin() once in setup(). Call update() every loop() iteration.
class BridgeSensors {
public:
    BridgeSensors(MCP23017& mcp, PCA9685& pca);

    bool begin();
    void update();

    // -----------------------------------------------------------------------
    // HC-SR04 — boat approach detection (East / West)
    // -----------------------------------------------------------------------
    bool     isBoatDetectedEast() const { return _sonarEast.confirmCount >= DEBOUNCE_SAMPLES; }
    bool     isBoatDetectedWest() const { return _sonarWest.confirmCount >= DEBOUNCE_SAMPLES; }
    uint32_t getDistanceEastMm()  const { return _sonarEast.lastDistMm; }
    uint32_t getDistanceWestMm()  const { return _sonarWest.lastDistMm; }

    // -----------------------------------------------------------------------
    // KY-032 IR Obstacle — under-bridge clearance safety interlock
    // -----------------------------------------------------------------------
    bool isObstacleBlocking() const { return _irConfirmCount >= DEBOUNCE_SAMPLES; }

    // -----------------------------------------------------------------------
    // KY-025 Reed Switch — magnetic home/closed limit switch
    // -----------------------------------------------------------------------
    bool isReedClosed() const { return _reedConfirmCount >= DEBOUNCE_SAMPLES; }

    // -----------------------------------------------------------------------
    // HX711 Load Cell — bridge deck occupancy (car present)
    // -----------------------------------------------------------------------
    void tareLoadCell();
    bool isWeightDetected() const { return _hx711.carCounter >= HX711_CONFIRM_SAMPLES; }

    // -----------------------------------------------------------------------
    // KY-040 Rotary Encoder — position feedback and stall detection
    // -----------------------------------------------------------------------
    int32_t getEncoderPosition()  const { return _encoder.position; }
    void    resetEncoderPosition()      { _encoder.position = 0; }
    bool    isEncoderPressed()    const { return _encoder.buttonPressed; }

private:
    MCP23017& _mcp;
    PCA9685&  _pca;

    // -----------------------------------------------------------------------
    // Detection thresholds
    // -----------------------------------------------------------------------
    static constexpr uint8_t  DEBOUNCE_SAMPLES      = 2;
    static constexpr uint8_t  HX711_CONFIRM_SAMPLES  = 3;
    static constexpr uint32_t BOAT_DETECT_MM         = 500;
    static constexpr uint32_t SONAR_MIN_PULSE_US     = 100;
    static constexpr uint32_t SONAR_MAX_PULSE_US     = 35000;

    // -----------------------------------------------------------------------
    // HX711 timing
    // -----------------------------------------------------------------------
    static constexpr unsigned long HX711_TARE_POLL_MS     = 10UL;
    static constexpr unsigned long HX711_POLL_MS           = 150UL;
    static constexpr unsigned long HX711_READY_TIMEOUT_MS  = 100UL;
    static constexpr uint8_t       HX711_TARE_SAMPLES      = 30;

    // HX711 raw-value sentinels that indicate corrupt readings (from load_cell.rs).
    static constexpr int32_t HX711_GLITCH_ZERO     = 0;
    static constexpr int32_t HX711_GLITCH_NEG_ONE  = -1;
    static constexpr int32_t HX711_GLITCH_SATURATE = 8388607;  // 0x7FFFFF

    static constexpr float CAR_THRESHOLD_G = 90.0f;
    static constexpr float HX711_SCALE     = 0.76f;
    static constexpr float DRIFT_ALPHA     = 0.2f;

    // -----------------------------------------------------------------------
    // HC-SR04 sonar state
    // Triggers are generated continuously by PCA9685. We poll MCP23017 Port B
    // to detect rising/falling edges on the echo pins and measure pulse width.
    // -----------------------------------------------------------------------
    struct SonarSM {
        uint8_t  echoPortBBit  = 0;     // which bit in Port B is this echo pin
        bool     lastEchoHigh  = false;
        uint32_t echoStartUs   = 0;     // micros() when echo went HIGH
        uint32_t lastDistMm    = 0;
        uint8_t  confirmCount  = 0;
    };

    SonarSM _sonarEast;
    SonarSM _sonarWest;

    // -----------------------------------------------------------------------
    // HX711 non-blocking state machine
    // -----------------------------------------------------------------------
    enum class Hx711Phase : uint8_t { Taring, Reading };

    struct Hx711SM {
        Hx711Phase    phase       = Hx711Phase::Taring;
        unsigned long lastPollMs  = 0;
        float         baseline    = 0.0f;
        uint8_t       tareSamples = 0;
        float         tareAccum   = 0.0f;
        uint8_t       carCounter  = 0;
    };

    Hx711SM _hx711;

    // -----------------------------------------------------------------------
    // Encoder state
    // -----------------------------------------------------------------------
    struct EncoderState {
        bool    lastClk       = false;
        int32_t position      = 0;
        bool    buttonPressed = false;
    };

    EncoderState _encoder;

    // Simple debounce counters for IR and reed (Port B reads)
    uint8_t _irConfirmCount   = 0;
    uint8_t _reedConfirmCount = 0;

    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------
    void    _updateSonar(SonarSM& sm, uint8_t portBByte);
    void    _updateIrReed(uint8_t portBByte);
    void    _updateEncoder(uint8_t portBByte);
    void    _updateHx711();
    int32_t _hx711ReadRaw();

    static bool _isHx711Glitch(int32_t v) {
        return v == HX711_GLITCH_ZERO
            || v == HX711_GLITCH_NEG_ONE
            || v == HX711_GLITCH_SATURATE;
    }
};

} // namespace bridge

#endif // BRIDGE_SENSORS_H
