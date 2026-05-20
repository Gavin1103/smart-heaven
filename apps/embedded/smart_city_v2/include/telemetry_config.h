#pragma once

// ── WiFi Credentials ─────────────────────────────────────────────────────────
// Override at build time via platformio.ini build_flags:
//   build_flags = -DTELEMETRY_WIFI_SSID='"MyNetwork"'
//                 -DTELEMETRY_WIFI_PASSWORD='"MyPassword"'
#ifndef TELEMETRY_WIFI_SSID
#define TELEMETRY_WIFI_SSID      "YOUR_WIFI_SSID"
#endif

#ifndef TELEMETRY_WIFI_PASSWORD
#define TELEMETRY_WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"
#endif

// ── Device Identity ───────────────────────────────────────────────────────────
#ifndef TELEMETRY_DEVICE_ID
#define TELEMETRY_DEVICE_ID      "bridge-01"
#endif

// ── Backend Endpoint ─────────────────────────────────────────────────────────
// POST <endpoint>  →  expects 2xx on success
#ifndef TELEMETRY_ENDPOINT
#define TELEMETRY_ENDPOINT       "http://192.168.1.100:3000/api/v1/telemetry"
#endif

// ── Connection Timeouts ───────────────────────────────────────────────────────
#define TELEMETRY_HTTP_TIMEOUT_MS   5000U
#define TELEMETRY_WIFI_TIMEOUT_MS  10000U
