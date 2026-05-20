# Railway barrier

> One-liner: A level crossing with barrier, lights, and sound that activates safely when a train is incoming.

## Status
- Owner: TBD
- Tile: T5 (suggested)
- Build stage: Designing
- Last updated: YYYY-MM-DD

---

## Goal
Provide a clear, safe railway crossing sequence with barrier motion, flashing lights, and optional bell sound.

**Success looks like:**
- [ ] Barrier lowers/raises smoothly with end detection
- [ ] Flashing lights and sound warn during closure
- [ ] Reset brings system to a known safe state

---

## Demo experience
**Trigger:** button press or simulated train approach signal

**What happens:**
1. Warning lights/bell start
2. Barrier lowers to block road
3. After delay, barrier raises and warnings stop

**Fail-safe behavior:**
- If sensor fault, keep barrier lowered and continue warnings until reset

---

## Inputs (sensors / events)
| Input | Type | Purpose | Notes |
|------|------|---------|------|
| train_in | reed/IR | simulate train approach | position near track |
| lim_down | limit switch | barrier fully down | near hinge |
| lim_up | limit switch | barrier fully up | near hinge |

---

## Outputs (actuators / UI)
| Output | Type | Purpose | Notes |
|------|------|---------|------|
| barrier_servo | servo | raise/lower arm | define angles |
| warn_lights | LEDs | flashing pattern | roadside posts |
| bell | buzzer | audible warning | optional |

---

## Hardware (BOM)
Microcontroller: ESP32 (TBD)  
Power: 5V (USB)  
Parts:
- 1× servo + horn + arm
- 2× limit switches (up/down)
- 2× LED posts + resistors
- 1× buzzer (optional)

Mounting & mechanics:
- Arm pivot on roadside plinth; protected travel stops

Wiring notes:
- Route under tile; label connectors; common ground

---

## Software
Behavior/state machine:
- Idle → Warn → Lower → Wait → Raise → Clear → Idle

Communication:
- Local initially; optional topic later: `sh/rail/{tile}/crossing`

Config values:
- warn duration, lower/raise speeds, debounce

---

## Physical design (3D print / laser)
Files:
- CAD/STL folder: TBD

Design constraints:
- Visible warnings; safe finger gaps

---

## Placement on tile
- Location: across the road at track; ensure clearance when up
- Cable routing: under tile; strain relief at hinge

---

## Test plan (demo-ready)
Basic tests
- [ ] 10× lower/raise cycles without fault
- [ ] Lights/bell flash in correct pattern

Failure tests
- [ ] Power loss mid‑cycle returns to safe state
- [ ] Sensor fault keeps barrier down until reset

Reset procedure
1. Power off; place arm to up if safe
2. Power on; home to up via limit switch
3. Trigger once to verify full cycle

---

## Risks & open questions
Risks
- Servo jitter or stall → ensure proper torque and supply

Open questions
- [ ] Exact track geometry and arm length