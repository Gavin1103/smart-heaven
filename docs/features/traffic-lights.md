# Traffic lights
One-liner: Smart intersection lights that react to cars/pedestrians and coordinate with other features.

## Status
- Owner: TBD
- Tile: T1 (suggested)
- Build stage: Designing
- Last updated: YYYY-MM-DD

---

## Goal
Create a reliable, demo-friendly intersection with realistic red/amber/green cycles that can be triggered by sensors or messages.

**Success looks like:**
- [ ] Stable cycle timing with clear visibility
- [ ] Sensor or button reliably triggers a pedestrian phase
- [ ] Reset procedure restores a safe default state

---

## Demo experience
**Trigger:** button press or vehicle sensor detection

**What happens:**
1. Lights cycle to stop traffic
2. Optional pedestrian phase activates
3. Lights return to normal operation

**Fail-safe behavior:**
- Revert to blinking amber (all directions) on error/reset

---

## Inputs (sensors / events)
| Input | Type | Purpose | Notes |
|------|------|---------|------|
| vehicle_detect | IR / ultrasonic | detect vehicles near stop line | mount low, aim towards lane |
| cross_button | button | manual trigger for pedestrian phase | accessible near sidewalk |

---

## Outputs (actuators / UI)
| Output | Type | Purpose | Notes |
|------|------|---------|------|
| light_NS | RGB LED array | north–south light | red/amber/green |
| light_EW | RGB LED array | east–west light | red/amber/green |
| ped_signal | LED / buzzer | pedestrian indicator | optional |

---

## Hardware (BOM)
Microcontroller: ESP32 (TBD)
Power: 5V (USB)  
Parts:
- 2× RGB LED modules or 6× mono LEDs (R/A/G × 2)
- 1× tactile button + housing
- 1× sensor (IR/ultrasonic) for vehicle detect
- Resistors, wires, headers

Mounting & mechanics:
- Traffic light housings as 3D prints; mount near crossing
- Hide wiring via underside channels

Wiring notes:
- Label channels and connectors; observe polarity

---

## Software
Behavior/state machine:
- Idle → Detect/Request → Transition → Pedestrian → Clear → Idle

Communication:
- Local only initially; later MQTT/HTTP topic naming: `sh/traffic_lights/{tile}/state`

Config values:
- green/yellow/red timings, debounce for button/sensor

---

## Physical design (3D print / laser)
Files:
- CAD/STL folder: TBD

Design constraints:
- compact, readable from audience distance; avoid glare

---

## Placement on tile
- Location: intersection centerline; ensure visibility from both approaches
- Cable routing: under tile where possible

---

## Test plan (demo-ready)
Basic tests
- [ ] 10× cycle without timing drift
- [ ] Button works 10× without bounce issues

Failure tests
- [ ] Unplug/replug power returns to safe state
- [ ] Sensor noisy input handled gracefully

Reset procedure
1. Hold button 3s to force blinking amber
2. Power-cycle controller
3. Verify both directions amber blinking, then resume

---

## Risks & open questions
Risks
- LED visibility in bright light → test diffusers

Open questions
- [ ] Exact intersection geometry and post spacing