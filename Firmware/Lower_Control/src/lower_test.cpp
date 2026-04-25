/*
 * Lower Prop. Control Module Testing FW
 * Authors: Jeevan Sanchez, Tristan Alderson
 * Hardware: Lower LC Board (HYDRA)
 * Env: PlatformIO (STM32)
 * Created: Apr.19.2026
 * Updated: Apr.24.2026
 * Purpose: SRAD firmware for peripheral testing of lower control module.
 * 
 * Note: Currently doesn't support HALL reads 
        * No hall sensors are physically connected to the board currently. 
        * Support will follow the development of an SRAD Hall Effect library
 * 
 * QRET Avionics 2025-2026
*/

#include "pinouts.h"
#include <SoftwareSerial.h>

SoftwareSerial debugSerial(USART_TX_PIN, USART_RX_PIN);

#include <prop_testing.h>
#include <SPIFlash.h>

const int ledArray[] = {CAN_LED_PIN, DB_LED_PIN};

ADS131M04 adc(ADC_CS_PIN, ADC_DRDY_PIN, &SPI);
SPIFlash flash(FL_CS_PIN);

void setup() {
    debugSerial.begin(38400);
    debugSerial.println("serial ready");

    
    pinMode(FL_CS_PIN, OUTPUT); 
    digitalWrite(FL_CS_PIN, HIGH); 

    SPI.begin(); 

    HardwareTimer *adcCk = new HardwareTimer(TIM2);
    adcCk->setOverflow(8192000, HERTZ_FORMAT);
    adcCk->setCaptureCompare(1, 50, PERCENT_COMPARE_FORMAT);
    adcCk->resume();

    if (flash.begin()) {
        debugSerial.println("flash memory online"); 
    } else {
        debugSerial.println("flash memory failed to initialize");
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
    digitalWrite(CAN_LED_PIN, LOW);

    digitalWrite(DB_LED_PIN, HIGH);

    debugSerial.println("start HYDRA lower control module testing");
    debugSerial.println("1: solenoid 1 | 2: solenoid 2 | 3: sensing results | 4: VSOL (24V) sense | 5: LEDs");
}

void loop() {
    if (debugSerial.available() > 0) {
        char input = debugSerial.read(); 

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

                readAnalogSensors(adc, 0, 1, 2, CJC_SENSE_PIN, true);
                enablePower(VPT_EN_PIN, false);
                break;
            
            case '4':
                enablePower(VSOL_EN_PIN); 
                delay(15); 

                debugSerial.print("VSOL Sense: ");
                debugSerial.println(powerSense(VSOL_SENSE_PIN));
                enablePower(VSOL_EN_PIN, false);
                break;

            case '5':
                debugSerial.print("flashing LEDs\n");
                flashLeds(ledArray, 2); 
                break; 
            
            default:
                debugSerial.println("1: solenoid 1 | 2: solenoid 2 | 3: sensing results | 4: VSOL (24V) sense | 5: LEDs");
                break;
        }

    }

}