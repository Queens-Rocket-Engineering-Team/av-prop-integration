/*
 * Propulsion Systems Testing Firmware
 * Authors: Jeevan Sanchez, Tristan Alderson
 * Hardware: Lower LC Board (HYDRA), Upper LC Board (PEGASUS)
 * Env: PlatformIO (STM32 and ESP32)
 * Created: ~Feb.12.2026
 * Updated: April.24.2026
 * Purpose: SRAD firmware for peripheral testing of prop. control boards.
 * 
 * QRET Avionics 2025-2026
*/
#include <prop_testing.h>

// SoftwareSerial instance - must be defined by user in their sketch
// e.g.: SoftwareSerial debugSerial(RX_PIN, TX_PIN);

// POWER ===================
// controlled power delivery
void enablePower(const int enablePin, bool state) {
    digitalWrite(enablePin, state ? HIGH : LOW);
}

// voltage sensing
// platform-specific ADC conversion (ESP32 vs STM32)
float powerSense(const int vSensePin) {
    float v_adc = 0.0f;

    #if defined(ARDUINO_ARCH_ESP32)
        v_adc = analogReadMilliVolts(vSensePin) / 1000.0f;

    #elif defined(ARDUINO_ARCH_STM32)
        int32_t raw = analogRead(vSensePin);
        if (raw < 0) return NAN;

        v_adc = (raw / 4095.0f) * 3.3f;

    #else
        #error "unsupported"
    #endif

    return v_adc * SENSE_24V_DIVIDER_SCALE;
}

// ADC ===================
// configure an ADS131M04 ADC with SPI
void adcSetup(ADS131M04& adc) {
    // Clock setup is handled in board-specific files BEFORE this function is called 
    
    adc.init();
    delay(100); 
}

// read all channels of ADC into a buffer
bool readAllADC(ADS131M04& adc, int32_t* outputBuffer) {
    bool success = adc.readChannels(outputBuffer); 

    // zero-out buffer on failure
    if (!success) {
        for (int i= 0; i < 4; i++) outputBuffer[i] = 0;
    }

    return success; 
}

// PRESSURE TRANSDUCER ===================
// returns and optionally logs PSI reading from converted ADC reading voltage
// optional DEBUG mode (single ADC read) or writes to a buffer
// uses pressure linear scaling
float processPT(uint8_t chID, float voltagePT, bool SERIAL_LOG_MODE) {
    float current = (voltagePT / shuntResistance) *  1000.0; // current in mA
    float PSI = (current - 4.0) * (maxPSI / (16.0)); // P = (I - I_min) * (P_max / (I_max - I_min)) | for 4-20 mA PT (16 = 20 - 4)

    if (SERIAL_LOG_MODE) {
        DEBUG_PORT.printf("PT on CH%d: %.4f V | %7.2f PSI\n", chID, voltagePT, PSI);
    }
    return PSI;
} 

// THERMOCOUPLE ===================
//  Steinhart-Hart B-parameter equation:
//  1/T = 1/T0 + (1/B) * ln(R / R0)
float readColdJunction(const int TEMP_SENSE) {
    uint32_t raw = analogRead(TEMP_SENSE); // read from CJC Thermistor

    // Convert ADC count to thermistor resistance via voltage divider
    float vOut = (raw / STM32_ADC_RES) * STM32_VREF;           // voltage at pin
    float rTherm = THERM_R_SERIES * vOut / (STM32_VREF - vOut); // NTC resistance (Ω)

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
float processTC(uint8_t chID, float voltageTC, const int thermPin, bool SERIAL_LOG_MODE) {
    float deltaTemp = readDeltaTemp(voltageTC); 
    float coldJunctionTemp = readColdJunction(thermPin); 
    float compensatedTemp = coldJunctionTemp + deltaTemp;

    if (SERIAL_LOG_MODE) {
        DEBUG_PORT.printf("TC CH%-1d | %8.6f V | %7.2f °C | CJC %6.2f °C\n", chID, voltageTC, compensatedTemp, coldJunctionTemp);
    }

    return compensatedTemp; 
}

// ANALOG SENSOR BULK READ ===================
// use for full tests to eliminate seperate ADC calls. 
void readAnalogSensors(ADS131M04& adc, int8_t chPT1, int8_t chPT2, int8_t chTC, const int thermPin, bool SERIAL_LOG_MODE) {
    int32_t raw[4]; // raw ADC readings
    float volts[4]; // readings converted to voltages

    adc.readChannels(raw);
    adc.computeVoltages(raw, volts); 

    // in the future can store p1,p2,t1 in a AnalogResults struct or something
    float p1 = processPT(chPT1, volts[chPT1], SERIAL_LOG_MODE); 
    float p2 = processPT(chPT2, volts[chPT2], SERIAL_LOG_MODE);
    float t1 = processTC(chTC, volts[chTC], thermPin, SERIAL_LOG_MODE);
}

// HALL EFFECT SENSOR ===================
// Temporary usage of the SparkFun TMAG5273 Library for immediate testing. 
// An SRAD library is in development. 
void hallSetup() {
    if (hallSensor.begin(HALL_ADDR, Wire) == true) {
        DEBUG_PORT.println("TMAG5273 online"); 
    } else {
        DEBUG_PORT.println("TMAG5263 failed to initialize"); 
    }
}

void readHall(int hallID) {
    float x = hallSensor.getXData();
    float y = hallSensor.getYData();
    float z = hallSensor.getZData(); 

    DEBUG_PORT.printf("HALL %d: MAG [mT] X:%.2f Y:%.2f Z:%.2f\n", hallID, x, y, z);
}

// VALVE CONTROL ===================
// actuate a solenoid for a custom duration or two seconds (when not specified)
// solID refers to the connector designator (on PCB), defaults to one
void valveControl(const int solENPin, const int solID, const int duration) {
    digitalWrite(solENPin, HIGH);
    DEBUG_PORT.printf("solenoid %d ON\n", solID);
    delay(duration); 

    digitalWrite(solENPin, LOW);
    DEBUG_PORT.printf("solenoid %d OFF\n", solID);
}

// UTILS ===================
// blink a single LED
void blinkLed(int ledPin, int delayMs) {
    digitalWrite(ledPin, HIGH);
    delay(delayMs); 
    digitalWrite(ledPin, LOW);
    delay(delayMs);
}

// blink several LEDs with an array of pins
void flashLeds(const int ledArray[], int ledCount) {
    for (int i = 0; i < ledCount; i++) {
        DEBUG_PORT.printf("flashing LED on GPIO %d\n", ledArray[i]);
        digitalWrite(ledArray[i], HIGH);
        delay(800);
        digitalWrite(ledArray[i], LOW);
    }
}







