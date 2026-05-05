/*
 * TMAG5273 Linear 3D Hall Effect Sensor Library
 * Authors: Jeevan Sanchez
 * Hardware: TI TMAG5273 https://www.ti.com/lit/ds/symlink/tmag5273.pdf
 * Env: Arduino Framework (PlatformIO - STM32 & ESP32)
 * Created: May.4.2026
 * Updated: 
 * Purpose: SRAD Library for use across control boards interfacing with TMAG5273 sensor(s)
 * 
 * QRET Avionics 2025-2026
*/

#include "TMAG5273.h"

// setup the TMAG5273
void TMAG5273::init(uint8_t addr, TwoWire &port) {

} 

// individual axis reads
float TMAG5273::getX() {}

float TMAG5273::getY() {}

float TMAG5273::getZ() {}

float getTemp() {}

// read into a float array for each axis
bool TMAG5273::getAll(float* axes) {}

// write data to a TMMAG5273 register
bool TMAG5273::writeRegister(uint8_t reg, uint8_t data) {
    if (!_i2c) return false; 

    _i2c->beginTransmission(_addr); 
    _i2c->write(reg); 
    _i2c->write(data); 

    if (_i2c->endTransmission() == 0) {
            return true; // success
    } else {
        return false; // endTransmission() = 1 = data too long, 2 = NACK on addr transmit, 3 = NACK on data transmit, 4 = other
    }
}

// read from a TMMAG5273 register
uint8_t TMAG5273::readRegister(uint8_t reg, uint8_t &out) {
    if (!_i2c) return false; 

    _i2c->beginTransmission(_addr); 
    _i2c->write(reg); 
    if (_i2c->endTransmission(false) != 0) return false; // failed to set register pointer
    
   if(_i2c->requestFrom(_addr, (uint8_t)1) != 1) return false; // get no. of received bytes (data)
   out = _i2c->read(); // populate  

   return true;
}

// helper to handle two's complement and read raw from an axis
void TMAG5273::readRawAxis(uint8_t msbReg) {}

