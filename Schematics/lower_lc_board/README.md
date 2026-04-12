# Lower Control Board (HYDRA) Documentation

<details>
<summary><strong>Table of Contents</strong></summary>

- [Lower Control Board (HYDRA) Documentation](#lower-control-board-hydra-documentation)
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
<img src="../../Datasheets/images/lower_board_3D.png" width="500">
<p style="font-size: 12px; color: gray;">
<em>Fig. 1: 3D-rendered view of the lower control board.</em>
</p>

The **Lower LC/Hydra** board is a control board designed for integration with the rocket's propulsion system. 
<br>
It operates in the **lower valve bay**, and drives solenoids to control the **dump** and **fill** valves.
<br>
The board supports analog pressure and temperature sensing, two digital hall-effect sensors for solenoid-status reading, and communication with the rest of the stack through CANbus. 

- Interfaces with:
  - STM32F1 MCU
  - Two 24V 205292 Solenoids
  - Two 24V 4-20mA Pressure Transducers (PT)
  - An Omega SA3-K Thermocouple (TC)
  - Two TMAG273 digital hall-effect sensors (via breakot board). 

### Key Features
- STM32F1 microcontroller with additional memory
- I2C and SPI interfaces for sensor integration
- Controlled power delivery for sensors and solenoids
- ORing circuits for power management
- Visual indication for status and debugging
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
| 0.0.1  | 2025-02-23 | Initial prototype with full peripheral and feature integration |
| 0.0.2  | 2025-03-15 | Optimize routing and component placement |
| 0.1.0   | 2026-03-20 | Review sprint: Design refactor for Backplane adapter, silkscreen + review |

## Contributors
- Jeevan Sanchez, Tristan Alderson

--- 

QRET Avionics 25/26