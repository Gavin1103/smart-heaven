# Embedded Firmware Platform

The embedded side of Smart Heaven runs on a single unified firmware project: **`smart_city_v2`**, located at `apps/embedded/smart_city_v2/`. Everything in the legacy folder is archived prototypes — all active work belongs here.

---

## Why this stack?

| Choice | Reason |
|:-------|:-------|
| **ESP32-S3** | Dual-core, plenty of GPIO, native USB, and solid FreeRTOS support — enough headroom for all city modules on one board |
| **PlatformIO** | Reproducible builds, library management, and two named build environments (`main` + `hwtest`) in a single config file |
| **Arduino framework** | Familiar to the whole team; `Wire`, `Servo`, and most sensor libraries just work without custom HAL code |
| **FreeRTOS tasks** | Each city module runs independently — one task each, so a slow sensor in one module can't block another |

The short version: the stack was chosen to be learnable by the whole team while still giving us the real-time control a moving bridge demands.

---

## How the firmware is structured

Every city component (bridge, traffic lights, parking, …) is a **library** under `lib/`. Each library contains its own hardware abstraction and implements a shared interface called `CityModule`. `main.cpp` is the only place where tasks are created and wired together.

```
smart_city_v2/
├── platformio.ini          # Two environments: production + hardware test
├── include/
│   └── city_config.h       # Pins, I2C address map, shared constants
├── src/
│   ├── main.cpp            # Wires modules into FreeRTOS tasks
│   └── main_hwtest.cpp     # Interactive wiring validator (separate build)
└── lib/
    ├── I2CManager/         # Mutex-protected I2C bus wrapper
    └── BridgeSystem/       # First completed module — the reference implementation
```

### The CityModule interface

Every module implements the same three-method contract:

```cpp
class CityModule {
public:
    virtual bool begin() = 0;   // one-time hardware init
    virtual void update() = 0;  // called every tick (~100 Hz)
    virtual const char* getName() = 0;
};
```

This keeps `main.cpp` simple: it creates a task per module, calls `begin()` once, then loops `update()` on a 10 ms timer. New modules slot in without touching any existing code.

### I2CManager — why not use Wire directly?

Multiple FreeRTOS tasks running at the same time can all try to use the I2C bus simultaneously. Without protection, that corrupts the bus. `I2CManager` wraps `Wire` with a FreeRTOS mutex so only one task can transact at a time. **Never call `Wire` directly** — always go through `I2CManager`.

---

## Two build environments

| Environment | Entry point | Purpose |
|:------------|:------------|:--------|
| `esp32-s3-devkitm-1` | `src/main.cpp` | Full production firmware — all modules running as tasks |
| `hwtest` | `src/main_hwtest.cpp` | Interactive wiring validator — test individual sensors and actuators over Serial before running the full build |

The hardware test build exists because wiring faults are cheap to catch early and expensive to debug inside the full firmware. Flash `hwtest` first on any new hardware, verify everything responds, then switch to the production build.

---

## I2C address map

Defined in `include/city_config.h`. Register your module's range here before using any address — this is the single source of truth that prevents address collisions across teams.

| Range | Module |
|:------|:-------|
| `0x10–0x1F` | Bridge |
| `0x20–0x2F` | Traffic lights *(reserved)* |
| `0x30–0x3F` | Parking *(reserved)* |

---

## Adding a new module

The pattern is the same every time:

1. Register an I2C address range in `city_config.h`
2. Create a library folder under `lib/` with a controller class, a signaling class, and a sensors class
3. Implement the `CityModule` interface in the controller
4. Add a `xTaskCreate` call in `main.cpp`
5. Add hardware test cases to `main_hwtest.cpp`

The `BridgeSystem` library is the reference implementation — read its code and README before starting a new module.

---

## Rules that keep everything working

| Rule | Why it matters |
|:-----|:---------------|
| No `delay()` in production code | Blocks the entire FreeRTOS task, starving other work at that priority |
| No direct `Wire` calls | `I2CManager` owns the bus mutex — bypassing it causes corruption |
| GPIO only inside leaf hardware classes | Keeps controllers clean; pin changes are a one-file edit |
| All waits as `millis()`-based FSMs | Non-blocking, composable, and straightforward to debug over Serial |
| One task per module | Timing stays independent; one slow sensor can't affect unrelated modules |
