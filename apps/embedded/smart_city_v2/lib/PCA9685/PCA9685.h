#pragma once

#include <Arduino.h>
#include "I2CManager.h"

/// Thin driver for PCA9685 16-channel PWM controller using I2CManager.
/// Supports servo angle control and fixed-duration trigger pulses (HC-SR04).
class PCA9685 {
public:
    PCA9685(I2CManager& bus, uint8_t addr);

    /// Initialise: set PWM frequency and wake the oscillator.
    /// @param freq  Target PWM frequency in Hz (e.g. 50.0 for servos).
    bool begin(float freq);

    /// Set raw ON/OFF counts for a channel (0-4095 each).
    void setChannel(uint8_t ch, uint16_t on, uint16_t off);

    /// Disable a channel using the hardware full-off bit (zero current draw).
    void setFullOff(uint8_t ch);

    /// Drive an SG90-compatible servo to the given angle (0-180 deg).
    void setServoAngle(uint8_t ch, uint8_t degrees);

    /// Configure a channel to emit a continuous 10 us trigger pulse at
    /// the current PWM frequency, starting at the given cycle offset.
    /// Used for HC-SR04 sonar triggers; offset staggers East/West pulses.
    void setSonarTrigger(uint8_t ch, uint16_t offset);

private:
    I2CManager& _bus;
    uint8_t     _addr;

    static constexpr uint8_t REG_MODE1     = 0x00;
    static constexpr uint8_t REG_MODE2     = 0x01;
    static constexpr uint8_t REG_LED0      = 0x06;  // LED0_ON_L base
    static constexpr uint8_t REG_PRE_SCALE = 0xFE;

    // SG90 pulse width at 50 Hz (4096 counts = 20 ms)
    // 0 deg  = 0.5 ms = 102 counts
    // 180 deg = 2.5 ms = 512 counts
    static constexpr uint16_t SERVO_MIN = 102;
    static constexpr uint16_t SERVO_MAX = 512;

    // HC-SR04 requires >= 10 us HIGH pulse.
    // At 50 Hz: 1 count = 20 ms / 4096 ~= 4.88 us => 3 counts >= 14.6 us.
    static constexpr uint16_t SONAR_PULSE_COUNTS = 3;

    static constexpr float OSC_FREQ_HZ = 25000000.0f;
};
