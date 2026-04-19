/*
 * Propulsion Systems Testing Firmware
 * Authors: Jeevan Sanchez, Tristan Alderson
 * Hardware: Lower LC Board (HYDRA), Upper LC Board (PEGASUS)
 * Env: PlatformIO (STM32 and ESP32)
 * Created: ~Feb.12.2026
 * Updated: April.15.2026
 * Purpose: SRAD firmware for peripheral testing of prop. control boards.
 * 
 * Avionics 2025-2026
*/

#include "prop_testing.h"

// ADC ===================
// configure an ADS131M04 ADC with SPI
void adcSetup(ADS131M04& adc, const int ADC_MOSI, const int ADC_MISO, const int ADC_SCLK, const int ADC_CS = 0) {
    SPI.begin(ADC_SCLK, ADC_MISO, ADC_MOSI, ADC_CS); 
    adc.begin();
    delay(100);
    adc.setGain(0, 0, 0, 0); // defualt
}

// read all channels of ADC into a buffer
void readAllADC(ADS131M04& adc, int32_t* outputBuffer) {
    int8_t channels[] = {0, 1, 2, 3}; 
    adc.rawChannels(channels, 4, outputBuffer); 
}

// PRESSURE TRANSDUCER ===================
// returns and optionally logs PSI reading from converted ADC reading voltage
// optional DEBUG mode (single ADC read) or writes to a buffer
// uses pressure linear scaling
float readPT(int8_t channelPT, int32_t* buffer, bool SERIAL_LOG_MODE=true) {
    if (channelPT < 0 || channelPT > 3) return -1.0; // guarded bounds for adc channels (4)
    if (buffer == nullptr) return -1.0; // buffer does not exist

    // convert the raw PT ADC reading into a voltage
    float voltagePT = rawToVoltage((buffer[channelPT])); // use passed down buffer from readall

    float current = (voltagePT / shuntResistance) *  1000.0; // current in mA
    float PSI = (current - 4.0) * (maxPSI / (16.0)); // P = (I - I_min) * (P_max / (I_max - I_min)) | for 4-20 mA PT (16 = 20 - 4)

    if (SERIAL_LOG_MODE) {
        Serial.printf("PT on CH%d: %.4f V | %7.2f PSI\n", channelPT, voltagePT, PSI);
    }

    return PSI;
} 

// THERMOCOUPLE ===================
//  Steinhart-Hart B-parameter equation:
//  1/T = 1/T0 + (1/B) * ln(R / R0)
float readColdJunction(const int TEMP_SENSE) {
    uint32_t raw = analogRead(TEMP_SENSE); // read from CJC Thermistor

    // Convert ADC count to thermistor resistance via voltage divider
    float vOut = (raw / ESP32_ADC_RES) * ESP32_VREF;           // voltage at pin
    float rTherm = THERM_R_SERIES * vOut / (ESP32_VREF - vOut); // NTC resistance (Ω)

    // B-parameter equation -> temperature in Kelvin
    float tKelvin = 1.0 / (1.0 / THERM_T0 + (1.0 / THERM_B) * log(rTherm / THERM_R0));

    return tKelvin - 273.15; // convert to Celsius
}

// --- K-type Thermocouple Temperature ---
// cold junction compensation (CJC):
//   T_hot = T_cold + (V_tc / Seebeck)
// voltage in Volts, returns temperature in Celsius.
float readDeltaTemp(float voltage) {
    float microVoltage = voltage * 1e6; // V -> µV
    return (float)microVoltage / K_TYPE_SEEBECK_UV_PER_C; // µV / (µV/°C) = °C 
    // return deltaTemp
}

// returns and optionally logs temp reading from converted ADC reading voltage
// optional DEBUG mode (single ADC read) or writes to a buffer
float readTC(int8_t channelTC, int32_t* buffer, const int thermistorPin, bool SERIAL_LOG_MODE=true) {
    if (channelTC < 0 || channelTC > 3) return -1.0; // guarded bounds for adc channels
    if (buffer == nullptr) return -1.0; // buffer does not exist

    // convert the raw ADC reading to a voltage
    float voltageTC = rawToVoltage((buffer[channelTC])); // use passed down buffer from readall

    float deltaTemp = readDeltaTemp(voltageTC); 
    float coldJunctionTemp = readColdJunction(thermistorPin); 

    float compensatedTemp = coldJunctionTemp + deltaTemp;

    if (SERIAL_LOG_MODE) {
        Serial.printf("TC CH%-1d | %8.6f V | %7.2f °C | CJC %6.2f °C\n", channelTC, voltageTC, compensatedTemp, coldJunctionTemp);
    }

    return compensatedTemp; 
}

// ANALOG SENSOR BULK READ ===================
// use for full tests to eliminate seperate ADC calls. 
void readAnalogSensors(ADS131M04& adc, int8_t channelPT1, int8_t channelPT2, int8_t channelTC, const int thermistorPin, bool SERIAL_LOG_MODE=true) {
    int32_t readingBuffer[4] = {0}; // 4 channel ADC

    readAllADC(adc, readingBuffer);

    // in the future can store p1,p2,t1 in a AnalogResults struct or something
    float p1 = readPT(channelPT1, readingBuffer, SERIAL_LOG_MODE); 
    float p2 = readPT(channelPT2, readingBuffer, SERIAL_LOG_MODE);

    float t1 = NAN; 
    if (channelTC != -1 && thermistorPin != -1) {
        t1 = readTC(channelTC, readingBuffer, thermistorPin, SERIAL_LOG_MODE);
    }
}

// HALL EFFECT SENSOR ===================
// TODO (implementation will follow the creation of a lib)

// VALVE CONTROL ===================
// actuate a solenoid for a custom duration or two seconds (when not specified)
// solID refers to the connector designator (on PCB), defaults to one
void valveControl(const int solENPin, const int solID = 1, const int duration = 2000) {
    digitalWrite(solENPin, HIGH);
    Serial.printf("solenoid %d ON\n", solID);
    delay(duration); 

    digitalWrite(solENPin, LOW);
    Serial.printf("solenoid %d OFF\n", solID);
}

// UTILS ===================
float rawToVoltage(int32_t raw) {
    return ((float)raw / ADCresFactor) * ADCVRef;
}

// blink a single LED
void blinkLed(int ledPin, int delayMs = 800) {
    digitalWrite(ledPin, HIGH);
    delay(delayMs); 
    digitalWrite(ledPin, LOW);
    delay(delayMs);
}

// blink several LEDs with an array of pins
void flashLeds(const int ledArray[], int ledCount) {
    for (int i = 0; i < ledCount; i++) {
        Serial.printf("flashing LED on GPIO %d\n", ledArray[i]);
        digitalWrite(ledArray[i], HIGH);
        delay(800);
        digitalWrite(ledArray[i], LOW);
    }
}







