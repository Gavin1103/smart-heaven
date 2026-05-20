# Connecting an Embedded Device to the API

This guide walks through everything needed to register a new embedded device and have it successfully authenticate and
post data to the Smart Heaven API.

---

## Overview

Embedded devices communicate with the backend over HTTP using **API key authentication**. The flow has two sides:

1. **Setup (done once by an admin)** — register the device in the database and generate an API key.
2. **Runtime (done by the firmware)** — attach the API key to every HTTP request via the `X-Api-Key` header.

**Production API base URL:** `https://smart-heaven-production.up.railway.app`

**Swagger (interactive docs):** `https://smart-heaven-production.up.railway.app/docs`

The two device-facing endpoints are:

| Endpoint                  | Method | Purpose                                                   |
|:--------------------------|:-------|:----------------------------------------------------------|
| `/telemetry`              | `POST` | Report an event (state transition, fault, sensor reading) |
| `/device-state/:deviceId` | `PUT`  | Push the current live state of the device                 |

Both endpoints accept either a **JWT Bearer token** (for dashboard/admin use) or the device **API key** (`X-Api-Key`
header).

---

## Step 1 — Register the Device (Admin)

An admin user must create the device record before the firmware can authenticate. This is done with the devices API —
JWT + `ADMIN` role required.

**Request:**

```http
POST https://smart-heaven-production.up.railway.app/devices
Authorization: Bearer <admin-jwt>
Content-Type: application/json

{
  "name": "Bridge - Tile 1",
  "code": "bridge-01",
  "description": "Railway bridge on tile 1",
  "hardwareTypeKey": "RAILWAY_BARRIER",
  "locationName": "Tile 1"
}
```

| Field             | Notes                                                                                                                                    |
|:------------------|:-----------------------------------------------------------------------------------------------------------------------------------------|
| `code`            | Short identifier — **must be unique**. This is also the prefix of the API key. Choose something stable (e.g. `bridge-01`, `traffic-t3`). |
| `hardwareTypeKey` | Must match an existing hardware type key in the database.                                                                                |

The response includes the device `id` — note it down, you'll need it in the next step.

---

## Step 2 — Generate an API Key (Admin)

```http
POST https://smart-heaven-production.up.railway.app/devices/:id/api-key
Authorization: Bearer <admin-jwt>
```

Replace `:id` with the numeric `id` returned in Step 1.

**The API key is only returned once.** Store it securely immediately — the backend only stores a bcrypt hash and cannot
recover it.

The key format is:

```
<deviceCode>.<32-byte-hex-secret>
```

Example:

```
bridge-01.a3f8c2e1d...
```

---

## Step 3 — Configure the Firmware

All network configuration lives in `apps/embedded/smart_city_v2/include/telemetry_config.h`. Override values at build
time via `platformio.ini` `build_flags` rather than editing the header — this keeps credentials out of source control.

**`platformio.ini`:**

```ini
[env:esp32-s3-devkitm-1]
build_flags =
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
    -I ${PROJECT_INCLUDE_DIR}
    -DTELEMETRY_WIFI_SSID='"YourNetworkName"'
    -DTELEMETRY_WIFI_PASSWORD='"YourPassword"'
    -DTELEMETRY_DEVICE_ID='"bridge-01"'
    -DTELEMETRY_ENDPOINT='"https://smart-heaven-production.up.railway.app/telemetry"'
    -DTELEMETRY_API_KEY='"bridge-01.a3f8c2e1d..."'
```

!!! warning
Never commit real credentials. Add `platformio.ini` local overrides to `.gitignore`, or use PlatformIO's `extra_scripts`
to load a `.env` file.

| Macro                     | Purpose                                           |
|:--------------------------|:--------------------------------------------------|
| `TELEMETRY_WIFI_SSID`     | WiFi network name                                 |
| `TELEMETRY_WIFI_PASSWORD` | WiFi password                                     |
| `TELEMETRY_DEVICE_ID`     | Must match the `code` you used in Step 1          |
| `TELEMETRY_ENDPOINT`      | Full URL to `POST /telemetry` on the backend host |
| `TELEMETRY_API_KEY`       | The full API key returned in Step 2               |

Add `TELEMETRY_API_KEY` to `telemetry_config.h` with a placeholder default:

```cpp
#ifndef TELEMETRY_API_KEY
#define TELEMETRY_API_KEY  ""
#endif
```

---

## Step 4 — Send the API Key in Every Request

The `TelemetryManager` must attach the `X-Api-Key` header to every HTTP POST. In `TelemetryManager.cpp`, add the header
alongside `Content-Type`:

```cpp
HTTPClient http;
http.begin(_endpoint);
http.addHeader("Content-Type", "application/json");
http.addHeader("X-Api-Key", TELEMETRY_API_KEY);   // <-- add this line
http.setTimeout(TELEMETRY_HTTP_TIMEOUT_MS);
```

The same header is required for device-state updates if you make those from firmware.

---

## Telemetry Payload Reference

`POST /telemetry` — authenticated with `X-Api-Key`.

```json
{
  "deviceCode": "bridge-01",
  "eventType": "BRIDGE_OPENED",
  "state": "Open",
  "level": "INFO",
  "message": "Bridge opened successfully",
  "data": {
    "fromState": "Opening",
    "toState": "Open",
    "sensors": {
      "boatDetected": true,
      "stepperPos": 6400
    }
  }
}
```

| Field        | Type   | Required | Notes                                                        |
|:-------------|:-------|:---------|:-------------------------------------------------------------|
| `deviceCode` | string | yes      | Must match the device's `code` in the database               |
| `eventType`  | string | yes      | Free-form event label, e.g. `BRIDGE_OPENED`, `FAULT_ENTERED` |
| `state`      | string | yes      | Current state name at the time of the event                  |
| `level`      | enum   | yes      | One of `INFO`, `WARN`, `ERROR`                               |
| `message`    | string | yes      | Human-readable description                                   |
| `data`       | object | no       | Any extra JSON — sensor snapshots, transition details, etc.  |

A successful post returns **HTTP 204 No Content**.

The firmware's `TelemetryManager` builds and sends this payload automatically via `enqueue()`. The `eventType` string
maps from the `EventCode` enum — see `TelemetryManager.cpp` for the full list.

---

## Device State Payload Reference

`PUT /device-state/:deviceId` — authenticated with `X-Api-Key`.

The `:deviceId` is the numeric database ID assigned in Step 1 (not the string `code`).

```json
{
  "state": "Open",
  "statusMessage": "Bridge is fully open",
  "data": {
    "stepperPos": 6400
  }
}
```

| Field           | Type   | Required | Notes                         |
|:----------------|:-------|:---------|:------------------------------|
| `state`         | string | yes      | Current state label           |
| `statusMessage` | string | no       | Optional human-readable note  |
| `data`          | object | no       | Any additional state metadata |

This endpoint is an upsert — it creates the state row on first call and overwrites it on subsequent calls. The dashboard
reads this to display live device status.

---

## Backend URL

The production API is hosted on Railway and publicly reachable — no local network setup required:

| Environment       | Base URL                                         |
|:------------------|:-------------------------------------------------|
| Production        | `https://smart-heaven-production.up.railway.app` |
| Local development | `http://<host-ip>:3000`                          |

For production, set `TELEMETRY_ENDPOINT` to:

```
https://smart-heaven-production.up.railway.app/telemetry
```

The ESP32 must be able to reach the internet over WiFi. For local development the ESP32 and the host running the API
must be on the same network; find the host IP with `ipconfig getifaddr en0` (macOS) or `ip route get 1` (Linux).

The full Swagger reference is available at `https://smart-heaven-production.up.railway.app/docs` — useful for testing
requests manually before flashing firmware.

---

## Troubleshooting

| Symptom                             | Likely cause                             | Fix                                                                                                           |
|:------------------------------------|:-----------------------------------------|:--------------------------------------------------------------------------------------------------------------|
| `[Telemetry] POST failed` in Serial | WiFi not connected or wrong IP           | Check SSID/password; verify host IP in `TELEMETRY_ENDPOINT`                                                   |
| HTTP 401                            | Missing or invalid `X-Api-Key`           | Verify key matches exactly (copy-paste, no whitespace); re-generate if needed                                 |
| HTTP 404                            | Wrong device code in body                | `deviceCode` in the JSON must match the `code` registered in Step 1                                           |
| HTTP 400                            | Missing required field or bad enum value | Check `level` is one of `INFO`, `WARN`, `ERROR`; all required fields are present                              |
| `[Telemetry] WARN: queue full`      | Events firing faster than WiFi can send  | Normal during bursts; events are dropped. Reduce event rate or increase `QUEUE_DEPTH` in `TelemetryManager.h` |
| `[Bridge] FATAL: begin() failed`    | I2C wiring fault                         | Run the `hwtest` build to verify MCP23017 / PCA9685 respond before flashing production firmware               |

---

## Summary Checklist

- [ ] Admin creates device via `POST /devices` with a unique `code`
- [ ] Admin generates API key via `POST /devices/:id/api-key` and stores it securely
- [ ] `platformio.ini` `build_flags` set: SSID, password, device ID, endpoint URL, API key
- [ ] `telemetry_config.h` has `TELEMETRY_API_KEY` macro defined
- [ ] `TelemetryManager.cpp` sends `X-Api-Key` header on every POST
- [ ] Device connects to WiFi and posts telemetry — Serial log shows `HTTP 204`
