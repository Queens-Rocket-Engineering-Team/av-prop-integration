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

        // settings
        bool setAveraging(uint8_t mode); 
        
        uint8_t getDeviceStatus(); 

    private: 
        uint8_t _addr; 
        uint8_t _range; 
        TwoWire* _i2c; 

        bool readRegister(uint8_t reg, uint8 &out); 
        bool writeRegister(uint8_t reg, uint8_t data); 
        int16_t combineBytes(uint8_t msb, uint8_t lsb); 
        float rawTomT(int16_t raw, float range = 80.0f);
}

#endif 