#include "PCA9685.h"
#include <math.h>

PCA9685::PCA9685(I2CManager& bus, uint8_t addr)
    : _bus(bus), _addr(addr) {}

bool PCA9685::begin(float freq) {
    // Enter sleep mode so the prescaler can be written.
    uint8_t sleep = 0x10;
    if (!_bus.writeBytes(_addr, REG_MODE1, &sleep, 1)) return false;

    // PRE_SCALE = round(OSC / (4096 * freq)) - 1
    uint8_t prescale = static_cast<uint8_t>(roundf(OSC_FREQ_HZ / (4096.0f * freq)) - 1.0f);
    if (!_bus.writeBytes(_addr, REG_PRE_SCALE, &prescale, 1)) return false;

    // Wake up and wait for oscillator to stabilise.
    uint8_t wake = 0x00;
    if (!_bus.writeBytes(_addr, REG_MODE1, &wake, 1)) return false;
    delay(5);

    // Enable auto-increment so multi-byte channel writes work with one call.
    uint8_t autoInc = 0x20;
    if (!_bus.writeBytes(_addr, REG_MODE1, &autoInc, 1)) return false;

    // Totem-pole output drive (default, explicit for clarity).
    uint8_t mode2 = 0x04;
    _bus.writeBytes(_addr, REG_MODE2, &mode2, 1);

    return true;
}

void PCA9685::setChannel(uint8_t ch, uint16_t on, uint16_t off) {
    uint8_t buf[4] = {
        static_cast<uint8_t>(on  & 0xFF),
        static_cast<uint8_t>(on  >> 8),
        static_cast<uint8_t>(off & 0xFF),
        static_cast<uint8_t>(off >> 8),
    };
    _bus.writeBytes(_addr, static_cast<uint8_t>(REG_LED0 + ch * 4u), buf, 4);
}

void PCA9685::setFullOff(uint8_t ch) {
    // Setting bit 4 of LEDn_OFF_H forces the output permanently low.
    uint8_t buf[4] = { 0x00, 0x00, 0x00, 0x10 };
    _bus.writeBytes(_addr, static_cast<uint8_t>(REG_LED0 + ch * 4u), buf, 4);
}

void PCA9685::setServoAngle(uint8_t ch, uint8_t degrees) {
    uint16_t pulse = static_cast<uint16_t>(
        SERVO_MIN + (static_cast<uint32_t>(degrees) * (SERVO_MAX - SERVO_MIN)) / 180u
    );
    setChannel(ch, 0, pulse);
}

void PCA9685::setSonarTrigger(uint8_t ch, uint16_t offset) {
    uint16_t on  = offset;
    uint16_t off = static_cast<uint16_t>((offset + SONAR_PULSE_COUNTS) % 4096u);
    setChannel(ch, on, off);
}
