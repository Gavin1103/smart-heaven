#include "I2CManager.h"

I2CManager::I2CManager(uint8_t sda, uint8_t scl, uint32_t freq)
    : sda_(sda), scl_(scl), freq_(freq), mutex_(nullptr)
{
}

I2CManager::~I2CManager()
{
    if (mutex_) {
        vSemaphoreDelete(mutex_);
    }
}

bool I2CManager::begin()
{
    mutex_ = xSemaphoreCreateMutex();
    if (!mutex_) {
        Serial.println("[I2CManager] ERROR: Failed to create mutex");
        return false;
    }

    Wire.begin(sda_, scl_);
    Wire.setClock(freq_);

    Serial.printf("[I2CManager] Ready on SDA=%d SCL=%d @ %lu Hz\n",
                  sda_, scl_, (unsigned long)freq_);
    return true;
}

std::vector<uint8_t> I2CManager::scan()
{
    std::vector<uint8_t> found;

    if (!takeMutex()) {
        Serial.println("[I2CManager] scan: mutex timeout");
        return found;
    }

    for (uint8_t addr = 0x08; addr <= 0x77; ++addr) {
        Wire.beginTransmission(addr);
        uint8_t err = Wire.endTransmission();
        if (err == 0) {
            found.push_back(addr);
            Serial.printf("[I2CManager] Device found at 0x%02X\n", addr);
        }
    }

    giveMutex();

    if (found.empty()) {
        Serial.println("[I2CManager] No I2C devices found");
    }

    return found;
}

bool I2CManager::writeBytes(uint8_t addr, uint8_t reg,
                             const uint8_t* data, size_t len)
{
    if (!data || len == 0) return false;

    if (!takeMutex()) {
        Serial.printf("[I2CManager] writeBytes: mutex timeout (addr=0x%02X)\n", addr);
        return false;
    }

    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(data, len);
    uint8_t err = Wire.endTransmission();

    giveMutex();

    if (err != 0) {
        Serial.printf("[I2CManager] writeBytes failed: addr=0x%02X reg=0x%02X err=%d\n",
                      addr, reg, err);
        return false;
    }
    return true;
}

bool I2CManager::readBytes(uint8_t addr, uint8_t reg,
                            uint8_t* buf, size_t len)
{
    if (!buf || len == 0) return false;

    if (!takeMutex()) {
        Serial.printf("[I2CManager] readBytes: mutex timeout (addr=0x%02X)\n", addr);
        return false;
    }

    // Point the device at the register
    Wire.beginTransmission(addr);
    Wire.write(reg);
    uint8_t err = Wire.endTransmission(false);   // repeated-start

    if (err != 0) {
        giveMutex();
        Serial.printf("[I2CManager] readBytes: addressing failed addr=0x%02X reg=0x%02X err=%d\n",
                      addr, reg, err);
        return false;
    }

    size_t received = Wire.requestFrom(static_cast<uint8_t>(addr),
                                       static_cast<uint8_t>(len));

    bool ok = (received == len);
    for (size_t i = 0; i < received; ++i) {
        buf[i] = Wire.read();
    }

    giveMutex();

    if (!ok) {
        Serial.printf("[I2CManager] readBytes: short read addr=0x%02X reg=0x%02X"
                      " requested=%u received=%u\n",
                      addr, reg, (unsigned)len, (unsigned)received);
    }
    return ok;
}

// ── Private helpers ────────────────────────────────────────────────────────

bool I2CManager::takeMutex()
{
    return xSemaphoreTake(mutex_,
                          pdMS_TO_TICKS(I2C_MUTEX_TIMEOUT_MS)) == pdTRUE;
}

void I2CManager::giveMutex()
{
    xSemaphoreGive(mutex_);
}
