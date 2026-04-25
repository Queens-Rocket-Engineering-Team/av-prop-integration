/*
 * Upper Prop. Control Module Testing FW
 * Authors: Jeevan Sanchez, Tristan Alderson
 * Hardware: Upper LC Board (PEGASUS)
 * Env: PlatformIO (ESP32)
 * Created: Apr.19.2026
 * Updated: Apr.25.2026
 * Purpose: SRAD firmware for peripheral testing of upper control module.
 *
 * QRET Avionics 2025-2026
 */

#include "pinouts.h"
#include <prop_testing.h>
#include <WiFi.h>
#include <FastLED.h>

const int ledArray[] = {CAN_LED_PIN, WIFI_LED_PIN, DEBUG_LED_PIN};

#define NUM_LEDS 1
CRGB rgb_leds[NUM_LEDS];


ADS131M04 adc(-1, ADC_DRDY_PIN, &SPI); // -1 for no CS pin (tied low)
// TMAG5273 hallSensor;

// WIFI credentials (replace)
const char *ssid = "";
const char *pswd = "";

// WIFI testing
void wifiTest(const char *ssid, const char *pswd)
{
    Serial.printf("\nconnecting to: %s\n", ssid);
    WiFi.begin(ssid, pswd);

    int timeout_idx = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        blinkLed(WIFI_LED_PIN, 300);
        Serial.print(".");
        timeout_idx++;
        if (timeout_idx == 10)
            break;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.printf("\nconnected to: %s\n", ssid);
        digitalWrite(WIFI_LED_PIN, HIGH);
    }
    else
    {
        Serial.println("\nFailed to connect.");
    }
}

void setup()
{
    Serial.begin(38400);

    // ADC clock setup
    ledcSetup(0, 8192000, 1); // 8.192MHz, 1-bit resolution
    ledcAttachPin(ADC_CLKIN_PIN, 1);
    ledcWrite(0, 1);

    SPI.begin(ADC_SCLK_PIN, ADC_MISO_PIN, ADC_MOSI_PIN, -1);
    Wire.begin(HALL_SDA_PIN, HALL_SCL_PIN);

    adcSetup(adc);
    hallSetup();

    pinMode(VPT_EN_PIN, OUTPUT);
    pinMode(SOL1_EN_PIN, OUTPUT);
    pinMode(SOL2_EN_PIN, OUTPUT);
    pinMode(BUZZ_EN_PIN, OUTPUT);

    for (int i = 0; i < 3; i++)
    {
        pinMode(ledArray[i], OUTPUT);
        digitalWrite(ledArray[i], LOW);
    }

    digitalWrite(VPT_EN_PIN, LOW);
    digitalWrite(SOL1_EN_PIN, LOW);
    digitalWrite(SOL2_EN_PIN, LOW);

    analogReadResolution(12);

    Serial.println("start PEGASUS upper control module testing");
    Serial.println("1: solenoid 1 | 2: solenoid 2 | 3: sensing results | 5: 24V sense | 6: LEDs | 7: WiFi");
}

void loop()
{
    uint8_t fluidBrightness = beatsin8(35, 20, 90);
    rgb_leds[0] = CHSV(55, 255, fluidBrightness);
    FastLED.show();

    if (Serial.available() > 0)
    {
        char input = Serial.read();

        switch (input)
        {
        case '1':
            enableValve(SOL1_EN_PIN);
            delay(5000);
            disableValve(SOL1_EN_PIN);
            break;

        case '2':
            enableValve(SOL2_EN_PIN, 2);
            delay(5000);
            disableValve(SOL2_EN_PIN);
            break;

        case '3':
            enablePower(VPT_EN_PIN);
            delay(20);

            readAnalogSensors(adc, 0, 1, -1, -1, true); // consult schematics for ADC channels
            disablePower(VPT_EN_PIN);
            break;

        case '4':
        {
            uint32_t rawVSOL = analogRead(VSOL_SENSE_PIN);
            float vPin = (rawVSOL / 4095.0) * 3.3;
            float vActual = vPin * SENSE_24V_DIVIDER_SCALE;

            serial.printf("VSOL: %5.2f V | Raw: %u\n", vActual, rawVSOL);
            break;
        }

        case '5':
            flashLeds(ledArray, 3);
            break;

        case '6':
            wifiTest(ssid, pswd);
            break;

        default:
            break;
        }
    }
}
