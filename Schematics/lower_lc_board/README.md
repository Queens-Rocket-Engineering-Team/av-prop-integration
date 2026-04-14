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
      - [Mode Selection](#mode-selection)
      - [Flash Storage Module](#flash-storage-module)
      - [CANBUS Interface](#canbus-interface)
      - [Serial Wire Debug](#serial-wire-debug)
      - [USB Interface](#usb-interface)
      - [Serial Wire Debug](#serial-wire-debug-1)
      - [System Feedback](#system-feedback)
    - [Valve Control](#valve-control)
    - [Valve Control](#valve-control-1)
    - [Sensing](#sensing)
      - [Data Aquisition](#data-aquisition)
      - [Pressure Transducers](#pressure-transducers)
      - [Thermocouples](#thermocouples)
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
This board supports analog pressure and temperature sensing, two digital hall-effect sensors for solenoid-status reading, and communication with the rest of the stack through CANbus. 

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
This board is primarily composed of four circuit modules representing its critical systems. 
<br>
These include [Power Management](#power-management), [STM32 Breakout](#mcu-breakout), [Valve Control](#valve-control), and [Sensing](#sensing).
<br>
All schematics are available as `kicad_sch` files in this directory.

### **Power Management**
This board receives power from the Hybrid Power Module through the Backplane. 
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
The [STM32F103]("https://www.st.com/resource/en/datasheet/stm32f103cb.pdf") was selected for this board's MCU. 
<br>
Key features include:
- 72 MHz ARM Cortex-M3 core
- Internal CAN controller 
- Compact LQFP48 package
  
#### Mode Selection
The STM32 can be put into **BOOT** mode and **RESET** with onboard buttons. 
- **BOOT** mode determines whether the chip runs existing code from flash memory or waits to download new firmware. 
  - The STM32 reads the state of the BOOT pin (BOOT0) during reset to select the appropriate boot mode.
  - It is **active-high**: when pulled HIGH (3V3), it will wait for new code over serial/USB.
  - The circuit consists of a momentary push-button and a pull-down resistor to ensure the chip is disabled by default button noise. 
  - It also includes another BOOT pin (BOOT1) that is pulled low to prevent entering SRAM mode. 

- **RESET** controls the Reset (NRST) pin which is the ON/OFF (reset) switch for the STM32. 
  - It is **active-low**: when pulled to GND it turns off the chip, and when released, it restarts. 
  - The circuit consists of a momentary push-button, a pull-up resistor to ensure the chip is enabled by default, and a 0.1 µF capacitor to filter out button noise (debouncing). 

| Level |Boot Mode |
|----------|------------|
| Boot HIGH  | Programming Mode |
| Boot LOW | Memory Mode | 

#### Flash Storage Module
This board includes a BY25Q12BE5 SPI flash memory module providing 16 MB of external non-volatile storage.
- Its connected to the MCU via an SPI interface (SCLK, MOSI, MISO, CS).

#### CANBUS Interface
The STM32 includes an internal CAN controller but requires a CAN transceiver to interface with the Stack's CANBUS. 
- Differential pairs from the backplane edge connector are routed to the CAN transceiver, which converts them into TX and RX signals for the STM32.
- The CAN transceiver selected is a high-speed [TCAN1051](https://www.ti.com/lit/ds/symlink/tcan1051h.pdf?HQS=dis-dk-null-digikeymode-dsf-pf-null-wwe&ts=1776143114328&ref_url=https%253A%252F%252Fwww.ti.com%252Fgeneral%252Fdocs%252Fsuppproductinfo.tsp%253FdistId%253D10%2526gotoUrl%253Dhttps%253A%252F%252Fwww.ti.com%252Flit%252Fgpn%252Ftcan1051h).

#### Serial Wire Debug
This board supports Serial Wire Debug (SWD) for programming and debugging the MCU using an ST-Link debugger.

- The SWD interface uses dedicated SWDIO and SWCLK lines routed to a debug header.
- This enables flashing and debugging via an ST-Link programmer. 


#### USB Interface
The STM32 interfaces with USB via a CP2102N USB-to-UART bridge.
- USB differential pairs are routed directly to the CP2102N, which converts them to UART signals for the STM32.
- The STM32 communicates with the bridge using standard TX and RX lines.

#### Serial Wire Debug
This board supports Serial Wire Debug (SWD) for programming and debugging the MCU using an ST-Link debugger.

- The SWD interface uses dedicated SWDIO and SWCLK lines routed to a debug header.
- This enables flashing and debugging via an ST-Link programmer.

#### System Feedback
This board incorporates feedback through LEDs.

- Visual indicators include a DEBUG indicator, CAN status, and a general STATUS indicator.
  - These consist of standard current-driven LEDs and an ARGB LED.
  

### Valve Control 
Valves are actuated via solenoids using MOSFET-based switching circuits driven by the MCU.
- The system uses a Festo 205292 solenoid to control the rocket vent valve.
  - Operates at 24V and is active-low.
  - Controlled via low-side switching (active when pulled to ground).
  - Interfaces through a MOLEX Nano-Fit connector.

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
This board interfaces with several external sensors, including analog pressure transducers and thermocouples, and digital hall-effect sensors.
- Pressure transducers are used for analog telemetry measurements.
- Hall-effect sensors are used for solenoid state detection and feedback.

#### Data Aquisition
- Analog sensor data is acquried via an external **ADC** (Analog-Digital-Converter) through the SPI protocol.
- Digital signals (from hall-effect sensors) are acquired through the I2C protocol.

- The ADC used in our circuit is the [ADS131M04](https://www.ti.com/lit/ds/symlink/ads131m04.pdf), it has 8-input channels, 24-bit resolution, 32 ksPs programmable sampling rate.
    - Each input channel receives a voltage or differential voltage from a sensor (V<sub>ref</sub> = 1.2 V).
    - A [**delta-sigma**](https://e2e.ti.com/blogs_/archives/b/precisionhub/posts/delta-sigma-adc-basics-understanding-the-delta-sigma-modulator) modulator converts the voltage into a high-frequency bitstream that represents the input analog signal.
        - This modulator is driven by an external clock (CRYSTAL OSCILLATOR, 8.319 mHz)
    - The bitstream is then filtered to produce a clean, high resolution digital value.
    - The ADC then sends a 24-bit number representing the measured voltage to the MCU via **SPI**. 

- A **RC-Filter** is used at the ADC inputs as a low-pass filter to smooth out high-frequency noise.
    - The capacitor charges and discharges through the resistor, smoothing fast transients so the ADC sees a stable voltage.

**Brief Overview on Analog Signals**
  - Analog signals vary continuously, taking any value within a given range, unlike digital signals which have discrete levels.
  - Changes in a sensor’s physical quantity are represented proportionally as a voltage or current signal.
  
#### Pressure Transducers
- The basis of the Pressure Transducer (PT) circuits lie in signal conditioning & protection before the ADC.
    - The PT used is a 4-20 mA [Setra 209H](https://www.setra.com/hubfs/Setra_Product_Data_Sheets/Model_209H_Spec_Sheet_2016_PRELIMINARY_WATERMARK.pdf) connected via Molex Nano-Fit. 
    - The signal line is initially referenced to GND through a 62 Ω shunt resistor to produce a voltage drop proportionate to the signal (current). 
   $$R_{shunt} = \frac{V_{ref}}{I_{max}} = \frac{1.2 V}{0.02 A} = 60  Ω$$
   <p style="font-size: 12px; color:gray;">
<em>** A 62 Ω resistor was chosen for availability.</em>
</p>

- The signal is then clamped to 3.3V and GND using low-leakage diodes.
- A 1 kΩ resistor is placed in series between the signal and diode to limit current into the diodes during overvoltage conditions, protecting the input from excessive current.
- The clamped signal then passes through a 200 Ω resistor connected to a 10 nF capacitor, forming the RC low-pass filter before being sampled by the ADC.

#### Thermocouples
- The basis of the Thermocouple (TC) circuits lie in signal conditioning, protection, and transformation before the ADC.
  - The TC used is a [Omega SA3-L](https://assets.omega.com/pdf/test-and-measurement-equipment/temperature/sensors/thermocouple-probes/SA3.pdf) connected via Screw Terminal. 
    - Unlike the 4–20 mA pressure transducer, it produces a small differential voltage across its two output wires.
    - As a result, the device uses two signal inputs for differential measurement.
  - Both signal lines are initially resistor-biased with two 1M Ω resistors. 
     - Connected to 3V3 and GND respectively, these resistors establish a defined reference operating point and prevents floating inputs.
   - The signals are then clamped to 3.3V and GND using low-leakage diodes.
   - A 1 kΩ resistor is placed in series between each signal and diode to limit current into the diodes during overvoltage conditions, protecting the input from excessive current
   - The signal then passes through a 200 Ω resistor connected to a 10 nF capacitor, forming the RC low-pass filter before being sampled by the ADC.

**Cold-Junction Compensation**

- Cold-junction compensation is used to determine the true absolute temperature at the TC connector, since a thermocouple only measures the temperature difference between the measurement junction and the reference junction.
- This provides the reference temperature of the PCB terminals.
- A **thermistor** is used to measure the reference junction temperature, placed as close as possible to the TC screw terminal.
  - The thermistor then uses a voltage divider to produce a proportional signal for processing by the STM32's internal ADC.

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