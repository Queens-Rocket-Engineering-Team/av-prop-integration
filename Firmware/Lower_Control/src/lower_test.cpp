/*
 * Lower Prop. Control Module Testing FW
 * Authors: Jeevan Sanchez, Tristan Alderson
 * Hardware: Lower LC Board (HYDRA)
 * Env: PlatformIO (STM32)
 * Created: Apr.19.2026
 * Updated: Apr.27.2026
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
#include <ADS131M04.h> 
#include <SPIFlash.h>
#include <FastLED.h>
#include <prop_testing.h>

SoftwareSerial serial(USART_RX_PIN, USART_TX_PIN); 

const int ledArray[] = {CAN_LED_PIN, DB_LED_PIN};

ADS131M04 adc(ADC_CS_PIN, ADC_DRDY_PIN, &SPI);
SPIFlash flash(FL_CS_PIN);

#define NUM_LEDS 1
CRGB rgb_leds[NUM_LEDS];

bool isPolling = false;
unsigned long lastPollTime = 0;
const int pollInterval = 500;

// helper for polling ADC reads
void pollData() {
    if (isPolling && (millis() - lastPollTime >= pollInterval)) {
        lastPollTime = millis();
        int32_t rawData[4];
        float volts[4];

    // readChannels method is true on a successful ADC read
    if (adc.readChannels(rawData)) {
                adc.computeVoltages(rawData, volts);

                float psi = processPT(volts[0]);

                // direct calculation instead of processPT() to log calculated internals (CJC temp)
                float coldJunc = readColdJunction(CJC_SENSE_PIN);
                float deltaT = readDeltaTemp(volts[2]);
                float tempC = coldJunc + deltaT; // compensated temp

                serial.print("PT1: "); 
                serial.print(psi, 1);
                serial.print(" PSI | V_PT: "); 
                serial.print(volts[0], 4); 
                
                serial.print(" | TC: ");
                serial.print(tempC, 1);
                serial.print(" C | CJC: ");
                serial.print(coldJunc, 1); 
                
                serial.print(" | V_TC: "); 
                serial.println(volts[2], 6); 
    }
  }
}

void setup() {
    serial.begin(9600);

    // setup the STM32 clock for ADC_CLKIN | TIM2_CH1 mapped to PA0 
    // https://github.com/stm32duino/Arduino_Core_STM32/wiki/HardwareTimer-library
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HardwareTimer *adcCk = new HardwareTimer(TIM2);
    adcCk->setPWM(1, PA0, 8192000, 50); 
    adcCk->resume();

    SPI.begin();
    
    // setup and disable the flash module
    pinMode(FL_CS_PIN, OUTPUT);
    digitalWrite(FL_CS_PIN, HIGH);

    // setup devices on SPI bus
    flash.initialize();
    adc.init();

    
    pinMode(VPT_EN_PIN, OUTPUT);
    pinMode(VSOL_EN_PIN, OUTPUT);
    pinMode(SOL1_EN_PIN, OUTPUT);
    pinMode(SOL2_EN_PIN, OUTPUT);

    // turn off all controlled power rails
    disablePower(VPT_EN_PIN);
    disablePower(VSOL_EN_PIN);

    for (int i = 0; i < 2; i++) {
        pinMode(ledArray[i], OUTPUT);
        digitalWrite(ledArray[i], LOW);
    }

    // set the resolution to match the STM32F1's 12-bit ADC
    // analog reads used for CJC & 24V sensing
    analogReadResolution(12);

    // RGB LED setup https://fastled.io/docs/
    FastLED.addLeds<WS2812B, RGB_DATA_PIN, GRB>(rgb_leds, NUM_LEDS);
    rgb_leds[0] = CRGB::Black;
    FastLED.show();
}

void loop() {

    // strobe an icey blue colouur
    uint8_t fluidBrightness = beatsin8(35, 20, 90);
    rgb_leds[0] = CHSV(135, 255, fluidBrightness);
    FastLED.show();
    
    pollData();

    if (serial.available() > 0) {
        char input = serial.read();
        switch (input) {
            case '1':
                enablePower(VSOL_EN_PIN);
                enableValve(SOL1_EN_PIN);
                delay(5000); 
                disableValve(SOL1_EN_PIN);
                disablePower(VSOL_EN_PIN);
                break;
                
            case '2':
                enablePower(VSOL_EN_PIN);
                enableValve(SOL2_EN_PIN);
                delay(5000); 
                disableValve(SOL2_EN_PIN);
                disablePower(VSOL_EN_PIN);
                break;

            case '3':
                isPolling = !isPolling;
                if (isPolling) {
                    enablePower(VPT_EN_PIN);
                } else {
                    disablePower(VPT_EN_PIN);
                }
                break;

            case '5':
                flashLeds(ledArray, 2);
                break;
        }
    }
}