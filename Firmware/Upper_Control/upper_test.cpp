/*
 * Upper Prop. Control Module Testing FW
 * Authors: Jeevan Sanchez, Tristan Alderson
 * Hardware: Upper LC Board (PEGASUS)
 * Env: PlatformIO (ESP32)
 * Created: Apr.19.2026
 * Updated: 
 * Purpose: SRAD firmware for peripheral testing of upper control module.
 * 
 * Avionics 2025-2026
*/

#include "pinouts.h"
#include "prop_testing.h"

const int ledArray[] = {CAN_LED_PIN, WIFI_LED_PIN, DEBUG_LED_PIN};

ADS131M04 adc(0, ADC_CLKIN_PIN, &SPI, 1);

// WIFI credentials (replace)
const char* ssid = ""; 
const char* pswd = ""; 

// WIFI testing 
void wifiTest(const char* ssid, const char* pswd) {
    Serial.printf("\nconnecting to: %s\n", ssid);
    WiFi.begin(ssid, pswd); 

    int timeout_idx = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        blinkLed(WIFI_LED_PIN, 300);
        Serial.print(".");
        timeout_idx++;
        if (timeout_idx == 10)
          break;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nconnected to: %s\n", ssid);
        digitalWrite(wifiLed, HIGH);
    } else {
        Serial.println("\nFailed to connect.");
    }
}

void setup() {
    Serial.begin(115200); 

    pinMode(VPT_EN_PIN, OUTPUT);
    pinMode(SOL1_EN_PIN, OUTPUT);
    pinMode(SOL2_EN_PIN, OUTPUT);
    pinMode(BUZZ_EN_PIN, OUTPUT);

    for (int i = 0; i < 3; i++) {
        pinMode(ledArray[i], OUTPUT);
        digitalWrite(ledArray[i], LOW);
    }

    digitalWrite(VPT_EN_PIN, LOW); 
    digitalWrite(SOL1_EN_PIN, LOW);
    digitalWrite(SOL2_EN_PIN, LOW); 

    adcSetup(adc, ADC_MOSI_PIN, ADC_MISO_PIN, ADC_SCLK_PIN);
    analogReadResolution(12); // forces it to use 0-4095


    Serial.println("start PEGASUS upper control module testing");
    Serial.println("1: solenoid 1 | 2: solenoid 2 | 3: sensing results | 4: 24V sense | 5: LEDs | 6: WiFi");
}

void loop() {
    if (Serial.available() > 0) {
        char input = Serial.read(); 

        switch (input) {
            case '1':
                valveControl(SOL1_EN_PIN); 
                break;
            
            case '2':
                valveControl(SOL2_EN_PIN, 2); 
                break;

            case '3':
                enablePower(VPT_EN_PIN);
                delay(15);
            
                readAnalogSensors(adc, 0, 1, -1, -1, true); // consult schematics for ADC channels
                enablePower(VPT_EN_PIN, false);
                break;

            case '4': 
                Serial.printf("24V Sense: %.3f V\n", powerSense(SENSE_24V_PIN));
                break;

            case '5':
                flashLeds(ledArray, 3); 
                break; 

            case '6':
                wifiTest(ssid, pswd); 
                break; 
            
            default:
                Serial.println("\n------\n");
                break;
        }

    }

}
