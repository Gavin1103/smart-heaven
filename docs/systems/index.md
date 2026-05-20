# Smart systems

How the city “thinks” and communicates. This section covers shared infrastructure and cross‑cutting systems that multiple features rely on.

!!! note "Decisions with the mayors"
Two teachers act as our city’s mayors and final decision makers. Check in with them before locking protocols or dashboards.

---

<div class="grid cards" markdown>

:material-access-point-network: **IoT network**
  
  Protocol options (UART/I²C/SPI/CAN vs Wi‑Fi/MQTT/HTTP/CoAP), decision criteria, and topic/schema suggestions.
  
  [:octicons-arrow-right-24: Open](iot-network.md)

:material-view-dashboard: **Dashboard**
  
  Ways to visualize city state (Node‑RED, custom web, Grafana, or simple embeds) and a minimal first step.
  
  [:octicons-arrow-right-24: Open](dashboard.md)

:material-lightning-bolt: **Energy**
  
  Power distribution ideas, safe supplies, and future solar/battery indicators.
  
  [:octicons-arrow-right-24: Open](energy.md)

:material-bike-fast: **Mobility**
  
  Movement, timing, and coordination patterns across traffic, bridge, and rail.
  
  [:octicons-arrow-right-24: Open](mobility.md)

:material-shield-check: **Safety**
  
  Fail‑safes, emergency stops, limit switches, and reset procedures across features.
  
  [:octicons-arrow-right-24: Open](safety.md)

  :material-shield-check: **Databseoptions**
  
  research to the database options for the city.
  
  [:octicons-arrow-right-24: Open](database-research.md)

</div>

---

## Interoperability principles
- Local first: features run standalone; integrations are added later
- Simple messages: small payloads, clear states, predictable timeouts
- Safe defaults: if comms fail, revert to a known safe behavior