# IoT Network

We’ll choose protocols as a team. For now, keep designs flexible so features can run locally and later connect.

## Options (to be decided)
- Physical/data-link: UART/Serial, I²C, SPI, CAN
- Network/application: Wi‑Fi, MQTT, HTTP, CoAP

## Decision criteria
- Distance and interference on a busy demo table
- Reliability and message ordering needs
- Energy/power constraints of controllers
- Complexity and team familiarity

## Suggested baseline (TBD)
- Start local only (no network) so each feature can demo in isolation
- If we adopt MQTT later, consider topics like:
  - `sh/traffic_lights/{tile}/state`
  - `sh/bridge/{tile}/state`
  - `sh/rail/{tile}/crossing`
  - `sh/parking/{tile}/bay/{n}`
- Keep payloads simple JSON or key=value lines

## Interop guidelines
- Every message should include: `feature`, `tile`, `state` (or `value`), and a timestamp if possible
- Define timeouts and safe defaults (what to do if messages stop)
- Document topics/endpoints on each feature page when added