# Lower Control Board (HYDRA) Documentation

<details>
<summary><strong>Table of Contents</strong></summary>

- [Lower Control Board (HYDRA) Documentation](#lower-control-board-hydra-documentation)
  - [Overview](#overview)
    - [Key Features](#key-features)
  - [Circuit Deisgn](#circuit-deisgn)
    - [**Power Management**](#power-management)
      - [**Power Entry**](#power-entry)
      - [**Power MUX**](#power-mux)
      - [**Power Switching**](#power-switching)
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
The board is primarily composed of four circuit modules representing its critical systems. 
<br>
These include [Power Management](#power-management), [STM32 Breakout](#mcu-breakout), [Valve Control](#valve-control), and [Sensing](#sensing).
<br>
All schematics are available as `kicad_sch` files in this directory.

### **Power Management**
The board receives power from the Hybrid Power Module through the Backplane. 
  - For more information on power delivery, check out [this repository](https://github.com/Queens-Rocket-Engineering-Team/av-power). 
<br>
The supply delivers **three** levels of voltage to the board. 

| Voltage (V) | Purpose |
|----------|------------|
| 3.3  | Logic supply for microcontroller and other peripheral devices, also serves as a reference for signal conditioning and protection circuits |
| 5  | Intermediate supply for specific peripheral devices | 
| 24   | High-voltage supply for driving external devices such as solenoids and sensors | 
<p style="font-size: 12px; color: light-gray;">
<em>Note that 3.3V is referred to as <b>3V3</b></em>
</p>


#### **Power Entry**
Power rails enter through a Molex Micro-Fit connector and are conditioned by bulk filtering and input protection circuitry. These circuits apply to the 5V and 24V rails, each decoupled with 100 µF electrolytic capacitors to handle voltage transients and provide local energy storage. The 24V rail includes a 2A Fuse for overcurrent protection.
<br>

Other power sources come from communication devices such as the USB connection and the JTAG debugger. The USB creates a voltage rail of 5V, which is then stepped-down to 3V3 with a **Low-Dropout Regulator (LDO)**. The JTAG debugger supplies 3V3. 

All rails feature test points and LEDs for diagnosis and debugging. 

#### **Power MUX**
Power Multiplexers (MUX) are circuits that select between multiple power inputs to drive a single load. 
<br>
This is needed to avoid back-feeding between power sources, which could lead to unintended powering of interfaces and potential damage.
<br>

This implementation uses a **Triple OR-ing** circuit to select between three 3.3V inputs: Backplane (V_BP), JTAG (V_SWD), and USB (V_USB).
  - The circuit involves three Ideal Diodes ICs, which use a MOSFET and controller to mimic the behaviour of an **Ideal Diode** (Zero voltage drop when forward-biased, zero current when reverse-biased)
  - The logic follows:
    - **ON conditon**: A chip turns on if its *Vin* is **higher** than the common output rail *Vout*. 
    - **ON conditon**: A chip turns on if its *Vin* is **higher** than the common output rail *Vout*.
    - Each chip uses its enable *CE* pin to monitor the output rail.
      - If it sees a higher a voltage present, it remains closed to prevent back-feeding.
<br>

In summary, a source will only drive the load if its voltage is higher than the current voltage on the output rail, allowing only **one** source to drive the output rail.
- Capacitors are placed at the input pins to filter noise and prevent voltage spikes when a source is suddenly connected or disconnected.
- On the output rail, capacitors act as reservoirs to maintain steady voltage while also filtering noise generated by the loads connected to the 3V3 rail.

#### **Power Switching**
The delivery of 24V to the pressure transducers and solenoids is controlled through a switching circuit. 
- For PTs, this was added to reduce power consumption by enabling sensor power only during active operation.
- For solenoids, this was added as a pre-actuation layer to provide additional protection and controlled switching.
- The controlled output rails are identified as `VPT` and `VSOL` respectively.
  
<br>

The circuit uses a **CMOS** (dual-MOSFET) setup: one N-channel at the input to drive a P-channel for the output. 

**MOSFET Basics**
  - **N-Channel**: **ON** when the gate voltage is higher than the source voltage (Positive V<sub>GS</sub>). 
    - V<sub>GS</sub> > V<sub>th</sub>
  - **P-Channel**: **ON** when the gate voltage is lower than the source voltage (Negative V<sub>GS</sub>). 
    - V<sub>GS</sub> < V<sub>th</sub>
  - **P-Channel**: **ON** when the gate voltage is euqal to the source voltage. 
    - V<sub>GS</sub> = 0
  - The **ON** state connects the **Drain** (output) pin to the **Source** pin. 

**Logic**
  - The STM32 outputs a digital signal to drive the gate of the N-channel MOSFET. The source of the MOSFET is set to GND (0V). 
  - The gate is now at a higher voltage than the source and it sets the drain to GND. 
  - The drain connects to a resistor divider with 24V to drive the gate of the P-channel MOSFET. The source of the MOSFET is set to 24V.
    - The resistor divider limits the gate voltage to approximately 14.81 V.
    - This is lower than the source, setting the drain to 24V, and powering the output rail. 
   $$V_g = 24 \times \frac{10k}{(6.2 + 10k)} \approx 14.81$$
   $$V_{gs} = 14.81-24 \approx -9.19 V$$
   $$V_{gs} = -9.19 < -20 V$$

- From [DMP4065 Datasheet](${KIPRJMOD}/../../Datasheets/pMOSFET_DMP4065S-7_DS.pdf), V<sub>GS</sub> = ±20 V

- `VPT` is filtered using a Pi filter with a ferrite bead between a 10 µF bulk capacitor and a 0.1 µF decoupling capacitor to reduce ripple and high-frequency noise.
- `VSOL` is decoupled with a 100 µF electrolytic capacitor for local energy storage and transient support.

- Additionally, `VSOL` uses a voltage divider for sensing.
  - The divider consists of two resistors (100k and 10k) that scale the 24V input down to approx. 2.18V for measurement by an STM32 ADC input.
   $$V = 24 \times \frac{10k}{(100k + 10k)} \approx 2.18$$

- The circuit also includes two resistors at the MOSFET gate. 
    - Series (Gate) Resistor: Limits inrush current into gate while controling switching speed and protecting MCU pin. 
    - Pull-Down Resistor: Connects signal to GND to define a default logic state when nothing is actively driving the gate.

### MCU Breakout

### Valve Control 
Valves are actuated via solenoids using MOSFET-based switching circuits driven by the MCU.
- The system uses two Festo 205292 solenoids to control the rocket's throttle and fill valves.
  - Operates at 24V and is active-low.
  - Controlled via low-side switching (active when pulled to ground).
  - Interfaces through a MOLEX Nano-Fit connector.

The circuits consist of an N-channel MOSFET for low-side switching, flyback (Schottky) diodes for protection, and hardware-driven LEDs for visual indication.
<p style="font-size: 12px; color:gray;">
<em>The MOSFET basics section in the <b>Power Management</b> section will be useful in understanding this circuit.</em>
</p>

**Logic**
  - The solenoid positive terminal is connected to 24V. 
  - The STM32 outputs a digital signal that serves as the circuit's control input. 
  - This signal feeds an N-channel MOSFET with its source connected to GND (0V) and its drain connected to the negative terminal of the solenoid input (SOL_RET). 
  - The signal will turn the MOSFET `ON` since the gate voltage (3.3V) will be higher than the source voltage (0V). 
  - This connects the MOSFET drain to source, pulling the solenoid negative terminal to ground and actuating the valve.

**Protection & Filtering**
- A 40V, 3A SS34 Schottky diode is placed across the solenoid to provide a path for inductive flyback (voltage spike) current during turn-off.
  - This is common practice to protect against voltage spikes generated by inductive loads during switching.
  
- A local 0.1 µF capacitor is placed near the connector for high-frequency decoupling.

**Visual Indication**
- For indication and debugging a purely-hardware driven LED circuit was leveraged. 
  - An LED with a series current-limiting resistor is placed across the solenoid, with the anode on the positive terminal and the cathode on the negative terminal.
  - By default, the anode is connected to 24V and the cathode is floating. 
  - When the solenoid is driven and the negative terminal is pulled low, the LED is connected on both sides and emits. 

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