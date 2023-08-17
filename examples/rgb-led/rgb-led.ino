#include "Freenove_WS2812_Lib_for_ESP32.h"

#define LED_PIN 1

Freenove_ESP32_WS2812 led = Freenove_ESP32_WS2812(1, LED_PIN, 0, TYPE_GRB);

void setup() {
  // put your setup code here, to run once:

  led.begin();
  led.setBrightness(30);  
}

void setLED(uint8_t r,uint8_t g,uint8_t b) {
  led.setLedColorData(0, r, g, b);
  led.show();
}

int demoLEDstate = 0;
void loop() {
  // put your main code here, to run repeatedly:
  switch(demoLEDstate%3) {
    case 0: 
      setLED(0,255,0);
      demoLEDstate++;
      break;
    case 1: 
      setLED(0,0,255);
      demoLEDstate++;
      break;
    default: 
      setLED(255,255,0);
      demoLEDstate = 0;
      break;
  }
  
  delay(1000);
}
