# Parking

> One-liner: Smart parking bays that detect occupancy and display availability.

## Status
- Owner: Bram
- Tile: T2 
- Build stage: Designing
- Last updated: 02/04/2026

---

## Goal
Detect when parking spots are taken and show availability locally (LED) and later on the dashboard.

**Success looks like:**
- [ ] Accurate detection for each bay
- [ ] Clear local indicator (green=free, red=occupied)
- [ ] Reset procedure returns all indicators to known state
- [ ] Signage displays the current amount of free bays/free or full
- [ ] Signage is colorcoded same as the individual LED's

---

## Demo experience
**Trigger:** car model placed/removed from bay to simulate.

**What happens:**
1. Sensor detects occupancy
2. Bay indicator changes color
3. Optional message emitted for dashboard
4. Number of free parking spaces on sign decreases by n

**Fail-safe behavior:**
- If sensor noisy, default to free and log blink pattern until stable
- If sign cannot receive data from DB it will:
    - if traffic flow is the focus -> default to full, to direct people away.
    - if efficient parking is the focus -> default to free since some spots may be open.

---

## Inputs (sensors / events)
| Input | Type | Purpose | Notes |
|------|------|---------|------|
| bay1_detect | IR/ultrasonic/reed | detect vehicle in bay 1 | mount under/side |
| bay2_detect | IR/ultrasonic/reed | detect vehicle in bay 2 | mount under/side |

---

## Outputs (actuators / UI)
| Output | Type | Purpose | Notes |
|------|------|---------|------|
| bay1_led | RGB LED | show bay 1 status | G=free, R=occupied |
| bay2_led | RGB LED | show bay 2 status | G=free, R=occupied |
| sign_led | LED matrix (opt.) | show total free | optional |

---

## Hardware (BOM)

### Parking sensor
| Part # | Manufacturer	| Description | Quantity | Price (incl. VAT) | Subtotal (incl. VAT) | Example url |
|-|-|-|-|-|-|-|
|HC-SRO4| MULTICOMP PRO | HC-SRO4 Ultrasone Sensor | 1 / parking spot| €3,00 | €3,00 | https://www.tinytronics.nl/nl/sensoren/afstand/ultrasonische-sensor-hc-sr04 |
| KY-011 | ? | KY-011 RG color LED module | 1 / parking spot | €1,49 | €1,49 | https://www.benselectronics.nl/ky-016-rgb-led-module.html |
| ESP32C3 | CFsunbird | ESP32-C3 Ontwikkelbord Esp32 Supermini Development Board Esp32 Development Board Wifi Bluetooth | 1 / 4 parking spots | €1,61 | €0,40 | https://nl.aliexpress.com/item/1005007723970275.html?spm=a2g0o.tesla.0.0.f0b0ka8nka8ni9&afTraceInfo=1005007723970275__pc__c_ppc_item_bridge_pc_main__xTJ9uzP__1772726890308&gatewayAdapt=glo2nld#nav-specification |
| MB-102 | HW Group | MB-102 solderless breadboard power supply module | 1 | €4,25 | €4,25 | https://www.tinytronics.nl/en/power/power-supplies/breadboard-power-supply-mb102 |
| DUPONT-MM-20 | SparkFun | Dupont jumper wires male-to-male 20 cm | ~3 | €0,15 | €1,50 | https://www.sparkfun.com/jumper-wires-standard-7-m-m-30-awg-30-pack.html |
| DUPONT-MF-20 | SparkFun | Dupont jumper wires male-to-female 20 cm | ~7 | €0,80 | €2,75 | https://www.sparkfun.com/jumper-wires-connected-6-m-f-20-pack.html |
| | | | | | | |

### Parking signage
| Part # | Manufacturer	| Description | Quantity | Price (incl. VAT) | Subtotal (incl. VAT) | Example url |
|-|-|-|-|-|-|-|
| ESP32S3 | Espressif Systems | ESP32 S3 Development Board met 2.4G Wifi BT Module voor AI IOT | 1 | €6,20 | €6,20 | https://nl.aliexpress.com/item/1005008796158734.html?isdl=y&aff_fsk=_oF4otSV&src=BooncyNL&aff_platform=aff_feeds&aff_short_key=_oF4otSV&dp=Cj0KCQjwp7jOBhDGARIsABe7C4dTF65cDnS8yx2Laq2Z5TS8GEZ-O86G_FEXBOgFg-iZBIse2fngyBQaAuZ4EALw_wcB&cn=bravonl&gad_campaignid=23455650631 |
| JMD0 96d-1 | Shenzhen Jinmading Electronics Co., Ltd. | Display OLED 0.96" I2C White JMD0.96D-1 | 1/sign | €2,70 | €2,70 | https://nl.aliexpress.com/i/1005009146291241.html?gatewayAdapt=glo2nld |
| DUPONT-MM-20 | SparkFun | Dupont jumper wires male-to-male 20 cm | ~ | € | €1,50 | https://www.sparkfun.com/jumper-wires-standard-7-m-m-30-awg-30-pack.html |
| DUPONT-MF-20 | SparkFun | Dupont jumper wires male-to-female 20 cm | ~4 | €0,50 | €2,75 | https://www.sparkfun.com/jumper-wires-connected-6-m-f-20-pack.html |
|  | | | | | | |

---

## Software
Behavior/state machine:
- Per-bay: Neutral state is free -> On debounce within parameters -> set status -> update lights -> rescan
- Sign: neutral state is everything full -> request data -> display full/free


Communication:
- Parkingbay: everything runs locally since it is just a simple switch, whenever a change in free/full happens a signal from the esp is sent to the backend to notify the current full/empty parking spaces. This will continue untill an ok signal is sent from the backend or a parking space changes in state when a new amount of free/full will be sent to the backend.

- Sign: Data about the amount of free/full spaces is requested from the backend every ~5 min. 

- Local only initially; later MQTT/HTTP topic naming: `sh/parking/{tile}/bay/{n}`

Config values:
- echo distance threshold, data request time, full to free min amount threshold

---

## Physical design (3D print / laser)
Files:
- CAD/STL folder: TBD
- 3d printed bracket for sensor: tbd
- 3d printed mount for the oled screen/sign: tbd
- 3d printed cover/mount for parking lights: tbd

Design constraints:
- Indicators visible from audience; avoid glare
    - can be solved by implementing a future lightmeter to adjust the amount of light emitted over the day.
---

## Placement on tile
- Location: roadside parking along the main street
- Cable routing: under tile; strain relief near edge
- parking spot dimensions:
    Width: 4cm
    Lenght: 8cm

---

## Test plan (demo-ready)
Basic tests
- [ ] 10× place/remove cycles per bay without false readings
- [ ] Indicators always match sensor state

Failure tests
- [ ] Sensor blocked produces safe default
- [ ] Power cycle restores last known safe defaults

Reset procedure
1. Long-press button to clear all bays to free
2. Power-cycle controller
3. Verify indicators show correct start state

---

## Risks & open questions
Risks
- Sensor cross-talk between adjacent bays → add shielding/threshold tuning

Open questions
- [ ] Number of bays and spacing