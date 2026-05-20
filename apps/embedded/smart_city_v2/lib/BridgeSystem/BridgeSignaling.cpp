#include "BridgeSignaling.h"

namespace bridge {

BridgeSignaling::BridgeSignaling(MCP23017& mcp, PCA9685& pca)
    : _mcp(mcp), _pca(pca) {}

bool BridgeSignaling::begin() {
    // Silence buzzer output on PCA9685 CH2
    _pca.setFullOff(PCA_CH_BUZZER);

    // Clear all LEDs via the shift register
    _outputByte = 0x00;
    _writeByte(_outputByte);
    return true;
}

void BridgeSignaling::update() {
    if (!_alarmActive) return;
    unsigned long now = millis();
    if (now - _alarmLastToggle >= ALARM_TOGGLE_MS) {
        _alarmLastToggle = now;
        _alarmBuzzerOn   = !_alarmBuzzerOn;
        _setBuzzerPwm(_alarmBuzzerOn);
    }
}

void BridgeSignaling::setRoadTraffic(Color c) {
    _outputByte &= ~(BIT_ROAD_RED | BIT_ROAD_GREEN | BIT_ROAD_YELLOW);
    switch (c) {
        case Color::Red:    _outputByte |= BIT_ROAD_RED;    break;
        case Color::Green:  _outputByte |= BIT_ROAD_GREEN;  break;
        case Color::Yellow: _outputByte |= BIT_ROAD_YELLOW; break;
        case Color::Off:    break;
    }
    _flushOutputs();
}

void BridgeSignaling::setWaterTraffic(Color c) {
    _outputByte &= ~(BIT_WATER_RED | BIT_WATER_GREEN);
    switch (c) {
        case Color::Red:   _outputByte |= BIT_WATER_RED;   break;
        case Color::Green: _outputByte |= BIT_WATER_GREEN; break;
        case Color::Yellow:
        case Color::Off:   break;
    }
    _flushOutputs();
}

void BridgeSignaling::setStatus(bool s1, bool s2) {
    _outputByte &= ~(BIT_STATUS_1 | BIT_STATUS_2);
    if (s1) _outputByte |= BIT_STATUS_1;
    if (s2) _outputByte |= BIT_STATUS_2;
    _flushOutputs();
}

void BridgeSignaling::setFault(bool on) {
    if (on) _outputByte |= BIT_FAULT;
    else    _outputByte &= ~BIT_FAULT;
    _flushOutputs();
}

void BridgeSignaling::setBuzzer(bool on) {
    _alarmActive = false;
    _setBuzzerPwm(on);
}

void BridgeSignaling::triggerAlarm() {
    _alarmActive     = true;
    _alarmBuzzerOn   = true;
    _alarmLastToggle = millis();
    _setBuzzerPwm(true);
}

void BridgeSignaling::stopAlarm() {
    _alarmActive = false;
    _setBuzzerPwm(false);
}

// Shift 8 bits MSB-first into the 74HC595 via MCP23017, then latch.
// Each bit requires 3 MCP23017 I2C writes (set DATA, pulse CLK high, CLK low).
// Total for 8 bits: ~24 I2C writes (~4.8 ms at 400 kHz).
// This is acceptable for LED state changes (not time-critical).
//
// MCP23017 Port A latch is preserved: only SR_DATA (GPA6) and SR_CLK (GPA7)
// bits change; stepper (GPA0-3) and HX711 (GPA5) bits are left intact.
void BridgeSignaling::_writeByte(uint8_t data) {
    for (int8_t i = 7; i >= 0; i--) {
        bool bit = (data >> i) & 0x01;
        _mcp.setBitA(MCP_A_SR_DATA, bit);   // set SER
        _mcp.setBitA(MCP_A_SR_CLK, true);   // SRCLK rising edge
        _mcp.setBitA(MCP_A_SR_CLK, false);  // SRCLK falling edge
    }
    // Latch pulse on GPB0 (SR_LATCH / RCLK)
    _mcp.setBitB(MCP_B_SR_LATCH, true);
    _mcp.setBitB(MCP_B_SR_LATCH, false);
}

void BridgeSignaling::_flushOutputs() {
    _writeByte(_outputByte);
}

// Enable or disable the PCA9685 buzzer channel.
// Full-on (50% duty at 50 Hz = 10 ms HIGH per cycle) drives an active buzzer;
// full-off silences it. The 500 ms alarm pattern is handled in update().
void BridgeSignaling::_setBuzzerPwm(bool on) {
    if (on) {
        // 50% duty cycle: ON=0, OFF=2048 (half of 4096 counts at 50 Hz)
        _pca.setChannel(PCA_CH_BUZZER, 0, 2048);
    } else {
        _pca.setFullOff(PCA_CH_BUZZER);
    }
}

} // namespace bridge
