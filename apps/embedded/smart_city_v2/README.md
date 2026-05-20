# Smart City v2 — Embedded Firmware

Unified embedded firmware for the Smart City project, running on an **ESP32-S3** via PlatformIO.  
All new embedded features belong here. The `legacy/` folder is archived for reference only.

---

## Table of Contents

- [Hardware](#hardware)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [System Architecture](#system-architecture)
- [The CityModule Interface](#the-citymodule-interface)
- [Adding Your Module](#adding-your-module)
- [I2C Address Map](#i2c-address-map)
- [Hardware Test Environment](#hardware-test-environment)
- [Rules & Conventions](#rules--conventions)

---

## Hardware

| Item | Detail |
|:-----|:-------|
| MCU | ESP32-S3 DevKitM-1 |
| Framework | Arduino (FreeRTOS available) |
| Toolchain | PlatformIO |
| I2C Bus | SDA = GPIO 8, SCL = GPIO 9, 400 kHz |
| Serial baud | 115200 |

---

## Project Structure

```
smart_city_v2/
├── platformio.ini          # Build config — two environments: main + hwtest
├── include/
│   └── city_config.h       # Shared pins, I2C address map, global constants
├── src/
│   ├── main.cpp            # Production entry point — wires all modules together
│   └── main_hwtest.cpp     # Hardware test entry point (compiled separately)
└── lib/
    ├── I2CManager/         # Thread-safe shared I2C bus wrapper
    └── BridgeSystem/       # Bridge module — see its own README for details
        └── README.md
```

Each city component (bridge, traffic lights, parking, ...) lives in its own folder under `lib/`. The `BridgeSystem` is the first completed module and acts as the reference implementation.

---

## Getting Started

### Prerequisites
- [PlatformIO](https://platformio.org/) installed (VS Code extension recommended)
- USB-C cable to the ESP32-S3

### Flash production firmware

```bash
pio run -e esp32-s3-devkitm-1 -t upload
pio device monitor
```

### Flash hardware test build

```bash
pio run -e hwtest -t upload
```

The two build environments compile different entry points. You cannot mix them in one build — this is intentional.

---

## System Architecture

The firmware is designed as a collection of independent city modules, each running in its own FreeRTOS task and communicating over a shared I2C bus.

```
main.cpp
 ├── I2CManager              shared, mutex-protected I2C bus
 ├── FreeRTOS Task: BridgeTask  @ 100 Hz
 │    └── BridgeController   (implements CityModule)
 ├── FreeRTOS Task: TrafficTask @ 100 Hz   ← your module goes here
 │    └── TrafficController  (implements CityModule)
 └── FreeRTOS Task: ParkingTask @ 100 Hz   ← or here
      └── ParkingController  (implements CityModule)
```

`main.cpp` is the only place tasks are created. `loop()` is intentionally idle — all work happens inside FreeRTOS tasks.

Each task calls `begin()` once on startup, then loops `update()` at its target rate:

```cpp
void myModuleTask(void*) {
    myModule.begin();
    while (true) {
        myModule.update();
        vTaskDelay(pdMS_TO_TICKS(10)); // 100 Hz
    }
}
```

### I2CManager

`I2CManager` is a FreeRTOS mutex-protected wrapper around Arduino's `Wire`. It is constructed once in `main.cpp` and passed by reference to every module that needs the bus.

**Never call `Wire` directly.** Bypassing `I2CManager` causes bus corruption when multiple tasks are running.

```cpp
// Scanning the bus (useful during bringup)
auto found = i2c.scan(); // returns std::vector<uint8_t> of responding addresses

// Writing to a peripheral
i2c.writeBytes(address, reg, data, len);

// Reading from a peripheral
i2c.readBytes(address, reg, buf, len);
```

---

## The CityModule Interface

Every module must implement this interface, defined in `lib/BridgeSystem/CityModule.h`:

```cpp
class CityModule {
public:
    virtual bool begin() = 0;        // one-time hardware init; return false on failure
    virtual void update() = 0;       // called every tick from the FreeRTOS task
    virtual const char* getName() = 0;
};
```

This is the contract the whole system is built on. Stick to it and your module will slot in cleanly.

---

## Adding Your Module

### 1. Register your I2C address range in `include/city_config.h`

```cpp
constexpr uint8_t MY_MODULE_BASE_ADDR = 0x20; // pick a free range from the table below
```

### 2. Create a library folder under `lib/`

```
lib/
└── TrafficSystem/
    ├── TrafficController.h/.cpp   # implements CityModule; owns subsystem objects
    ├── TrafficSignaling.h/.cpp    # hardware abstraction for lights/outputs
    └── TrafficSensors.h/.cpp      # hardware abstraction for sensors/inputs
```

Keep hardware GPIO access inside the leaf classes (`Signaling`, `Sensors`, `Motion`). The controller should only call semantic methods — `isDetected()`, `setRed()`, `moveBy(n)` — never `digitalRead` / `digitalWrite` directly.

### 3. Implement CityModule

```cpp
#include "BridgeSystem/CityModule.h"
#include "I2CManager/I2CManager.h"

class TrafficController : public CityModule {
public:
    TrafficController(I2CManager& bus, uint8_t address);
    bool begin() override;
    void update() override;
    const char* getName() override { return "TrafficController"; }
private:
    I2CManager& _bus;
    uint8_t _address;
    // state machine enum, millis() timer, subsystem objects...
};
```

### 4. Wire it up in `src/main.cpp`

```cpp
#include "TrafficSystem/TrafficController.h"

TrafficController trafficNode(i2c, TRAFFIC_BASE_ADDR);

void trafficTask(void*) {
    trafficNode.begin();
    while (true) {
        trafficNode.update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// inside setup():
xTaskCreate(trafficTask, "TrafficTask", 4096, nullptr, 1, nullptr);
```

### 5. Add a hardware test

Add a test case to `src/main_hwtest.cpp` following the existing `ActiveTest` enum + FSM pattern — one character key, one non-blocking test FSM, PASS/FAIL printed to Serial. See [BridgeSystem/README.md](lib/BridgeSystem/README.md) for an example.

---

## I2C Address Map

Defined in `include/city_config.h`. **Register your range here before using any address.**

| Range | Module |
|:------|:-------|
| `0x10–0x1F` | Bridge |
| `0x20–0x2F` | Traffic lights *(reserved)* |
| `0x30–0x3F` | Parking *(reserved)* |

---

## Hardware Test Environment

The `hwtest` build (`src/main_hwtest.cpp`) is a standalone interactive wiring validator. Flash it to confirm your hardware before running the full firmware.

Open the serial monitor at 115200 baud and press the key for the test you want to run. Only one test runs at a time — a running test blocks further input until it finishes.

See [BridgeSystem/README.md](lib/BridgeSystem/README.md) for the full list of existing bridge tests and how to structure your own.

---

## Rules & Conventions

| Rule | Why |
|:-----|:----|
| Never call `delay()` in production code | Blocks the FreeRTOS task and starves other modules |
| Never call `Wire` directly | `I2CManager` holds the mutex; bypassing it corrupts the bus |
| GPIO access stays inside leaf hardware classes | Keeps controllers clean and pin assignments centralized |
| All waits are `millis()`-based FSMs | Non-blocking, composable, and easy to debug |
| One FreeRTOS task per module | Keeps timing independent between modules |
| Register your I2C address in `city_config.h` | Prevents address collisions across teams |
