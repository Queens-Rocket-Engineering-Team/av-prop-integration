/*
REFERENCE - to be deleted soon (replaced with library + board-specific testing)
*/

#include <Arduino.h>
#include <ADS131M04.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_adc_cal.h>

const int8_t ADC_MOSI = 11;
const int8_t ADC_MISO = 13;
const int8_t ADC_CLKIN = 18;
const int8_t ADC_SCLK = 12;
const int8_t ADC_CS = 0;

const int valveEN1 = 9;
const int valveEN2 = 3;
const int TEMP_SENSE = 10;

int leds[] = {35, 36, 37, 48, 40, 41, 42};
const int ledCount = 7;
const int wifiLed = 40;

const float ADCVRef = 1.2;
const float ADCresFactor = 8388608.0; // 2^23
const float shuntResistance = 165.0;
const float maxPSI = 100; // needs to be double-checked

// FILL IN - wifi credentials
const char* ssid = "";
const char* pswd = "";

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

ADS131M04 adc(ADC_CS, ADC_CLKIN, &SPI, 1);

void adcSetup() {
    SPI.begin(ADC_SCLK, ADC_MISO, ADC_MOSI, ADC_CS);
    adc.begin(); 
    delay(100);
    adc.setGain(0, 0, 0, 0);
}

void blinkLed(int ledPin, int ms) {
    digitalWrite(ledPin, HIGH);
    delay(ms);
    digitalWrite(ledPin, LOW);
    delay(ms);
}

void wifiTest(const char* ssid, const char* pswd) {
    Serial.printf("\nconnecting to: %s\n", ssid);
    WiFi.begin(ssid, pswd); 

    int timeout_idx = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        blinkLed(wifiLed, 300);
        Serial.print(".");
        timeout_idx++;
        if (timeout_idx=10)
          break;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\connected to: %s\n", ssid);
        digitalWrite(wifiLed, HIGH);
    } else {
        Serial.println("\nFailed to connect.");
    }
}

void valveControl(const int valvePin) {
    digitalWrite(valvePin, HIGH);
    Serial.println("solenoid ON");
    delay(2000);
    digitalWrite(valvePin, LOW);
    Serial.println("solenoid OFF");
}

void flashLeds(const int ledArray[], int count) {
    for (int i = 0; i < count; i++) {
        Serial.printf("flashing LED on GPIO %d\n", ledArray[i]);
        digitalWrite(ledArray[i], HIGH);
        delay(800);
        digitalWrite(ledArray[i], LOW);
    }
}

float rawToVoltage(int32_t reading) {
    float voltage = (reading / ADCresFactor) * ADCVRef; 
    return voltage; 
}

// likely will be implemented with more filtering
float voltageToPSI(float voltage) {
    float currentMA = (voltage / shuntResistance) * 1000;
    float psiReading = (currentMA - 4.0) * (maxPSI / 16.0); // P = (I - I_min) * (P_max / (I_max - I_min))
    if (psiReading < 0) psiReading = 0; 

    return psiReading;
}

void readADC(int32_t* outputBuffer) {
    int8_t allChannels[] = {0, 1, 2, 3};
    adc.rawChannels(allChannels, 4, outputBuffer);
}

// Uses the Steinhart-Hart B-parameter equation:
//   1/T = 1/T0 + (1/B) * ln(R / R0)
// Returns temperature in Celsius.
float readColdJunction() {
    uint32_t raw = analogRead(TEMP_SENSE);

    // Convert ADC count to thermistor resistance via voltage divider
    float vOut = (raw / ESP32_ADC_RES) * ESP32_VREF;           // voltage at pin
    float rTherm = THERM_R_SERIES * vOut / (ESP32_VREF - vOut); // NTC resistance (Ω)

    // B-parameter equation -> temperature in Kelvin
    float tKelvin = 1.0 / (1.0 / THERM_T0 + (1.0 / THERM_B) * log(rTherm / THERM_R0));

    return tKelvin - 273.15; // convert to Celsius
}

// --- K-type Thermocouple Temperature (CH0) ---
// cold junction compensation (CJC):
//   T_hot = T_cold + (V_tc / Seebeck)
// voltage in Volts, returns temperature in Celsius.
float voltageToThermocouple(float voltage, float coldJunctionCelsius) {
    float voltageMicroV = voltage * 1e6;                               // V -> µV
    float deltaTemp     = voltageMicroV / K_TYPE_SEEBECK_UV_PER_C;    // µV / (µV/°C) = °C
    return coldJunctionCelsius + deltaTemp;
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    Serial.println("start upper-lc-board testing");
    Serial.println("1: solenoid 1 | 2: solenoid 2 | 3: LEDs | 4: ADC | 5: TEMP | 6: WiFi");

    pinMode(valveEN1, OUTPUT);
    pinMode(valveEN2, OUTPUT);
    digitalWrite(valveEN1, LOW);
    digitalWrite(valveEN2, LOW);

    pinMode(TEMP_SENSE, INPUT);

    pinMode(wifiLed, OUTPUT); 
    digitalWrite(wifiLed, LOW); // on by default, so explicitly turn it off til needed

    for (int i = 0; i < ledCount; i++) {
        pinMode(leds[i], OUTPUT);
        digitalWrite(leds[i], LOW);
    }

    adcSetup();
}

void loop() {
    if (Serial.available() > 0) {
        char input = Serial.read();
        int32_t adcResults[4] = {0, 0, 0, 0};
        uint32_t temp_sense = 0;

        switch (input) {
            case '1':
                valveControl(valveEN1);
                break;

            case '2':
                valveControl(valveEN2);
                break;

            case '3':
                flashLeds(leds, ledCount);
                break;

            case '4': {
                readADC(adcResults);
                float coldJunctionC = readColdJunction(); // read CJC once before ADC loop

                Serial.println("ADC Readings:");
                for (int i = 0; i < 4; i++) {
                    float volts = rawToVoltage(adcResults[i]);

                    if (i == 0) {
                        // CH0: K-type thermocouple with cold junction compensation
                        float tempC = voltageToThermocouple(volts, coldJunctionC);
                        Serial.printf("CH%d: %.4f V | %7.2f °C (K-type TC, CJC: %.2f °C)", i, volts, tempC, coldJunctionC);
                    } else {
                        // CH1-3: pressure transducers
                        float psi = voltageToPSI(volts);
                        Serial.printf("CH%d: %.4f V | %7.2f PSI", i, volts, psi);
                    }
                    Serial.println();
                }
                break;
            }

            case '5':
                temp_sense = analogRead(TEMP_SENSE);
                Serial.printf("Temp analog raw: %d", temp_sense);
                Serial.println();
                {
                    float cjTemp = readColdJunction();
                    Serial.printf("Cold junction: %.2f °C", cjTemp);
                    Serial.println();
                }
                break;

            case '6':
                wifiTest(ssid, pswd); 
                break;

            default:
                Serial.println("uh");
                break;
        }
    }
}
