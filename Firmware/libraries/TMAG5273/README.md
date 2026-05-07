# TMAG5273 Low-Power Linear 3D Hall-Effect Sensor Library
> QRET Avionics 2025-2026 

A C++/Arduino library for interfacing with the TI TMAG5273A1.

## Features 
- **3-Axis Sensing:** Full magnetic flux density retrieval (X, Y, Z) in mT. 
- **Internal Temperature Compensation:** Integrated temperature sensing for environmental control and reading offsets. 
- **Platform Compatibility:** Designed for **ESP32** and **STM32** architectures via the `TwoWire` interface.
  
<hr>

## Installation
#### PlatformIO 
Add the following to your `platformio.ini` file:
```ini
lib_deps = 
    https://github.com/Queens-Rocket-Engineering-Team/av-prop.git#main:Firmware/libraries/TMAG5273
```

#### Manual Installation
1. Download this repository
2. Place the folder into your project's `lib/` directory. 

<hr>

## Wiring Reference

| TMAG5273 Pin | Function              |  Connection | 
|--------------|-----------------------|----------------|
| VCC          | Power (2.3V - 3.6V)   | 3V3            | 
| SDA          | I2C Data              | MCU I2C SDA pin| 
| SCL          | I2C Clock             | MCU I2C SCL pin| 
| GND          | Ground                | GND            | 

**Note:** Ensure 2.2kΩ - 10kΩ pull-up resistors are present on the SDA and SCL lines. 

<hr>

## Using the Library

<hr>

## API Reference

QRET Avionics 25/26

