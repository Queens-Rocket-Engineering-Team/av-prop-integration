/*
 * Header for testing utilities.
 *
 * Constants for ADC config.
 * Constants for PT and TC processing.
 * Function prototypes
 */

#ifndef PROP_TESTING_H
#define PROP_TESTING_H

#include <Arduino.h>
#include <ADS131M04.h>
#include <SPI.h>
#include <esp_adc_cal.h>

// ADC constants
const float ADCVRef = 1.2;
const float ADCresFactor = 8388608.0; // 2^23


// PT constants
const float shuntResistance = 62.0;
const float maxPSI = 100; // needs to be double-checked

// Thermistor (cold junction) constants 
// Ohmite TX Series NTC thermistor on pin 10
// Assumes voltage divider: 3.3V -> R_SERIES -> NTC -> GND
// Confirm R_SERIES matches your board's pull-up resistor
const float THERM_R_SERIES  = 10000.0;  // series resistor (Ohms)
const float THERM_R0        = 10000.0;  // thermistor resistance at 25°C (Ohms)
const float THERM_B         = 3435.0;   // B-value (K) — TX06F103F3435ER / TX08F103F3435ER
const float THERM_T0        = 298.15;   // 25°C in Kelvin
const float ESP32_VREF      = 3.3;
const float ESP32_ADC_RES   = 4095.0;   // 12-bit

// --- K-type thermocouple constants ---
// CH0 on the ADS131M04 is a K-type thermocouple
// Seebeck coefficient for K-type: ~41.276 µV/°C (linear approx, valid -200 to +1372°C)
const float K_TYPE_SEEBECK_UV_PER_C = 41.276; // µV / °C

// function prototypes
// ADC
void adcSetup(ADS131M04& adc, const int ADC_MOSI, const int ADC_MISO, const int ADC_SCLK, const int ADC_CS = 0);
void readAllADC(ADS131M04& adc, int32_t* outputBuffer);

// PT
float readPT(int8_t channelPT, int32_t* buffer, bool SERIAL_LOG_MODE=true);

// TC
float readColdJunction(const int TEMP_SENSE);
float readDeltaTemp(float voltage);
float readTC(int8_t channelTC, int32_t* buffer, const int thermistorPin, bool SERIAL_LOG_MODE=true);

// ANOLOG SENSOR BULK READ
void readAnalogSensors(ADS131M04& adc, int8_t channelPT1, int8_t channelPT2, int8_t channelTC, const int thermistorPin, bool SERIAL_LOG_MODE=true);
// VALVE CONTROL
void valveControl(const int solENPin, const int solID = 1, const int duration = 2000);

// UTILS
float rawToVoltage(int32_t raw);
void blinkLed(int ledPin, int delayMs = 800);
void flashLeds(const int ledArray[], int ledCount);


#endif