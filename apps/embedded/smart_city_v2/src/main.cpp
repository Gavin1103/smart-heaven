#include <Arduino.h>
#include <WiFi.h>
#include "I2CManager.h"
#include "city_config.h"
#include "telemetry_config.h"
#include "TelemetryManager.h"
#include "BridgeController.h"

// Single shared I2C manager
static I2CManager i2c(I2C_SDA_PIN, I2C_SCL_PIN, 400000);

// Remote telemetry — POST JSON events to the backend over WiFi
static telemetry::TelemetryManager gTelemetry(TELEMETRY_ENDPOINT, TELEMETRY_DEVICE_ID);

// Bridge state machine wired to telemetry so every state change is reported
bridge::BridgeController bridgeNode(i2c, 0x10, &gTelemetry);

void bridgeTask(void *pvParameters) {
    if (!bridgeNode.begin()) {
        // PCB not responding (MCP23017 / PCA9685 missing or mis-addressed).
        // Running update() over a dead bus would drive a blind control loop,
        // so halt here instead and let the error log stand.
        Serial.println("[Bridge] FATAL: begin() failed — bridge task halted");
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    for(;;) {
        bridgeNode.update();
        vTaskDelay(pdMS_TO_TICKS(10)); // 100 Hz logic rate
    }
}

static void connectWiFi() {
    Serial.printf("[WiFi] Connecting to \"%s\"\n", TELEMETRY_WIFI_SSID);
    // Reconnect automatically if the link drops later — otherwise telemetry is
    // lost permanently after the first disconnect.
    WiFi.setAutoReconnect(true);
    WiFi.begin(TELEMETRY_WIFI_SSID, TELEMETRY_WIFI_PASSWORD);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < TELEMETRY_WIFI_TIMEOUT_MS) {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Connected — IP: %s\n",
                      WiFi.localIP().toString().c_str());
    } else {
        Serial.println("[WiFi] WARN: connection failed — telemetry will be offline");
    }
}

void setup() {
    Serial.begin(115200);
    i2c.begin();

    connectWiFi();
    gTelemetry.begin();

    // Launch bridge logic as a separate thread
    xTaskCreate(bridgeTask, "BridgeTask", 4096, NULL, 1, NULL);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
