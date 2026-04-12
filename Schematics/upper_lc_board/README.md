# Upper Control Board (PEGASUS) Documentation

<details>
<summary><strong>Table of Contents</strong></summary>

- [Upper Control Board (PEGASUS) Documentation](#upper-control-board-pegasus-documentation)
  - [Overview](#overview)
    - [Key Features](#key-features)
  - [Circuit Deisgn](#circuit-deisgn)
    - [Power Management](#power-management)
    - [MCU Breakout](#mcu-breakout)
    - [Valve Control](#valve-control)
    - [Sensing](#sensing)
  - [Board Design](#board-design)
  - [Revision History](#revision-history)
  - [Contributors](#contributors)
</details>

## Overview
<img src="../../Datasheets/images/upper_board_3D.png" width="500">
<p style="font-size: 12px; color: gray;">
<em>Fig. 1: 3D-rendered view of the upper control board.</em>
</p>

The **Upper LC/Pegasus** board is a control board designed for integration with the rocket's propulsion system. 
<br>
It operates in the **upper valve bay**, and drives solenoids to control the rocket's vent valve.
<br>
The board supports analog pressure sensing, a digital hall-effect sensor for solenoid-status reading, and communication with the rest of the stack through CANbus. It is also **WiFi-capable**. 

- Interfaces with:
  - ESP32-S3 Module
  - Two 24V 205292 Solenoids
  - Two 24V 4-20mA Pressure Transducers (PT)
  - One TMAG273 digital hall-effect sensor (via breakot board). 

### Key Features
- ESP32-S3 microcontroller with integrated WiFi
- I2C and SPI interfaces for sensor integration
- Controlled power delivery for sensors
- ORing circuits for power management
- Auditory and visual indication for status and debugging
- 4-layer PCB with dedicated ground planes
- Protection and filtering components across board

## Circuit Deisgn 

### Power Management

### MCU Breakout

### Valve Control 

### Sensing

## Board Design

## Revision History
| Revision | Date       | Description |
|----------|------------|-------------|
| 0.0.1   | 2025-12-15 | Initial prototype with full peripheral integration |
| 0.0.2 | 2025-12-30 | Optimized routing and component placement |
| 1.1   | 2026-01-16 | Onboard trace cuts and adjustments to prevent short due to reversed diode in thermocouple circuitry |
| 2.0    | 2026-03-05 | Started prototype for second revision of board for manufacturing. Including fixed TC circuit, buzzer, and power control for PT supply |
| 2.1   | 2026-03-27 | Review sprint: route cleanup and silkscreen addiiton |

## Contributors
- Jeevan Sanchez, Tristan Alderson

--- 

QRET Avionics 25/26