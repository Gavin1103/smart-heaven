#pragma once

#include <Arduino.h>
#include "I2CManager.h"

/// Thin driver for MCP23017 I/O expander using I2CManager for bus access.
/// IOCON.BANK=0 (default register map).
class MCP23017 {
public:
    MCP23017(I2CManager& bus, uint8_t addr);

    /// Configure direction (1=input, 0=output) and pull-ups for both ports.
    bool begin(uint8_t iodirA, uint8_t iodirB, uint8_t gppuA, uint8_t gppuB);

    uint8_t readPortA();
    uint8_t readPortB();

    /// Write output latch A (only affects output-configured pins).
    void writePortA(uint8_t val);

    /// Set or clear a single bit in the output latch for Port A, then write.
    void setBitA(uint8_t bit, bool on);

    /// Set or clear a single bit in the output latch for Port B, then write.
    void setBitB(uint8_t bit, bool on);

    /// Cached output latch value for Port A (used by BridgeMotion to preserve upper nibble).
    uint8_t latchA() const { return _latchA; }

private:
    I2CManager& _bus;
    uint8_t     _addr;
    uint8_t     _latchA = 0;
    uint8_t     _latchB = 0;

    // Last successfully read input bytes. Returned when an I2C read fails so a
    // transient bus error cannot inject a spurious all-low (= all active-low
    // inputs triggered) reading into the FSM. Seeded 0xFF = nothing triggered.
    uint8_t     _lastInputA = 0xFF;
    uint8_t     _lastInputB = 0xFF;

    // MCP23017 register addresses (IOCON.BANK=0)
    static constexpr uint8_t REG_IODIRA = 0x00;
    static constexpr uint8_t REG_IODIRB = 0x01;
    static constexpr uint8_t REG_GPPUA  = 0x0C;
    static constexpr uint8_t REG_GPPUB  = 0x0D;
    static constexpr uint8_t REG_GPIOA  = 0x12;
    static constexpr uint8_t REG_GPIOB  = 0x13;
    static constexpr uint8_t REG_OLATA  = 0x14;
    static constexpr uint8_t REG_OLATB  = 0x15;
};
