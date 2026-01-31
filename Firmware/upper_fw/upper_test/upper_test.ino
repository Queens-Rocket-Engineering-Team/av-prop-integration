#include <Arduino.h>

#define ADC_MOSI 11
#define ADC_MISO 13
#define ADC_CLK 12
#define ADC_DRDY 14
#define ADC_CS 0

#include <ADS131M0x.h>

SPIClass SpiADC(HSPI);
ADS131M0x adc;
adcOutput adc_out;

void adcInit()
{
    adc.setClockSpeed(200000);
    adc.begin(&SpiADC, ADC_CLK, ADC_MISO, ADC_MOSI, ADC_CS, ADC_DRDY);
    adc.setInputChannelSelection(0, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
    adc.setChannelPGA(0, CHANNEL_PGA_1);
}

// initial testing code for upper control board (ESP32-S3)
// test valve control and LED indication
// make sure espressif board manager URL is added in preferences if uploading from arudino IDE, and make sure in CDC meode. 

const int valveEN1 = 9;
const int valveEN2 = 3;

int leds[] = {35, 36, 37, 48, 40, 41, 42};
int ledCount = 7;

// enable MOSFET gate to drive solenoid
void valveControl(const int valvePin) {
    digitalWrite(valvePin, HIGH); 
    Serial.println("solenoid ON");
    delay(2000);
    digitalWrite(valvePin, LOW);
}

// flash all LEDs
void flashLeds(int ledArray[]) {
    for (int i = 0; i < ledCount; i++) {
        Serial.print("flashing LED connected to GPIO:  ");
        Serial.println(ledArray[i]);
        digitalWrite(ledArray[i], HIGH);
        delay(800);
        digitalWrite(ledArray[i], LOW);
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("start upper-launch-control-board testing");
    Serial.println("press 1 to enable solenoid 1, 2 to enable solenoid 2, and 3 to run LED flashing sequence");

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

void loop() {
    if (Serial.available() > 0 ) {
        char input = Serial.read();

        switch (input) {
            case '1':
                Serial.println("starting solenoid 1 test");
                delay(200);
                valveControl(valveEN1);
                break;
            case '2':
                Serial.println("starting solenoid 2 test");
                delay(200);
                valveControl(valveEN2);
                break;
            case '3':
                Serial.println("starting LED test");
                delay(200);
                flashLeds(leds);
                break;

            case '4':
                Serial.println("Read ADC data");
                adc_out = adc.readADC();
                for(int i = 0; i<4; i++){
                  if(digitalRead(ADC_DRDY)) Serial.println("ADC Ready");
                  int32_t output = -1;
                  if (i == 0) output = adc_out.ch0;
                  if (i == 1) output = adc_out.ch1;
                  if (i == 2) output = adc_out.ch2;
                  if (i == 3) output = adc_out.ch3;
                  delay(200);
                  Serial.printf("ADC%d-Val: %d",i, output);
                  Serial.println("");
                }
                break;
            default:
                Serial.println("uh");
                break;
        }
    }
}
