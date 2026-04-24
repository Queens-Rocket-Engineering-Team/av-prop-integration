/*
 * Lower Prop. Control Module Testing FW
 * Authors: Jeevan Sanchez, Tristan Alderson
 * Hardware: Lower LC Board (HYDRA)
 * Env: PlatformIO (STM32)
 * Created: Apr.19.2026
 * Updated: Apr.24.2026
 * Purpose: SRAD firmware for peripheral testing of lower control module.
 * 
 * QRET Avionics 2025-2026
*/

#include "pinouts.h"
#include "prop_testing.h"
#include <SPIMemory.h>

const int ledArray[] = {CAN_LED_PIN, DB_LED_PIN};

ADS131M04 adc(ADC_CS_PIN, ADC_DRDY_PIN, &SPI);
SPIFlash flash(FL_CS_PIN, &SPI);

void setup() {
    Serial.begin(115200); 

    pinMode(FL_CS_PIN, OUTPUT); 
    digitalWrite(FL_CS_PIN, HIGH); 

    // start master clock
    HardwareTimer *adcCk = new HardwareTimer(TIM2); // timer 2 (PA0)
    adcCk->setOverflow(8192000, HERTZ_FORMAT);
    adcCk->setCaptureCompare(1, 50, PERCENT_COMPARE_FORMAT); // channel 1 (PA0)
    adcCk->resume();

    SPI.begin(); 

    if (flash.begin()) {
        Serial.println("flash memory online"); 
    } else {
        Serial.println("flash memory failed to initialize");
    }
    
    adcSetup(adc);

    pinMode(VPT_EN_PIN, OUTPUT);
    pinMode(VSOL_EN_PIN, OUTPUT);
    pinMode(SOL1_EN_PIN, OUTPUT);
    pinMode(SOL2_EN_PIN, OUTPUT);
    pinMode(CAN_TX_PIN, INPUT);

    for (int i = 0; i < 2; i++) {
        pinMode(ledArray[i], OUTPUT);
        digitalWrite(ledArray[i], LOW);
    }

    digitalWrite(VPT_EN_PIN, LOW); 
    digitalWrite(VSOL_EN_PIN, LOW);
    digitalWrite(SOL1_EN_PIN, LOW);
    digitalWrite(SOL2_EN_PIN, LOW); 

    Serial.println("start HYDRA lower control module testing");
    Serial.println("1: solenoid 1 | 2: solenoid 2 | 3: sensing results | 4: VSOL (24V) sense | 5: LEDs");
}

void loop() {
    if (Serial.available() > 0) {
        char input = Serial.read(); 

        switch (input) {
            case '1':
                enablePower(VSOL_EN_PIN);
                delay(15);

                valveControl(SOL1_EN_PIN); 
                enablePower(VSOL_EN_PIN, false);
                break;
            
            case '2':
                enablePower(VSOL_EN_PIN); 
                delay(15);

                valveControl(SOL2_EN_PIN, 2); 
                enablePower(VSOL_EN_PIN, false);
                break;

            case '3':
                enablePower(VPT_EN_PIN);
                delay(15);

                readAnalogSensors(adc, 0, 1, 2, CJC_SENSE_PIN, true); // consult schematics for ADC channels
                enablePower(VPT_EN_PIN, false);
                break;
            
            case '4':
                enablePower(VSOL_EN_PIN); 
                delay(15); 

                Serial.printf("VSOL Sense: %.3f V\n", powerSense(VSOL_SENSE_PIN));
                enablePower(VSOL_EN_PIN, false);
                break;

            case '5':
                flashLeds(ledArray, 2); 
                break; 
            
            default:
                Serial.println("------ HYDRA IDLE ------ ");
                break;
        }

    }

}
