#pragma once

// ── I2C bus pins (ESP32-S3 DevKit) ────────────────────────────────────────
#define I2C_SDA_PIN  8
#define I2C_SCL_PIN  9

// ── City node address ranges (logical node IDs on the hub routing table) ──
#define ADDR_BRIDGE_MIN    0x10
#define ADDR_BRIDGE_MAX    0x1F

#define ADDR_TRAFFIC_MIN   0x20
#define ADDR_TRAFFIC_MAX   0x2F

#define ADDR_PARKING_MIN   0x30
#define ADDR_PARKING_MAX   0x3F

// ── Bridge Node PCB — IC I2C addresses ────────────────────────────────────
// Each Bridge Node PCB carries an MCP23017 (I/O expander) and a PCA9685PW
// (PWM driver). Their hardware address pins are set by solder jumpers on the PCB.
// See docs/features/bridge/bridge_pcb_design.md §7 for the full address table.
//
//   Bridge 1 (default, all address pins low):  MCP=0x20  PCA=0x40
//   Bridge 2 (A0 high on both ICs):            MCP=0x21  PCA=0x41
//   Bridge 3 (A1 high on both ICs):            MCP=0x22  PCA=0x42
//
// Compute from node address: mcp = MCP_BASE + (node - ADDR_BRIDGE_MIN)
#define ADDR_BRIDGE_MCP_BASE  0x20  // MCP23017 base (bridge 1 default)
#define ADDR_BRIDGE_PCA_BASE  0x40  // PCA9685PW base (bridge 1 default)

// ── Bridge Node PCB — MCP23017 Port A bit assignments ─────────────────────
// Port A direction register (IODIRA): GPA4 = input, all others = output
#define MCP_A_IODIR    0x10   // 0b00010000
#define MCP_A_PULLUP   0x10   // pull-up on HX711 DOUT (GPA4) only

#define MCP_A_STEPPER_IN1  0  // GPA0 → ULN2003 IN1 (stepper coil 1)
#define MCP_A_STEPPER_IN2  1  // GPA1 → ULN2003 IN2 (stepper coil 2)
#define MCP_A_STEPPER_IN3  2  // GPA2 → ULN2003 IN3 (stepper coil 3)
#define MCP_A_STEPPER_IN4  3  // GPA3 → ULN2003 IN4 (stepper coil 4)
#define MCP_A_HX_DT        4  // GPA4 → HX711 DOUT  (input)
#define MCP_A_HX_SCK       5  // GPA5 → HX711 SCK   (output)
#define MCP_A_SR_DATA      6  // GPA6 → 74HC595 SER  (output)
#define MCP_A_SR_CLK       7  // GPA7 → 74HC595 SRCLK (output)

// ── Bridge Node PCB — MCP23017 Port B bit assignments ─────────────────────
// Port B direction register (IODIRB): GPB0 = output, all others = input
#define MCP_B_IODIR    0xFE   // 0b11111110
#define MCP_B_PULLUP   0xFE   // pull-up on all inputs (GPB1–GPB7)

#define MCP_B_SR_LATCH    0  // GPB0 → 74HC595 RCLK   (output)
#define MCP_B_ENC_A       1  // GPB1 → encoder CLK    (input)
#define MCP_B_ENC_B       2  // GPB2 → encoder DT     (input)
#define MCP_B_ENC_SW      3  // GPB3 → encoder SW     (input, active-low)
#define MCP_B_REED        4  // GPB4 → KY-025 reed    (input, active-low)
#define MCP_B_IR          5  // GPB5 → KY-032 IR      (input, active-low)
#define MCP_B_ECHO_EAST   6  // GPB6 → HC-SR04 East echo (input)
#define MCP_B_ECHO_WEST   7  // GPB7 → HC-SR04 West echo (input)

// ── Bridge Node PCB — PCA9685PW channel assignments ───────────────────────
#define PCA_CH_SERVO_LEFT   0  // CH0 → left barrier servo
#define PCA_CH_SERVO_RIGHT  1  // CH1 → right barrier servo
#define PCA_CH_BUZZER       2  // CH2 → passive buzzer (PWM tone)
#define PCA_CH_SONAR_EAST   3  // CH3 → HC-SR04 East trigger
#define PCA_CH_SONAR_WEST   4  // CH4 → HC-SR04 West trigger

// Sonar stagger: East fires at period start, West fires at half-period (10 ms
// later at 50 Hz) to avoid simultaneous echo interference.
#define PCA_SONAR_EAST_OFFSET  0u
#define PCA_SONAR_WEST_OFFSET  2048u
