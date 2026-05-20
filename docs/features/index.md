# Features

Working demo modules that make Smart Heaven feel alive. Each feature has its own page with goals, sensors/actuators, hardware, software/state, tests, and a reset procedure.

!!! tip "Add a new feature"
Copy the template and start filling it out as soon as you begin building.

[:material-file-document-edit-outline: Feature template](./_template.md)

---

<div class="grid cards" markdown>

:material-traffic-light: **Traffic lights**
  
  Smart intersection lights that react to cars/pedestrians and coordinate with other features.
  
  [:material-arrow-right: Open](traffic-lights.md)

:material-bridge: **Bridge**
  
  A small moving bridge that safely opens/closes with clear signals.
  
  [:material-arrow-right: Open](bridge/index.md)

:material-train: **Railway barrier**
  
  A level crossing with barrier, lights, and sound that activates safely when a train is incoming.
  
  [:material-arrow-right: Open](railway-barrier.md)

:material-parking: **Parking**
  
  A simple parking bay with occupancy detection and a status indicator.
  
  [:material-arrow-right: Open](parking.md)


:material-bus-stop: **Busstop – Design**

Technical design of the smart bus stop, including hardware layout, sensors, and system structure.

[:material-arrow-right: Open](busstop_design.md)


:material-lightbulb-on: **Busstop – Advice**

Design decisions, improvements, and recommendations for building a reliable smart bus stop module.

[:material-arrow-right: Open](busstop_advice.md)


:material-magnify: **Busstop – Research**

Background research on requirements, technologies, and data sources used for the smart bus stop.

[:material-arrow-right: Open](busstop_research.md)

</div>

---

## How features interact (later)
- Share events over a common protocol (TBD: MQTT/HTTP)
- Use consistent topic/endpoint names, e.g. `sh/{feature}/{tile}/state`
- Design for safe defaults if comms fail