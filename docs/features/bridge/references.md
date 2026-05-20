# Project References & Resources

This page centralizes the technical datasheets, software documentation, and regulatory research used to develop the
Smart Drawbridge.

---

## 🏗️ Hardware Datasheets

Detailed technical specifications for the core components used in this build:

- **Controller:** [ESP32-S3 Series Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- **Logic Expansion:** [TI SN74HC595 8-Bit Shift Register](https://www.ti.com/lit/ds/symlink/sn74hc595.pdf)
- **Weight Sensing:** [Avia Semiconductor HX711 (24-Bit ADC)](https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf)
- **Motion:** [28BYJ-48 Stepper Motor & ULN2003 Driver](https://components101.com/motors/28byj-48-stepper-motor)

---

## 🦀 Software & Frameworks

Resources for the Rust/Embassy asynchronous ecosystem:

- **Framework:** [Embassy-rs Documentation](https://embassy.dev/) – Focus on the `embassy-executor` and `embassy-time`
  for FSM timing.
- **Hardware Abstraction:** [esp-hal](https://github.com/esp-rs/esp-hal) – Rust peripheral access for ESP32 chips.
- **Logic Patterns:** [State Machine Patterns in Rust](https://docs.rust-embedded.org/book/design-patterns/hal/state-machines.html)

---

## 🇳🇱 Dutch Infrastructure Standards

Research into real-world maritime and road safety protocols:

- **Public Transport Org:** [Ministerie van Infrastructuur en Waterstaat (2025)](https://www.government.nl/ministries/ministerie-van-infrastructuur-en-waterstaat)
- **Navigation Standards:** [Rijkswaterstaat: Nautical Signaling and Bridge Operations](https://www.rijkswaterstaat.nl/en)
- **Context:** [Bochen, N. (2024): Public Transportation in the Netherlands](https://www.adamsrecruitment.com/blog/public-transportation-in-the-netherlands)

---

## 🔌 Electrical Design Principles

General guides used for safe circuit design:

- **Signal Protection:** [SparkFun: Voltage Dividers](https://learn.sparkfun.com/tutorials/voltage-dividers/all) – Used
  for 5V to 3.3V sensor logic conversion.
- **Power Isolation:** [Decoupling Capacitors Guide](https://learn.sparkfun.com/tutorials/capacitors/application-examples) – To prevent
  motor-induced brownouts.