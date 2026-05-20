#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace telemetry {

// ── Event codes ───────────────────────────────────────────────────────────────
// Every bridge state transition enqueues a payload with one of these codes.
// Codes 0x02–0x0B are assigned to semantically significant transitions so the
// backend can filter without parsing state-name strings.
enum class EventCode : uint8_t {
    StateTransition   = 0x01,  // generic: any transition not covered below
    BoatDetected      = 0x02,  // IdleClosed → BoatDetected
    BoatCleared       = 0x03,  // BoatDetected → IdleClosed (timeout/abort)
    FaultEntered      = 0x04,  // any → Fault
    FaultRecovered    = 0x05,  // Fault → Homing (encoder button press)
    HomingComplete    = 0x06,  // Homing → IdleClosed
    BridgeOpened      = 0x07,  // Opening → Open
    BridgeClosed      = 0x08,  // Closing → RaiseBarriers
    ClosingAborted    = 0x09,  // Closing → Opening (obstacle reversed close)
    StartupValidated  = 0x0A,  // Startup → Homing
};

// ── Sensor snapshot ───────────────────────────────────────────────────────────
// Captured at the moment of the state transition for correlation.
struct SensorSnapshot {
    bool boatDetected;
    bool obstacleDetected;
    bool carDetected;
    bool reedClosed;
    int  stepperPos;
};

// ── Payload ───────────────────────────────────────────────────────────────────
// Copied into the FreeRTOS queue by value.  All const char* members point to
// string literals in .rodata — safe to transfer across tasks.
struct TelemetryPayload {
    EventCode      eventCode;
    const char*    fromState;   // stateToString() result; never nullptr
    const char*    toState;     // stateToString() result; never nullptr
    const char*    faultReason; // faultReasonToString() result; nullptr if not a fault
    SensorSnapshot sensors;
    uint32_t       uptimeMs;
};

// ── TelemetryManager ─────────────────────────────────────────────────────────
// Owns a FreeRTOS queue + background task that serialises payloads to JSON and
// HTTP-POSTs them to the backend.  enqueue() is non-blocking and safe to call
// from any FreeRTOS task (including the 100 Hz bridge task).
class TelemetryManager {
public:
    TelemetryManager(const char* endpoint, const char* deviceId);

    // Starts the background FreeRTOS task and queue.  Call once from setup().
    bool begin();

    // Pushes a payload onto the queue.  Drops silently if the queue is full.
    void enqueue(const TelemetryPayload& payload);

private:
    static void telemetryTask(void* param);
    void        sendPayload(const TelemetryPayload& payload);
    static const char* eventCodeToString(EventCode code);

    const char*   _endpoint;
    const char*   _deviceId;
    QueueHandle_t _queue;

    static constexpr size_t QUEUE_DEPTH     = 16;
    static constexpr size_t TASK_STACK_SIZE = 8192;
    static constexpr int    TASK_PRIORITY   = 2;
};

} // namespace telemetry
