#include <Arduino.h>
#include <SPI.h>
#include <ADS131M0x.h>

/* ===================== ADC PINS ===================== */
#define ADC_MOSI 11
#define ADC_MISO 13
#define ADC_CLK  12
#define ADC_CS   0
#define ADC_DRDY 14

SPIClass SpiADC(HSPI);
ADS131M0x adc;
adcOutput adc_out;

/* ===================== VALVES ===================== */
const int valveEN1 = 9;
const int valveEN2 = 3;

/* ===================== LEDS ===================== */
int leds[] = {35, 36, 37, 48, 40, 41, 42};
const int ledCount = 7;

/* ===================== ADC INIT ===================== */
void adcInit() {
    pinMode(ADC_DRDY, INPUT);

    SpiADC.begin(ADC_CLK, ADC_MISO, ADC_MOSI, ADC_CS);

    adc.setClockSpeed(200000);
    adc.begin(&SpiADC, ADC_CLK, ADC_MISO, ADC_MOSI, ADC_CS, ADC_DRDY);

    // ---- Configure CHANNEL 2 ----
    adc.setInputChannelSelection(2, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
    adc.setChannelPGA(2, CHANNEL_PGA_1);
}

/* ===================== VALVE CONTROL ===================== */
void valveControl(const int valvePin) {
    digitalWrite(valvePin, HIGH);
    Serial.println("solenoid ON");
    delay(2000);
    digitalWrite(valvePin, LOW);
}

/* ===================== LED TEST ===================== */
void flashLeds(const int ledArray[], int count) {
    for (int i = 0; i < count; i++) {
        Serial.print("flashing LED on GPIO ");
        Serial.println(ledArray[i]);
        digitalWrite(ledArray[i], HIGH);
        delay(800);
        digitalWrite(ledArray[i], LOW);
    }
}

/* ===================== SETUP ===================== */
void setup() {
    Serial.begin(115200);
    Serial.println("start upper-launch-control-board testing");
    Serial.println("1: solenoid 1 | 2: solenoid 2 | 3: LEDs | 4: ADC CH2");

    pinMode(valveEN1, OUTPUT);
    pinMode(valveEN2, OUTPUT);
    digitalWrite(valveEN1, LOW);
    digitalWrite(valveEN2, LOW);

    for (int i = 0; i < ledCount; i++) {
        pinMode(leds[i], OUTPUT);
        digitalWrite(leds[i], LOW);
    }

    adcInit();
}

/* ===================== LOOP ===================== */
void loop() {
    if (Serial.available() > 0) {
        char input = Serial.read();

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

            case '4':
                Serial.println("Reading ADC channel 2");

                // DRDY is active-LOW
                while (digitalRead(ADC_DRDY));

                adc_out = adc.readADC();

                Serial.print("ADC2 Value: ");
                Serial.println(adc_out.ch2);
                break;

            default:
                Serial.println("unknown command");
                break;
        }
    }
}
