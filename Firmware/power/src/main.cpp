#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial serial(PA10, PA9);

void setup() {
    serial.begin(38400);

    delay(100);
    serial.println("begin");
}


void loop() {

    delay(5000);
    serial.println("loop");

}