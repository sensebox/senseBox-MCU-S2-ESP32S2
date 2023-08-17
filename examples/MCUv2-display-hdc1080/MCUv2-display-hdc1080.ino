/**
 * Example for senseBoxMCUv2 with SSD1306 and HDC1080 connect via I2C
 * Date: 08.06.2022
 * Author: Mario
 **/


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h> // http://librarymanager/All#Adafruit_GFX_Library
#include <Adafruit_SSD1306.h> // http://librarymanager/All#Adafruit_SSD1306
#include <Adafruit_HDC1000.h> // http://librarymanager/All#Adafruit_HDC1000_Library


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_HDC1000 hdc = Adafruit_HDC1000();





void setup() {
Wire.begin(39,40);
delay(2000);
display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
display.display();
delay(100);
display.clearDisplay();
hdc.begin(0x40);



}


void loop() {
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE,BLACK);
  display.println(hdc.readTemperature());
display.display();

}
