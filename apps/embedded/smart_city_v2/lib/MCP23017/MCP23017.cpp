#include "MCP23017.h"

MCP23017::MCP23017(I2CManager& bus, uint8_t addr)
    : _bus(bus), _addr(addr) {}

bool MCP23017::begin(uint8_t iodirA, uint8_t iodirB, uint8_t gppuA, uint8_t gppuB) {
    _latchA = 0;
    _latchB = 0;
    if (!_bus.writeBytes(_addr, REG_IODIRA, &iodirA, 1)) return false;
    if (!_bus.writeBytes(_addr, REG_IODIRB, &iodirB, 1)) return false;
    if (!_bus.writeBytes(_addr, REG_GPPUA,  &gppuA,  1)) return false;
    if (!_bus.writeBytes(_addr, REG_GPPUB,  &gppuB,  1)) return false;
    if (!_bus.writeBytes(_addr, REG_OLATA,  &_latchA, 1)) return false;
    if (!_bus.writeBytes(_addr, REG_OLATB,  &_latchB, 1)) return false;
    return true;
}

uint8_t MCP23017::readPortA() {
    uint8_t val = 0;
    if (_bus.readBytes(_addr, REG_GPIOA, &val, 1)) _lastInputA = val;
    return _lastInputA;
}

uint8_t MCP23017::readPortB() {
    uint8_t val = 0;
    if (_bus.readBytes(_addr, REG_GPIOB, &val, 1)) _lastInputB = val;
    return _lastInputB;
}

void MCP23017::writePortA(uint8_t val) {
    _latchA = val;
    _bus.writeBytes(_addr, REG_OLATA, &_latchA, 1);
}

void MCP23017::setBitA(uint8_t bit, bool on) {
    if (on) _latchA |=  static_cast<uint8_t>(1u << bit);
    else    _latchA &= ~static_cast<uint8_t>(1u << bit);
    _bus.writeBytes(_addr, REG_OLATA, &_latchA, 1);
}

void MCP23017::setBitB(uint8_t bit, bool on) {
    if (on) _latchB |=  static_cast<uint8_t>(1u << bit);
    else    _latchB &= ~static_cast<uint8_t>(1u << bit);
    _bus.writeBytes(_addr, REG_OLATB, &_latchB, 1);
}
