# Dashboard

TBD — we’ll decide with stakeholders (mayors) and teacher input.

## Options
- Node‑RED Dashboard: fast to prototype, easy MQTT/HTTP integration
- Custom web app: full control, more effort
- Grafana: great for time‑series and panels; needs a data source
- MkDocs page with embedded iframes/images: simplest showcase

## First step (suggestion)
  - Start with Node‑RED dashboard or a minimal custom page that shows:
  - Traffic lights state
  - Bridge state (open/closed)
  - Railway barrier state (up/down)
  - Parking bays occupancy

## Data model (TBD)
- Keep simple fields: `feature`, `tile`, `state/value`, `ts`
- Derive aggregate views (e.g., total free parking) on the dashboard