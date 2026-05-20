# Technical Recommendation: Bridge Deck Occupancy Detection

**Project:** Smart Drawbridge (Canal Crossing)  
**Author:** Rocco Reus – Embedded & Robotics Engineer  
**Date:** March 19, 2026  
**Target audience:** Mayor of Smart Heaven City / Project Evaluator

<!-- TOC -->
* [Technical Recommendation: Bridge Deck Occupancy Detection](#technical-recommendation-bridge-deck-occupancy-detection)
  * [1. Executive Summary](#1-executive-summary)
  * [2. Decision Context](#2-decision-context)
  * [3. Options Considered](#3-options-considered)
  * [4. Evidence and Analysis](#4-evidence-and-analysis)
    * [4.1 External evidence](#41-external-evidence)
  * [5. Recommendation](#5-recommendation)
  * [6. Expected Impact](#6-expected-impact)
  * [7. Conclusion](#7-conclusion)
  * [8. Use of AI.](#8-use-of-ai)
  * [9. References](#9-references)
<!-- TOC -->

---

## 1. Executive Summary

This memo evaluates four options for verifying that the movable bridge deck is clear before the opening sequence starts:
**computer vision, ultrasonic sensing, IR beam detection, and load-cell weight sensing**. Based on external technical
sources and the current Smart Drawbridge design, I recommend using **HX711 + load cells as the primary bridge-deck
occupancy check**.

This recommendation is the strongest fit for the project because it provides a direct physical measurement of deck load,
fits the current hardware stack, and integrates well with the Rust/Embassy software
architecture. [1],[4],[5],[I1],[I2],[I3]

---

## 2. Decision Context

The central safety risk of an automated drawbridge is simple: **the bridge must never begin opening while a vehicle,
cyclist, or pedestrian is still on the movable deck**. For that reason, the detection method should be judged on
criteria that are common in traffic-sensor selection, including:

1. **Coverage of the deck area**
2. **Quality of the safety confirmation**
3. **Integration effort in the current prototype**
4. **Installation and maintenance impact**

These criteria are consistent with traffic-detector guidance, which evaluates sensor technologies by factors such as
data accuracy, spatial resolution, detection area, installation requirements, and maintenance burden. [2]

---

## 3. Options Considered

| Option | Method             | Strengths                                                               | Limitations                                                                  |
|:-------|:-------------------|:------------------------------------------------------------------------|:-----------------------------------------------------------------------------|
| **A**  | Computer Vision    | Rich scene information; can distinguish different object types          | Depends on image quality, camera placement, and added software complexity    |
| **B**  | Ultrasonic Sensors | Cheap and easy to prototype                                             | Limited beam pattern; reflective surfaces and geometry can reduce confidence |
| **C**  | IR Beam / Array    | Fast binary detection and simple logic                                  | Coverage depends on beam placement; does not directly measure deck load      |
| **D**  | HX711 + Load Cells | Direct physical load measurement; strong fit for clearance verification | Requires mechanical integration, calibration, and threshold tuning           |

---

## 4. Evidence and Analysis

### 4.1 External evidence

**Computer vision is a realistic bridge-safety technology, but mainly as operator support.** Rijkswaterstaat’s Smart
Patrol project describes bridge-deck detection based on existing camera feeds and AI, with the goal of recognising
unsafe situations during bridge operation. The same source also states that the notification supports the operator and
that operation remains a human decision.[3] This makes camera-based detection a credible option, but it also suggests
that vision systems are often used as a support layer rather than the sole automatic safety interlock.

**Traffic engineering treats sensor selection as a trade-off between coverage, accuracy, and maintenance.** The FHWA
Traffic Detector Handbook explains that sensor choice depends on parameters such as accuracy, spatial resolution,
detection area, installation requirements, and maintenance burden.[2] For this project, the most important question is
not “what object is on the deck?” but “is the deck physically clear enough to open safely?”

**The HX711 is technically suited to load-cell based occupancy sensing.** The HX711 datasheet describes it as a
precision 24-bit ADC for weigh scales and industrial control applications, designed to interface directly with bridge
sensors, with a low-noise programmable gain amplifier and simple digital control interface. [1] That makes it a
technically appropriate choice for a prototype that uses load cells to detect whether the bridge deck is still carrying
weight.

**Embassy supports the concurrent control style used by this project.** Embassy’s executor documentation describes async
tasks and task spawning, while Espressif’s Rust on ESP documentation describes Embassy as an async framework for
embedded Rust and notes executor support for interrupt-mode and thread-mode use. [4],[5] That supports the project
approach of monitoring sensors continuously while separately handling motion and signalling tasks.

---

## 5. Recommendation

**I recommend that the City of Smart Heaven use HX711 + load cells as the primary bridge-deck occupancy detection
method.**

This is the strongest option for this project because it:

- provides **direct physical measurement** rather than only visual or beam-based observation;
- fits the project’s chosen **hardware stack**;
- is consistent with the project documents that describe **weight pads as the deck-clearance mechanism**;
- is supported by external technical sources on load-cell sensing and embedded async control. [1],[4],[5],[I2],[I3],[I4]

A separate note is that the **IR safety beam remains valuable elsewhere in the bridge system**, specifically for
**under-bridge clearance during open and closing phases**, but that is a different safety function from the
deck-occupancy decision addressed in this memo. [I2],[I4]

---

## 6. Expected Impact

If this recommendation is implemented, the project will gain a more defensible and better-structured safety argument:

- the bridge opening decision will be based on **physical deck-clearance evidence**;
- the recommendation will match the documented role of the **HX711 + load-cell subsystem**;
- the project will demonstrate that the final advice is based on **both external research and internal design evidence
  **;
- the prototype will better reflect how real infrastructure projects combine sensing, control logic, and separate safety
  interlocks for different hazards. [2],[3],[I2],[I3],[I4]

---

## 7. Conclusion

Weight-based occupancy detection is not the only possible detection method, but it is the **best-supported choice for
this project context**. Camera-based detection is relevant in real bridge operations, ultrasonic and IR sensing are
useful supporting technologies, and traffic engineering shows that detector choice should be based on coverage,
accuracy, and maintenance criteria. [2],[3]

For the Smart Drawbridge specifically, **HX711 + load cells** are the most suitable primary method for checking whether
the bridge deck is clear before opening. The IR beam still plays an important role in the wider system, but as an
**under-bridge clearance interlock**, not as the main detector for cars on the bridge. [1],[I2],[I3],[I4]

---

## 8. Use of AI.

This document was prepared with the assistance of an AI writing collaborator. The AI was utilized
strictly as an assistant to perform the following tasks:

- **Drafting and Formatting:** Organizing raw engineering notes into a professional memo format.
- **Editorial Review:** Improving grammatical clarity, tone consistency, and spelling.
- **Structural Optimization:** Ensuring the document follows a logical flow suitable for a municipal evaluator.

Note on Research: All technical conclusions, hardware selections (HX711, Load Cells, ESP32/Rust), and safety logic are
the result of human research. The AI did not generate the underlying technical data or the final recommendation; it
served only to refine the presentation of that information.

---

## 9. References

- Avia Semiconductor. *HX711: 24-Bit Analog-to-Digital Converter (ADC) for Weigh Scales*. Available via SparkFun
  mirror: <https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf>
- Federal Highway Administration. *Traffic Detector Handbook: Third Edition, Chapter 4 – In-Roadway Sensor
  Design*. <https://www.fhwa.dot.gov/publications/research/operations/its/06108/04.cfm>
- Smart Patrol. *Detectie op de brug*. <https://smartpatrol.nl/detectie-op-de-brug>
- Embassy. *embassy-executor documentation*. <https://docs.embassy.dev/>
- Espressif. *Async – The Rust on ESP
  Book*. <https://docs.espressif.com/projects/rust/book/application-development/async.html>
- Gemini. (2026). Gemini (3 PRO). https://gemini.google.com

[1]: https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf>

[2]: https://www.fhwa.dot.gov/publications/research/operations/its/06108/04.cfm>

[3]: https://smartpatrol.nl/detectie-op-de-brug>

[4]: https://docs.embassy.dev/>

[5]: https://docs.espressif.com/projects/rust/book/application-development/async.html>

[I1]: bridge_architecture.md

[I2]: bridge_components_list.md

[I3]: bridge_research.md

[I4]: index.md
