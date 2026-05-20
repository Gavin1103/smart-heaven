#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <vector>

// Timeout for acquiring the I2C mutex (ms → ticks)
static constexpr uint32_t I2C_MUTEX_TIMEOUT_MS = 100;

class I2CManager {
public:
    /**
     * @param sda     GPIO pin for SDA
     * @param scl     GPIO pin for SCL
     * @param freq    I2C clock frequency in Hz (default 400 kHz)
     */
    I2CManager(uint8_t sda, uint8_t scl, uint32_t freq = 400000);

    // No copy – owns a FreeRTOS mutex handle
    I2CManager(const I2CManager&)            = delete;
    I2CManager& operator=(const I2CManager&) = delete;

    ~I2CManager();

    /**
     * Initialises Wire and creates the mutex.
     * Call once from setup() before any tile logic starts.
     * @return true on success
     */
    bool begin();

    /**
     * Scans the full 7-bit address space and returns every responding address.
     * Safe to call from any task.
     */
    std::vector<uint8_t> scan();

    /**
     * Write bytes to a register on a device.
     *
     * @param addr   7-bit I2C address
     * @param reg    Register / sub-address byte
     * @param data   Pointer to data buffer
     * @param len    Number of bytes to write
     * @return true if the transmission was acknowledged
     */
    bool writeBytes(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len);

    /**
     * Read bytes from a register on a device.
     *
     * @param addr   7-bit I2C address
     * @param reg    Register / sub-address byte
     * @param buf    Output buffer (must be at least `len` bytes)
     * @param len    Number of bytes to read
     * @return true if all bytes were received
     */
    bool readBytes(uint8_t addr, uint8_t reg, uint8_t* buf, size_t len);

private:
    uint8_t  sda_;
    uint8_t  scl_;
    uint32_t freq_;

    SemaphoreHandle_t mutex_;

    /** Acquire mutex with timeout. Returns false if it could not be taken. */
    bool takeMutex();

    /** Release mutex – only call after a successful takeMutex(). */
    void giveMutex();
};
