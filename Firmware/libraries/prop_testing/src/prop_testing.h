/*
 * Header for testing utilities.
 *
 * Constants for ADC config.
 * Constants for PT and TC processing.
 * Function prototypes
 * 
 * QRET Avionics 2025-2026
 */

#ifndef PROP_TESTING_H
#define PROP_TESTING_H

#include <Arduino.h>
#include <ADS131M04.h>
#include <SPI.h>
#include <esp_adc_cal.h>
#include <SparkFun_TMAG5273_Arduino_Library.h>
#include <Wire.h>

// PT constants
const float shuntResistance = 62.0;
const float maxPSI = 100; // needs to be double-checked

// 24V sense voltage divider resistance ratio
#define SENSE_24V_DIVIDER_SCALE 11.0f

// Thermistor (cold junction) constants 
// Ohmite TX Series NTC thermistor on pin 10
// Assumes voltage divider: 3.3V -> R_SERIES -> NTC -> GND
const float THERM_R_SERIES  = 10000.0;  // series resistor (Ohms)
const float THERM_R0        = 10000.0;  // thermistor resistance at 25°C (Ohms)
const float THERM_B         = 3435.0;   // B-value (K) — TX06F103F3435ER / TX08F103F3435ER
const float THERM_T0        = 298.15;   // 25°C in Kelvin
const float STM32_VREF      = 3.3;
const float STM32_ADC_RES   = 4095.0;   // 12-bit

// I2C address of TMAG5273 Hall Sensor (from temporary lib homepage:  https://docs.sparkfun.com/SparkFun_TMAG5273_Arduino_Library/)
#define HALL_ADDR 0x22

// --- K-type thermocouple constants ---
// CH0 on the ADS131M04 is a K-type thermocouple
// Seebeck coefficient for K-type: ~41.276 µV/°C (linear approx, valid -200 to +1372°C)
const float K_TYPE_SEEBECK_UV_PER_C = 41.276; // µV / °C

// POWER
void enablePower(const int enablePin, bool state = true);
float powerSense(const int vSensePin);

// ADC
void adcSetup(ADS131M04& adc);
void readAllADC(ADS131M04& adc, int32_t* outputBuffer);

// PT
float processPT(uint8_t chID, float voltagePT, bool SERIAL_LOG_MODE=true);

// TC
float readColdJunction(const int TEMP_SENSE);
float readDeltaTemp(float voltage);
float processTC(uint8_t chID, float voltageTC, const int thermPin, bool SERIAL_LOG_MODE=true);

// ANOLOG SENSOR BULK READ
void readAnalogSensors(ADS131M04& adc, int8_t chPT1, int8_t chPT2, int8_t chTC, const int thermPin, bool SERIAL_LOG_MODE=true);

// HALL SENSOR
extern TMAG6273 hallSensor; 
void hallSetup();
void readHall(); 


// VALVE CONTROL
void valveControl(const int solENPin, const int solID = 1, const int duration = 2000);

// UTILS
void blinkLed(int ledPin, int delayMs = 800);
void flashLeds(const int ledArray[], int ledCount);


#endif