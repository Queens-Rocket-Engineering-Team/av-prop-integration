/*
* Upper LC Board Testing Firmware
* Authors: Jeevan Sanchez
* Hardware: Upper Launch-Control Board (ESP32-S3)
* Created: February 2026
* Overview: Valve Ctrl, ADC, WiFi, LED testing for the upper lc board. (CAN testing not implemented yet.)


** Av/Propulsion Integration 
*/

#include <Arduino.h>
#include <ADS131M04.h>
#include <SPI.h>
#include <WiFi.h>

const int8_t ADC_MOSI = 11;
const int8_t ADC_MISO = 13;
const int8_t ADC_CLKIN = 18;
const int8_t ADC_SCLK = 12;
const int8_t ADC_CS = 0;

const int valveEN1 = 9;
const int valveEN2 = 3;

int leds[] = {35, 36, 37, 48, 40, 41, 42};
const int ledCount = 7;
const int wifiLed = 40;

const float ADCVRef = 1.2;
const float ADCresFactor = 8388608.0; // 2^23
const float shuntResistance = 165.0;
const float maxPSI = 100; // needs to be double-checked

// FILL IN - wifi credetnials
const char* ssid = "";
const char* pswd = "";

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

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        blinkLed(wifiLed, 300);
        Serial.print(".");
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
    float voltage = (reading / ADCresFactor ) * ADCVRef; 
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

void setup() {
    Serial.begin(9600);
    delay(1000);
    Serial.println("start upper-lc-board testing");
    Serial.println("1: solenoid 1 | 2: solenoid 2 | 3: LEDs | 4: ADC | 5: WiFi");

    pinMode(valveEN1, OUTPUT);
    pinMode(valveEN2, OUTPUT);
    digitalWrite(valveEN1, LOW);
    digitalWrite(valveEN2, LOW);

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
                Serial.println("ADC Readings:");
                for(int i = 0; i < 4; i++) {
                    float volts = rawToVoltage(adcResults[i]);
                    float psi = voltageToPSI(volts); 
                    
                    Serial.printf("CH%d: %.4f V | %7.2f PSI", i, volts, psi);
                    Serial.println();
                }
                break;
            }
            case '5':
                wifiTest(ssid, pswd); 
                break;

            default:
                Serial.println("uh");
                break;
        }
    }
}