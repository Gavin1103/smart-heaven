# Bridge Module — Development Roadmap

> **Owner:** Rocco Reus
> **Last updated:** 2026-04-23
> **Status:** Active development — Phase 3 telemetry shipped, PCB integration next

---

## Where We Are Now

The Rust/Embassy logic has been fully ported into a modular C++ BridgeSystem library running on ESP32-S3.
The 14-state FSM, all sensor drivers, motor control, and LED signaling are working and tested.
The bridge runs as a standalone autonomous node at 100 Hz via a FreeRTOS task.

Phase 3 (telemetry) is now implemented. Every FSM state transition is enqueued into a FreeRTOS queue
and HTTP-POSTed to the backend by a dedicated background task. The full payload schema and event-code
catalogue are documented in [`telemetry_spec.md`](./telemetry_spec.md).

See [`bridge_firmware.md`](./bridge_firmware.md) for the current implementation details.

---

## Roadmap

### Phase 1 — PCB Hardware Integration (I2C Peripheral Replacement)

**Goal:** Replace direct GPIO wiring with a dedicated PCB containing an MCP23017 (I/O expander) and PCA9685PW (PWM driver), connected to all bridge components. The bridge PCB becomes a self-contained node — plug it in, assign an I2C address, done.

**Why:** Enables clean node scalability. A second bridge, a third — each gets its own PCB with no wiring changes to the main controller.

**Work items:**

- [ ] Swap `BridgeSensors` GPIO reads (digital inputs: KY-032, KY-025, encoder) to MCP23017 via I2C
- [ ] Swap `BridgeSignaling` shift-register output to MCP23017 GPIO bank (Q0–Q7 → MCP outputs)
- [ ] Swap `BridgeMotion` servo PWM to PCA9685PW channels (replaces direct `analogWrite` / ledc)
- [ ] Keep stepper (28BYJ-48) on direct GPIO or route through MCP23017 outputs — evaluate step-rate latency over I2C
- [ ] Wire HX711 (SPI-like protocol) through MCP23017 or keep direct — HX711 is timing-sensitive, direct preferred
- [ ] Update `BridgeController::begin()` to initialise MCP23017 and PCA9685PW on startup
- [ ] Implement `I2CManager` integration in `BridgeController` (the ref is stored but unused)
- [ ] Address scheme: bridge node fixed at `0x10` (MCP23017), `0x40` (PCA9685PW) by default; PCB jumpers select address for multi-bridge setups
- [ ] Validate power budget on PCB — stepper + servo mutual exclusion rule must still hold on the new layout
- [ ] Update [`bridge_components_list.md`](./bridge_components_list.md) and [`bridge_wiring_and_power.md`](./bridge_wiring_and_power.md) with PCB pinout

**Definition of done:** Bridge operates identically to today, but all IO goes through the PCB over I2C. No jumper wires between ESP32-S3 and bridge components.

---

### Phase 2 — Water Traffic Signaling Fix (Simultaneous Red + Green)

**Goal:** Fix the known gap in `BridgeSignaling` where the water channel cannot output Red and Green simultaneously during the `WaterPrepRedGreen` state.

**Background:** Dutch nautical standards require a Red + Green combination signal to tell a vessel "prepare to pass — bridge is opening." Currently the shift register output map only supports Red OR Green on the water channel (Q3 / Q4), not both at once because the byte written to the register overwrites both bits.

**Work items:**

- [ ] Extend `BridgeSignaling` API: add `setWaterPrepSignal()` method that sets both Q3 and Q4 high simultaneously
- [ ] Update `_writeByte()` logic to compose the correct bitmask for this state without clobbering other outputs
- [ ] Verify on hardware that both LEDs light — check current draw does not exceed shift register output limit per pin (35 mA max per output, 70 mA total for SN74HC595)
- [ ] Update `BridgeController` `WaterPrepRedGreen` state to call `setWaterPrepSignal()` instead of current workaround
- [ ] If migrating to MCP23017 (Phase 1): MCP23017 output pins are independently addressable — this fix is trivial in that context

**Definition of done:** During `WaterPrepRedGreen`, both the red and green water LEDs are lit simultaneously.

---

### Phase 3 — Telemetry & State Logging (Network Uplink) ✓ Shipped

**Goal:** Every meaningful event on the bridge — state transitions, sensor triggers, fault conditions, motor actions — is logged and sent to the backend API in real time over Wi-Fi.

**Why:** Right now the bridge is a black box. Operators can only see the physical state. With logging, the backend dashboard can show bridge state history, detect recurring faults, and support maintenance scheduling.

**Events logged:**

| Event category | Examples |
|---|---|
| FSM state changes | `IdleClosed → BoatDetected`, `Opening → Open`, `* → Fault` |
| Sensor snapshot | boat, obstacle, car, reed, stepper position — captured at every transition |
| Fault events | Reason code (`HomingTimeout`, `MotorStall`, etc.) + sensor state at fault |
| Override events | Override applied, override lifted (Phase 4) |

**Work items:**

- [x] Add Wi-Fi connection management to `main.cpp` — blocking connect with 10 s timeout; bridge continues offline if WiFi unavailable
- [x] Implement an HTTP POST client for log events — `HTTPClient` with 5 s timeout, fires on each dequeued payload
- [x] Define the log payload schema (JSON) — documented in [`telemetry_spec.md`](./telemetry_spec.md); fields: `device_id`, `uptime_ms`, `event`, `event_code`, `from_state`, `to_state`, `fault_reason`, `sensors{}`
- [x] Create `TelemetryManager` class — non-blocking `enqueue()` called from `BridgeController::transitionTo()`; 10 specific `EventCode` values (see spec)
- [x] Queue log events in a FreeRTOS queue (depth 16) — separate priority-2 task drains and POSTs; bridge task is never blocked
- [x] Expose `device_id` as compile-time config (`TELEMETRY_DEVICE_ID` in `telemetry_config.h`; overridable via `build_flags`)
- [ ] Handle offline reconnect flush — currently events are dropped when WiFi is down; a persistent ring buffer with flush-on-reconnect is not yet implemented
- [ ] Add NTP time sync on boot — timestamps are `uptime_ms` (millis since boot), not wall-clock; NTP sync would enable absolute timestamps
- [ ] Backend: implement `POST /api/v1/telemetry` receiver — schema defined in [`telemetry_spec.md`](./telemetry_spec.md); coordinate with API team

**Implementation notes:**
- Endpoint, credentials, and device ID are set in `include/telemetry_config.h` — override at build time with `build_flags` to avoid committing real credentials
- HTTP was chosen over MQTT for simplicity; switch to MQTT pub/sub if the city network runs a broker (see Open Questions)
- `TelemetryManager` is injected into `BridgeController` as a nullable pointer — pass `nullptr` in `hwtest` builds to skip telemetry entirely

**Definition of done:** Every FSM state transition appears in the backend within 2 seconds. Faults are visible on the dashboard with reason and timestamp. *(Core device-side work complete; backend receiver and NTP sync still open.)*

---

### Phase 4 — Backend Override API (Remote Control)

**Goal:** The backend can send override commands to the bridge node. Two initial override types:

1. **Lock Closed** — Bridge is not allowed to open for a specified duration (e.g. VIP/royal convoy, emergency road closure)
2. **Force Open / Hold Open** — Bridge must stay open for a minimum duration (e.g. large vessel scheduled passage, maintenance)

The bridge FSM respects overrides while still enforcing all safety guards. An override is never a raw GPIO command — it is a constraint injected into the FSM.

**Override semantics:**

| Override type | FSM effect |
|---|---|
| `LOCK_CLOSED` | Bridge ignores boat detection triggers for the duration. If currently open, it closes at the next safe opportunity and then locks. |
| `HOLD_OPEN` | Bridge opens (or stays open) and suppresses the normal waterway-clear close trigger until duration expires or override is lifted. |
| `CLEAR` | Removes any active override, returns to normal autonomous operation. |

**Work items:**

- [ ] Backend: define `POST /api/bridge/{node_id}/override` endpoint — body: `{ type: "LOCK_CLOSED" | "HOLD_OPEN" | "CLEAR", duration_seconds: number | null }`
- [ ] Bridge: implement a lightweight HTTP server on the ESP32-S3 (or poll a backend endpoint on a schedule) to receive override commands
  - Polling approach is simpler and more robust for embedded — poll `GET /api/bridge/{node_id}/override/active` every 5 seconds
  - Push (backend POSTs to bridge IP) is lower latency but requires stable IP and firewall rules
- [ ] Add `OverrideState` struct to `BridgeController`: `{ type, expires_at_ms }` — checked at the top of `update()` before FSM logic
- [ ] FSM guard changes:
  - `BoatDetected` entry guarded by `!isLockClosed()`
  - `Open` exit guarded by `!isHoldOpen() || holdExpired()`
- [ ] Log all override applications and lifts (feeds into Phase 3 telemetry)
- [ ] Safety invariant: override never disables sensor-based safety guards — IR obstruction detection and fault detection remain active regardless of override

**Definition of done:** Backend can lock the bridge closed for a timed period and the bridge does not open. Backend can request hold-open and bridge stays open past the normal waterway-clear trigger. All override events appear in the telemetry log.

---

## Dependency Order

```
Phase 1 (PCB)
    └── Phase 2 (Red+Green fix) — can ship with Phase 1 or independently
            └── Phase 3 (Telemetry) ✓ device side done — backend receiver + NTP still open
                    └── Phase 4 (Override API) — depends on Phase 3 network stack and log pipeline
```

Phase 2 is independent and can be done at any time. Phase 3 device-side is complete; Phase 4 can begin once the backend implements the telemetry receiver. Phase 1 does not block Phases 3/4.

---

## Open Questions

- **Stepper over I2C:** The 28BYJ-48 half-step sequence runs at ~333 steps/sec (3 ms/step). MCP23017 I2C writes at 400 kHz add ~100–200 µs overhead per byte. At 4 output pins per step, this is feasible but needs validation on hardware. If latency causes missed steps, stepper stays on direct GPIO.
- **Wi-Fi vs MQTT:** Phase 3 proposes HTTP polling. If the city network runs an MQTT broker (already in `CLAUDE.md` as the intended IoT transport), switching to MQTT pub/sub is straightforward and lower overhead — coordinate with the backend team on which to standardise on.
- **Override push vs poll:** Push gives <1 s latency for the VIP convoy use case; poll gives 5 s. Decision depends on whether the bridge will have a stable local IP and whether the backend can reach it directly.
- **NTP server:** Confirm which NTP server is reachable on the local network used for the city demo.
