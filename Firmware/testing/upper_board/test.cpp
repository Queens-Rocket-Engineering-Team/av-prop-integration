#include <Arduino.h>

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

            default:
                Serial.println("uh");
                break;
        }
    }
}
