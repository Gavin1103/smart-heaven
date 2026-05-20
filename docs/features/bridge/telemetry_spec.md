# Bridge Telemetry Specification

This document defines the JSON schema, HTTP endpoint contract, and complete event-code catalogue for the bridge node's remote telemetry system. The backend team should implement a receiver that accepts these payloads and the embedded team should treat this document as the single source of truth for payload structure.

---

## HTTP Endpoint

| Property    | Value |
|-------------|-------|
| Method      | `POST` |
| Path        | `/api/v1/telemetry` |
| Content-Type | `application/json` |
| Auth        | None (add bearer token when deployed outside the local network) |
| Expected response | `2xx` — body is ignored by the device |
| Failure behaviour | Device logs the HTTP error and discards the payload; **no retry** |
| Timeout     | 5 000 ms per request |

The endpoint is configured at build time via `include/telemetry_config.h`:

```cpp
#define TELEMETRY_ENDPOINT  "http://192.168.1.100:3000/api/v1/telemetry"
```

---

## JSON Schema

Every POST body is a single JSON object. All fields are always present unless marked *optional*.

```jsonc
{
  "device_id":   "bridge-01",        // string  — device identifier from TELEMETRY_DEVICE_ID
  "uptime_ms":   12345,              // uint32  — millis() at the moment of the transition
  "event":       "BOAT_DETECTED",    // string  — human-readable event name (see Event Codes)
  "event_code":  2,                  // uint8   — numeric event code (see Event Codes)
  "from_state":  "IdleClosed",       // string  — state machine state before the transition
  "to_state":    "BoatDetected",     // string  — state machine state after the transition
  "fault_reason": null,              // string | null — set only when event_code == 4 (FAULT_ENTERED)
  "sensors": {
    "boat_detected":     true,       // bool — HC-SR04 east OR west within threshold
    "obstacle_detected": false,      // bool — KY-032 IR sensor active (under-bridge clearance)
    "car_detected":      false,      // bool — HX711 load cell above CAR_THRESHOLD_G (90 g)
    "reed_closed":       true,       // bool — KY-025 reed switch triggered (bridge seated)
    "stepper_pos":       0           // int  — encoder step count relative to last reset
  }
}
```

### Field Details

| Field | Type | Description |
|-------|------|-------------|
| `device_id` | string | Unique identifier for this bridge node. Matches `TELEMETRY_DEVICE_ID` in firmware config. |
| `uptime_ms` | uint32 | `millis()` timestamp at the moment `transitionTo()` was called. Wraps after ~49 days. |
| `event` | string | Human-readable name of the event (see table below). Safe for display; do not use for filtering — use `event_code`. |
| `event_code` | uint8 | Numeric code for efficient filtering and routing. Stable across firmware versions. |
| `from_state` | string | State machine state immediately before this transition. |
| `to_state` | string | State machine state immediately after this transition. |
| `fault_reason` | string \| null | Present only when `event_code == 4`. One of the Fault Reason values listed below. `null` otherwise. |
| `sensors.boat_detected` | bool | `true` if the HC-SR04 east **or** west sonar reads a target within 500 mm. Requires 2 consecutive confirmations. |
| `sensors.obstacle_detected` | bool | `true` if the KY-032 IR sensor detects an object under the bridge deck. Requires 2 consecutive confirmations. |
| `sensors.car_detected` | bool | `true` if the HX711 load cell reads > 90 g above the tared baseline. Requires 3 consecutive confirmations. |
| `sensors.reed_closed` | bool | `true` if the KY-025 reed switch is triggered (magnet present = bridge fully seated). |
| `sensors.stepper_pos` | int | Encoder step count since the last `resetEncoderPosition()` call. Reset at the start of every homing, opening, or closing motion. |

---

## Event Codes

Every state transition fires exactly one event. The code is chosen by `toEventCode(from, to)` in `BridgeController.cpp`.

| Code | Name | Trigger condition |
|------|------|-------------------|
| `0x01` | `STATE_TRANSITION` | Any transition not covered by a more specific code below |
| `0x02` | `BOAT_DETECTED` | `IdleClosed` → `BoatDetected` — sonar confirmed boat approach |
| `0x03` | `BOAT_CLEARED` | `BoatDetected` → `IdleClosed` — boat disappeared before 1 s confirmation window |
| `0x04` | `FAULT_ENTERED` | Any → `Fault` — unrecoverable condition detected; see `fault_reason` |
| `0x05` | `FAULT_RECOVERED` | `Fault` → `Homing` — operator pressed the encoder button to recover |
| `0x06` | `HOMING_COMPLETE` | `Homing` → `IdleClosed` — bridge successfully seated and tared |
| `0x07` | `BRIDGE_OPENED` | `Opening` → `Open` — full rotation complete, reed left closed position |
| `0x08` | `BRIDGE_CLOSED` | `Closing` → `RaiseBarriers` — reed triggered, bridge has reseated |
| `0x09` | `CLOSING_ABORTED` | `Closing` → `Opening` — IR obstacle detected mid-close; reversed to open |
| `0x0A` | `STARTUP_VALIDATED` | `Startup` → `Homing` — sensor state validated, starting homing sequence |

### Fault Reasons

`fault_reason` is populated when `event_code == 0x04` (`FAULT_ENTERED`):

| Value | Trigger |
|-------|---------|
| `HomingTimeout` | Reed switch not triggered within 30 s during homing |
| `OpeningTimeout` | Full rotation not reached within 40 s during opening |
| `ClosingTimeout` | Reed switch not triggered within 40 s during closing |
| `InvalidStartupState` | Boat + obstacle both active simultaneously at startup |
| `SafetyCheckFailed` | Car on deck for more than 30 s during `WaitBridgeClear` |
| `MotorStall` | Full rotation completed but reed switch never left closed — motor stall detected |

---

## State Name Reference

All possible `from_state` / `to_state` values:

| State name | Meaning |
|------------|---------|
| `Startup` | Initial validation of sensor state |
| `Homing` | Motor driving toward closed position until reed triggers |
| `IdleClosed` | Bridge seated and idle, waiting for a boat |
| `BoatDetected` | Debounce window (1 s) after sonar detects a boat |
| `WarningRoadTraffic` | Flashing yellow road lights for 3 s |
| `WaitBridgeClear` | Waiting for the load cell to confirm the deck is empty |
| `LowerBarriers` | Servo barriers lowering sequentially |
| `WaterPrepRedGreen` | Red + green water light signal for 2 s |
| `Opening` | Motor rotating bridge to full-open position |
| `Open` | Bridge fully open; waiting 30 s minimum |
| `ClosingCheckClearance` | Final 2 s safety window before closing |
| `Closing` | Motor rotating bridge back to closed position |
| `RaiseBarriers` | Servo barriers raising sequentially |
| `Fault` | Halted; operator must press encoder button to recover |

---

## Example Payloads

### Boat detected

```json
{
  "device_id": "bridge-01",
  "uptime_ms": 45231,
  "event": "BOAT_DETECTED",
  "event_code": 2,
  "from_state": "IdleClosed",
  "to_state": "BoatDetected",
  "fault_reason": null,
  "sensors": {
    "boat_detected": true,
    "obstacle_detected": false,
    "car_detected": false,
    "reed_closed": true,
    "stepper_pos": 0
  }
}
```

### Bridge fully opened

```json
{
  "device_id": "bridge-01",
  "uptime_ms": 98450,
  "event": "BRIDGE_OPENED",
  "event_code": 7,
  "from_state": "Opening",
  "to_state": "Open",
  "fault_reason": null,
  "sensors": {
    "boat_detected": true,
    "obstacle_detected": false,
    "car_detected": false,
    "reed_closed": false,
    "stepper_pos": 6400
  }
}
```

### Fault — homing timeout

```json
{
  "device_id": "bridge-01",
  "uptime_ms": 30421,
  "event": "FAULT_ENTERED",
  "event_code": 4,
  "from_state": "Homing",
  "to_state": "Fault",
  "fault_reason": "HomingTimeout",
  "sensors": {
    "boat_detected": false,
    "obstacle_detected": false,
    "car_detected": false,
    "reed_closed": false,
    "stepper_pos": -4200
  }
}
```

### Closing aborted by obstacle

```json
{
  "device_id": "bridge-01",
  "uptime_ms": 187320,
  "event": "CLOSING_ABORTED",
  "event_code": 9,
  "from_state": "Closing",
  "to_state": "Opening",
  "fault_reason": null,
  "sensors": {
    "boat_detected": false,
    "obstacle_detected": true,
    "car_detected": false,
    "reed_closed": false,
    "stepper_pos": -3100
  }
}
```

---

## Backend Implementation Notes

- **Idempotency:** The device does not retry failed requests. A missing payload means a connectivity gap, not a duplicate. Do not implement deduplication based on uptime.
- **Uptime wraps:** `uptime_ms` is `uint32_t` and wraps after ~49.7 days. If continuous session tracking is needed, combine `device_id` + `uptime_ms` + a session counter (not yet implemented).
- **Ordering:** Events are enqueued in-order and sent sequentially from a single FreeRTOS task. TCP ordering within a single connection is preserved; across reconnects it is not.
- **Offline gaps:** When WiFi is disconnected the device drops events and logs a warning locally. The queue holds up to 16 events; if the network outage outlasts the queue depth, the oldest events are silently discarded.
- **`STATE_TRANSITION` catch-all:** Any transition pair not listed in the Event Codes table above fires `STATE_TRANSITION` (0x01). Use `from_state` + `to_state` to distinguish them.
