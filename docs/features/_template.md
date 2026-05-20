# Feature: Feature name

> One-liner: Describe what this feature does in the city, in 1 sentence.

## Status
- **Owner:** TBD
- **Tile:** T1–T5 / district name
- **Build stage:** Idea / Designing / Prototyping / Working / Polishing
- **Last updated:** YYYY-MM-DD

---

## Goal
What problem does it solve or what experience does it create?

**Success looks like:**
- [ ] clear measurable outcome 1
- [ ] clear measurable outcome 2
- [ ] clear measurable outcome 3

---

## Demo experience
What will people see when this runs?

**Trigger:**
- button press / sensor detects / timer / remote command

**What happens:**
1. step 1
2. step 2
3. step 3

**Fail-safe behavior:**
- describe what it does if something goes wrong

---

## Inputs (sensors / events)
| Input | Type | Purpose | Notes |
|------|------|---------|------|
| name | IR / ultrasonic / LDR / reed switch / button | why | mounting / placement |

---

## Outputs (actuators / UI)
| Output | Type | Purpose | Notes |
|------|------|---------|------|
| name | LED / servo / motor / display / buzzer | why | movement range / brightness |

---

## Hardware (BOM)
**Microcontroller:** ESP32 / Arduino / Raspberry Pi Pico / etc.  
**Power:** 5V / 12V / USB (+ estimated current)  
**Parts list:**
- part 1
- part 2
- part 3

**Mounting & mechanics:**
- how it attaches to the tile
- moving parts + safety stops / limit switches

**Wiring notes:**
- connector type, cable length limits, label scheme

---

## Software
**Behavior/state machine (short):**
- State A: ...
- State B: ...
- State C: ...

**Communication (if any):**
- Local only / Serial / MQTT / HTTP
- Topic/endpoint naming (if used): `example/topic`

**Config values:**
- timing: ms/s
- thresholds: sensor threshold
- servo angles: min/max

---

## Physical design (3D print / laser)
**Files:**
- CAD: link or folder path
- STL/DXF: link or folder path

**Design constraints:**
- max footprint: mm x mm
- max height: mm
- tolerances: e.g. 0.2mm
- material: PLA/PETG/wood/acrylic

---

## Placement on tile
- **Location:** near road / corner / center
- **Green space impact:** still ≥10%? yes/no
- **Cable routing:** under tile / channel / edge

---

## Test plan (demo-ready)
**Basic test**
- [ ] Trigger works 10 times in a row
- [ ] Output behaves correctly
- [ ] No loose wires / no snagging parts

**Failure tests**
- [ ] Sensor blocked / noisy input
- [ ] Power unplug/replug
- [ ] Movement hits limit safely

**Reset procedure (IMPORTANT)**
1. describe how to reset to safe state
2. describe how to restart software
3. describe how to verify it’s ready

---

## Risks & open questions
**Risks**
- risk 1 + mitigation
- risk 2 + mitigation

**Open questions**
- [ ] question 1
- [ ] question 2

---

## Notes / decisions
Add anything important so we don’t forget later.
