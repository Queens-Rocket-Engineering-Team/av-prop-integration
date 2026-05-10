
# Upper Board (Pheonix) Firmware

  

> This directory contains the firmware for the *upper* launch control board.

  

- Written in Arduino C/C++

  
---

### Configuring Arduino environment for ESP32-S3
 Ensure your [Arduino IDE](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE) is up-to-date. 
- Open the *Preferences* menu (Ctrl+Comma), and add the link below into the *Additional Boards Manager* field. 
https://espressif.github.io/arduino-esp32/package_esp32_index.json 

- Navigate to the *Boards Manager* (Ctrl+Shift+B), and download the package **esp32** by **Espressif Systems**. 
- Once installed, open the *Tools* menu, and select the *Board* field, under the library you just installed (esp32), select the board: **ESP32S3 Dev Module**. 
- After selecting your board, you will see more fields under the *Tools* menu, find the field **USB CDC On Boot**, and **ENABLE** it. 
- Make sure you've selected the corrected port and upload!

### ADC Library used for reading from the onboard ADS131M04.
> https://github.com/icl-rocketry/ADS131M04-Lib/tree/master