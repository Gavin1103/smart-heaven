# Safety & Timing Specification and Verification Plan

## 1. Purpose

This document translates the smart bridge’s functional safety goals into measurable requirements, acceptance criteria,
and verification tests for the current prototype.

## 2. Scope and limits

- Prototype platform: ESP32-S3
- Sensors: HC-SR04, KY-032 IR, reed switch, encoder, HX711
- Actuators: 28BYJ-48 stepper, MG90S servos, LEDs via SN74HC595
- This document defines what can be tested on the prototype and what cannot be guaranteed as industrial-grade safety.

## 3. Safety functions

### SF-01: No opening while bridge deck is occupied

### SF-02: No closing while vessel is still under the bridge

### SF-03: Bridge must reach safe closed reference after boot

### SF-04: Barrier sequence must complete before deck motion begins

### SF-05: Faults must force or keep the system in a safe state

## 4. Acceptance criteria

| ID    | Requirement                                              | Acceptance criterion                                                      | Verification method |
|-------|----------------------------------------------------------|---------------------------------------------------------------------------|---------------------|
| AC-01 | Deck must not open if weight is detected                 | Opening is blocked in 100% of 10 repeated trials while weight > threshold | Functional test     |
| AC-02 | Bridge must reopen if IR beam breaks during closing      | Reopen starts within X ms after beam interruption in 10/10 trials         | Timing test         |
| AC-03 | Homing must succeed after boot                           | System reaches IdleClosed within Y seconds in 10/10 trials                | Startup test        |
| AC-04 | Barrier arms must lower before opening                   | Stepper motion begins only after both servo positions are reached         | Sequence test       |
| AC-05 | LED startup self-test must detect output issues visually | LED sweep completes on all configured outputs at startup                  | POST observation    |

## 5. Test plan

- Test 1: HC-SR04 distance stability
- Test 2: IR debounce / reopen timing
- Test 3: Stepper open-close duration and stall behavior
- Test 4: Servo barrier timing and sequence
- Test 5: LED sweep / fault indication

## 6. FMEA-light

| Failure mode                  | Effect                        | Mitigation                           | Test   |
|-------------------------------|-------------------------------|--------------------------------------|--------|
| HC-SR04 false trigger         | Unnecessary opening cycle     | Debounce and repeated measurement    | Test 1 |
| IR false clear during closing | Risk of closing on vessel     | Immediate reopen on beam break       | Test 2 |
| Stepper stall                 | Deck not in expected position | Encoder/reed timeout to Fault        | Test 3 |
| Servo does not lower barrier  | Unsafe opening sequence       | Guard condition blocks stepper start | Test 4 |
| LED output failure            | Wrong signal to users         | POST + fault LED + manual inspection | Test 5 |

## 7. Prototype limitations

- Hobby components have limited repeatability
- HC-SR04 readings may vary by angle and surface
- Prototype can demonstrate safe logic, but not certify real-world bridge safety