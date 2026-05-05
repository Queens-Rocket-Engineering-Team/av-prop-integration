/*
 * Header for TMAG5273A1 Linear 3D Hall Effect Sensor Library
 * 
 * Consult Table 6-2 in datasheet for I2C adresses if NOT using the A1 model. 
       * https://www.ti.com/lit/ds/symlink/tmag5273.pdf
 * 
 * QRET Avionics 2025-2026
*/

#ifndef TMAG5273_H
#define TMAG5273_H

#include <Arduino.h>
#include <Wire.h>

#include "registers.h"

class TMAG5273 {
    public:
        bool init(uint8_t addr = 0x35, TwoWire &port = Wire); 

        // data
        float getX(); 
        float getY(); 
        float getZ(); 
        float getTemp();
        bool getAll(float* axes); 

    private: 
        uint8_t _addr; 
        uint8_t _range; 
        TwoWire* _i2c; 

        uint8_t readRegister(uint8_t reg); 
        void writeRegister(uint8_t reg, uint8_t data); 
        void readRawAxis(uint8_t msbReg); 

}

#endif 