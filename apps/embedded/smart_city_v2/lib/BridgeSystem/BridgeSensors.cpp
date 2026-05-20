#include "BridgeSensors.h"

namespace bridge {

BridgeSensors::BridgeSensors(MCP23017& mcp, PCA9685& pca)
    : _mcp(mcp), _pca(pca) {}

bool BridgeSensors::begin() {
    _sonarEast.echoPortBBit = MCP_B_ECHO_EAST;
    _sonarWest.echoPortBBit = MCP_B_ECHO_WEST;

    _pca.setSonarTrigger(PCA_CH_SONAR_EAST, PCA_SONAR_EAST_OFFSET);
    _pca.setSonarTrigger(PCA_CH_SONAR_WEST, PCA_SONAR_WEST_OFFSET);

    uint8_t portB = _mcp.readPortB();
    _encoder.lastClk = (portB >> MCP_B_ENC_A) & 1;

    tareLoadCell();
    return true;
}

void BridgeSensors::update() {
    uint8_t portB = _mcp.readPortB();
    _updateSonar(_sonarEast, portB);
    _updateSonar(_sonarWest, portB);
    _updateIrReed(portB);
    _updateEncoder(portB);
    _updateHx711();
}

void BridgeSensors::tareLoadCell() {
    _hx711.phase       = Hx711Phase::Taring;
    _hx711.tareSamples = 0;
    _hx711.tareAccum   = 0.0f;
    _hx711.baseline    = 0.0f;
    _hx711.carCounter  = 0;
    _hx711.lastPollMs  = millis();
}

void BridgeSensors::_updateSonar(SonarSM& sm, uint8_t portBByte) {
    bool echoHigh = (portBByte >> sm.echoPortBBit) & 0x01;

    if (echoHigh && !sm.lastEchoHigh) {
        sm.echoStartUs = micros();
    } else if (!echoHigh && sm.lastEchoHigh) {
        uint32_t pulseUs = static_cast<uint32_t>(micros() - sm.echoStartUs);
        if (pulseUs >= SONAR_MIN_PULSE_US && pulseUs <= SONAR_MAX_PULSE_US) {
            uint32_t distMm = (pulseUs * 343UL) / 2000UL;
            sm.lastDistMm = distMm;
            if (distMm < BOAT_DETECT_MM) {
                if (sm.confirmCount < DEBOUNCE_SAMPLES) sm.confirmCount++;
            } else {
                sm.confirmCount = 0;
            }
        }
    }
    sm.lastEchoHigh = echoHigh;
}

void BridgeSensors::_updateIrReed(uint8_t portBByte) {
    bool irBlocked = !((portBByte >> MCP_B_IR)   & 0x01);
    bool reedLow   = !((portBByte >> MCP_B_REED) & 0x01);

    if (irBlocked) { if (_irConfirmCount   < DEBOUNCE_SAMPLES) _irConfirmCount++;   }
    else           { _irConfirmCount   = 0; }
    if (reedLow)   { if (_reedConfirmCount < DEBOUNCE_SAMPLES) _reedConfirmCount++; }
    else           { _reedConfirmCount = 0; }
}

void BridgeSensors::_updateEncoder(uint8_t portBByte) {
    bool currentClk = (portBByte >> MCP_B_ENC_A) & 0x01;
    if (currentClk != _encoder.lastClk) {
        bool dt = (portBByte >> MCP_B_ENC_B) & 0x01;
        if (dt != currentClk) _encoder.position++;
        else                  _encoder.position--;
    }
    _encoder.lastClk       = currentClk;
    _encoder.buttonPressed = !((portBByte >> MCP_B_ENC_SW) & 0x01);
}

void BridgeSensors::_updateHx711() {
    unsigned long now    = millis();
    unsigned long pollMs = (_hx711.phase == Hx711Phase::Taring)
                               ? HX711_TARE_POLL_MS : HX711_POLL_MS;

    if (now - _hx711.lastPollMs < pollMs) return;

    uint8_t portA = _mcp.readPortA();
    if ((portA >> MCP_A_HX_DT) & 0x01) {
        if (now - _hx711.lastPollMs >= HX711_READY_TIMEOUT_MS) _hx711.lastPollMs = now;
        return;
    }

    int32_t raw = _hx711ReadRaw();
    _hx711.lastPollMs = now;
    if (_isHx711Glitch(raw)) return;

    switch (_hx711.phase) {
        case Hx711Phase::Taring:
            _hx711.tareAccum += static_cast<float>(raw);
            _hx711.tareSamples++;
            if (_hx711.tareSamples >= HX711_TARE_SAMPLES) {
                _hx711.baseline = _hx711.tareAccum / static_cast<float>(HX711_TARE_SAMPLES);
                _hx711.phase    = Hx711Phase::Reading;
            }
            break;
        case Hx711Phase::Reading: {
            float current = static_cast<float>(raw);
            float diff    = current - _hx711.baseline;
            if (diff < 0.0f) diff = -diff;
            float grams   = diff / HX711_SCALE;
            if (grams > CAR_THRESHOLD_G) {
                if (_hx711.carCounter < HX711_CONFIRM_SAMPLES) _hx711.carCounter++;
            } else {
                _hx711.carCounter = 0;
                _hx711.baseline   = _hx711.baseline * (1.0f - DRIFT_ALPHA)
                                  + current * DRIFT_ALPHA;
            }
            break;
        }
    }
}

// Bit-bangs 24 bits from HX711 via MCP23017 Port A.
// SCK HIGH per bit lasts ~200 us (one I2C write), exceeding the HX711 60 us
// power-down threshold. See bridge_pcb_design.md sec 4.4 -- deliberate trade-off.
int32_t BridgeSensors::_hx711ReadRaw() {
    int32_t value = 0;
    for (uint8_t i = 0; i < 24; i++) {
        _mcp.setBitA(MCP_A_HX_SCK, true);
        uint8_t portA = _mcp.readPortA();
        _mcp.setBitA(MCP_A_HX_SCK, false);
        value <<= 1;
        if ((portA >> MCP_A_HX_DT) & 0x01) value |= 1;
    }
    // 25th pulse sets gain=128 for next conversion
    _mcp.setBitA(MCP_A_HX_SCK, true);
    _mcp.setBitA(MCP_A_HX_SCK, false);

    if (value & 0x800000) value |= static_cast<int32_t>(0xFF000000);
    return value;
}

} // namespace bridge
