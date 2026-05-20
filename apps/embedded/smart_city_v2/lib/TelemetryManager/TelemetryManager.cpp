#include "TelemetryManager.h"
#include "telemetry_config.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

namespace telemetry {

// static
const char* TelemetryManager::eventCodeToString(EventCode code) {
    switch (code) {
        case EventCode::StateTransition:  return "STATE_TRANSITION";
        case EventCode::BoatDetected:     return "BOAT_DETECTED";
        case EventCode::BoatCleared:      return "BOAT_CLEARED";
        case EventCode::FaultEntered:     return "FAULT_ENTERED";
        case EventCode::FaultRecovered:   return "FAULT_RECOVERED";
        case EventCode::HomingComplete:   return "HOMING_COMPLETE";
        case EventCode::BridgeOpened:     return "BRIDGE_OPENED";
        case EventCode::BridgeClosed:     return "BRIDGE_CLOSED";
        case EventCode::ClosingAborted:   return "CLOSING_ABORTED";
        case EventCode::StartupValidated: return "STARTUP_VALIDATED";
        default:                          return "UNKNOWN";
    }
}

TelemetryManager::TelemetryManager(const char* endpoint, const char* deviceId)
    : _endpoint(endpoint), _deviceId(deviceId), _queue(nullptr) {}

bool TelemetryManager::begin() {
    _queue = xQueueCreate(QUEUE_DEPTH, sizeof(TelemetryPayload));
    if (!_queue) {
        Serial.println("[Telemetry] ERROR: queue creation failed");
        return false;
    }
    xTaskCreate(telemetryTask, "TelemetryTask", TASK_STACK_SIZE, this, TASK_PRIORITY, nullptr);
    Serial.println("[Telemetry] Task started");
    return true;
}

void TelemetryManager::enqueue(const TelemetryPayload& payload) {
    if (!_queue) return;
    Serial.printf("[Telemetry] %s  %s → %s\n",
                  eventCodeToString(payload.eventCode),
                  payload.fromState,
                  payload.toState);
    if (xQueueSend(_queue, &payload, 0) != pdTRUE) {
        Serial.println("[Telemetry] WARN: queue full — event dropped");
    }
}

// static — runs as a dedicated FreeRTOS task
void TelemetryManager::telemetryTask(void* param) {
    auto* self = static_cast<TelemetryManager*>(param);
    TelemetryPayload payload;
    for (;;) {
        if (xQueueReceive(self->_queue, &payload, portMAX_DELAY) == pdTRUE) {
            self->sendPayload(payload);
        }
    }
}

void TelemetryManager::sendPayload(const TelemetryPayload& p) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.printf("[Telemetry] WiFi offline — dropping %s\n",
                      eventCodeToString(p.eventCode));
        return;
    }

    // Build JSON payload (~200 bytes typical)
    JsonDocument doc;
    doc["device_id"]  = _deviceId;
    doc["uptime_ms"]  = p.uptimeMs;
    doc["event"]      = eventCodeToString(p.eventCode);
    doc["event_code"] = static_cast<uint8_t>(p.eventCode);
    doc["from_state"] = p.fromState;
    doc["to_state"]   = p.toState;
    if (p.faultReason) doc["fault_reason"] = p.faultReason;

    JsonObject sensors          = doc["sensors"].to<JsonObject>();
    sensors["boat_detected"]     = p.sensors.boatDetected;
    sensors["obstacle_detected"] = p.sensors.obstacleDetected;
    sensors["car_detected"]      = p.sensors.carDetected;
    sensors["reed_closed"]       = p.sensors.reedClosed;
    sensors["stepper_pos"]       = p.sensors.stepperPos;

    char body[512];
    serializeJson(doc, body, sizeof(body));

    HTTPClient http;
    http.begin(_endpoint);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(TELEMETRY_HTTP_TIMEOUT_MS);

    int httpCode = http.POST(body);
    if (httpCode > 0) {
        Serial.printf("[Telemetry] POST %s → HTTP %d\n",
                      eventCodeToString(p.eventCode), httpCode);
    } else {
        Serial.printf("[Telemetry] POST failed (%s): %s\n",
                      eventCodeToString(p.eventCode),
                      http.errorToString(httpCode).c_str());
    }
    http.end();
}

} // namespace telemetry
