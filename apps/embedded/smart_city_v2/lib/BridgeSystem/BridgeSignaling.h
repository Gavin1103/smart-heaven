#ifndef BRIDGE_SIGNALING_H
#define BRIDGE_SIGNALING_H

#include <Arduino.h>
#include "city_config.h"
#include "MCP23017.h"
#include "PCA9685.h"

namespace bridge {

enum class Color : uint8_t { Off, Red, Green, Yellow };

/// Drives the 74HC595 LED shift register (via MCP23017 Port A/B) and the
/// passive buzzer (via PCA9685 channel 2).
///
/// Shift register is clocked through three MCP23017 pins:
///   SR_DATA  = GPA6   SR_CLK = GPA7   SR_LATCH = GPB0
///
/// LED output map (74HC595 Q0-Q7, J6 on Bridge Node PCB):
///   Q0 Road Red    Q1 Road Green    Q2 Road Yellow
///   Q3 Water Red   Q4 Water Green
///   Q5 Status 1    Q6 Status 2      Q7 Fault
///
/// Buzzer: PCA9685 CH2 at 50 Hz -- full-on for alarm on, full-off for silent.
/// The 500 ms alarm toggle is software-driven by update().
///
/// Call begin() once in setup(). Call update() every loop() iteration.
class BridgeSignaling {
public:
    BridgeSignaling(MCP23017& mcp, PCA9685& pca);

    bool begin();
    void update();

    void setRoadTraffic(Color c);
    void setWaterTraffic(Color c);
    void setStatus(bool s1, bool s2);
    void setFault(bool on);

    void setBuzzer(bool on);
    void triggerAlarm();
    void stopAlarm();

private:
    MCP23017& _mcp;
    PCA9685&  _pca;

    // 74HC595 bit masks (Q0 = LSB, matches shift_register.rs write_byte mapping)
    static constexpr uint8_t BIT_ROAD_RED    = (1u << 0);
    static constexpr uint8_t BIT_ROAD_GREEN  = (1u << 1);
    static constexpr uint8_t BIT_ROAD_YELLOW = (1u << 2);
    static constexpr uint8_t BIT_WATER_RED   = (1u << 3);
    static constexpr uint8_t BIT_WATER_GREEN = (1u << 4);
    static constexpr uint8_t BIT_STATUS_1    = (1u << 5);
    static constexpr uint8_t BIT_STATUS_2    = (1u << 6);
    static constexpr uint8_t BIT_FAULT       = (1u << 7);

    static constexpr unsigned long ALARM_TOGGLE_MS = 500UL;

    uint8_t       _outputByte      = 0x00;
    bool          _alarmActive     = false;
    bool          _alarmBuzzerOn   = false;
    unsigned long _alarmLastToggle = 0;

    // Shift 8 bits MSB-first into the 74HC595 via MCP23017 and latch.
    // Mirrors write_byte() from shift_register.rs -- same bit order.
    void _writeByte(uint8_t data);
    void _flushOutputs();
    void _setBuzzerPwm(bool on);
};

} // namespace bridge

#endif // BRIDGE_SIGNALING_H
