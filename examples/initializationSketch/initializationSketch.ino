#include <Adafruit_NeoPixel.h>
#include <Adafruit_MPU6050.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Adafruit_SSD1306.h>
#include "SD.h"
#include "SPI.h"

// display defines
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

char testVersion[] = "1.0";

// Object declaration
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_NeoPixel rgb_led(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Adafruit_MPU6050 mpu;
WiFiMulti WiFiMulti;
WiFiServer server(80);


// Variables for tests
bool pdTestPassed = false;
bool mpuTestPassed = false;
bool wifiTestPassed = false;
bool displayTestPassed = false;
bool sdTestPassed = false;

int deviceCount = 0; 
IPAddress accessPointIP;
// MPU
sensors_event_t a, g, temp;
// PD
int pdTestValue = 0;
// Array to hold all connected I2C devices
const int maxDevices = 128; // Maximale Anzahl möglicher I2C-Adressen
byte deviceAddresses[maxDevices];
// interval for display page change
const long intervalInterval = 5000;
long time_startInterval = 0;
long time_actualInterval = 0;
// counter for which page which is displayed
int displayPage = 0;
int fileCounter = 0;
// ssid and pass for the access point
char ssid[] = "sensebox"; 
char pass[] = "sensebox";


unsigned long getUptimeInSeconds() {
  return millis() / 1000;
}


void displayCheck() {
  Wire.begin();
  byte error;
  Wire.beginTransmission(0x3D);
  error = Wire.endTransmission();
  if (error != 0){
    Serial.println("Display nicht gefunden!");
    return;
  }
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.display();
  Serial.println("Display gefunden!");
  displayTestPassed = true;
  delay(1000);
}

void displayTestResults() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE,BLACK);
  
  display.println("MPU Test: ");
  if (mpuTestPassed)
    display.println("Erfolgreich!");
  else
    display.println("Fehlgeschlagen!");
  
  display.println(" ");

  display.println("Photodiode Test: ");
  if (pdTestPassed)
    display.println("Erfolgreich!");
  else
    display.println("Fehlgeschlagen!");
  
  display.println(" ");

  display.println("Wifi Test: ");
    if (wifiTestPassed)
    display.println("Erfolgreich!");
  else
    display.println("Fehlgeschlagen!");
  display.display();
}

void displayI2CResults() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE,BLACK);
  display.println("Suche nach Geraeten... ");
  display.display();
}

void displayWifiStatus(){
  if(wifiTestPassed) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.setTextColor(WHITE,BLACK);
    display.print("SSID: "); 
    display.println(ssid); 
    display.print("Password: "); 
    display.println(pass);
    display.println("Server erreichbar unter: ");
    display.println(accessPointIP);
  }
  else {
    display.println("WiFi Test fehlgeschlagen!"); 
  }
  display.display();  

}

void displayUptimeAndSDCard(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE,BLACK);
  display.print("Zeit seit dem Hochfahren:"); 
  display.println(millis());
  display.print("SD Karte: "); 
  if(!sdTestPassed) {
    display.println("Keine SD Karte eingesteckt"); 
  }
  else{
    display.print(fileCounter);
    display.println(" Dateien gefunden.");
  }
  display.display();
}
void displayI2CCount() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE,BLACK);
  display.println("Suche nach Geraeten..");
  for (int i = 0; i < deviceCount; i++) {
    display.printf("0x%02X", deviceAddresses[i]);
    if(i != deviceCount - 1) {
      display.print(", "); 
    }
  }
  display.display();
}

void countI2cDevices () {
  byte error, address;
    Serial.println("Scannen nach I2C Geraeten ...");
  for(address = 0x01; address < 0x7f; address++){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0){
      Serial.printf("I2C Geraet gefunden an 0x%02X\n", address);
      deviceAddresses[deviceCount++] = address;
    } else if(error != 2){
      Serial.printf("Fehler %d an 0x%02X\n", error, address);
    }
  }
  if (deviceCount == 0){
    Serial.println("Kein I2C Geraet gefunden!");
  }
}

void sdTest(){
  // enable SD module
  pinMode(SD_ENABLE, OUTPUT);
  digitalWrite(SD_ENABLE,LOW);
  SPIClass sdspi = SPIClass();
  sdspi.begin(VSPI_SCLK,VSPI_MISO,VSPI_MOSI,VSPI_SS);
  if(!SD.begin(VSPI_SS,sdspi)){
        Serial.println("Keine SD Karte gefunden!");
        return;
    }

  if(!SD.begin(VSPI_SS,sdspi)){
    Serial.println("Keine SD Karte gefunden!");
    return; 
  }
  uint8_t cardType = SD.cardType(); 
  
  if (cardType == CARD_NONE){
    Serial.println("Keine SD Karte gefunden"); 
    return;
  }

  Serial.println("SD Karte erfolgreich initiiert!");
  File root = SD.open("/");
  // Lese alle Dateien im Stammverzeichnis und gebe ihre Namen aus
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      // Es gibt keine weiteren Dateien
      break;
    }
    if(entry.name() != "System Volume Information") {
      fileCounter++;
      Serial.print("Datei gefunden: ");
      Serial.println(entry.name());
    }
    entry.close();

  }
  sdTestPassed = true; 
  return; 
}

void mpuTest() {
  Wire1.begin();

  if (!mpu.begin(0x68, &Wire1)) {
    Serial.println("MPU6050 Chip wurde nicht gefunden");
    return;
  }

  Serial.println("MPU6050 gefunden!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.getEvent(&a, &g, &temp);

  Serial.println("Beschleunigungswerte:");
  Serial.print(a.acceleration.x);
  Serial.print(" ");
  Serial.print(a.acceleration.y);
  Serial.print(" ");
  Serial.println(a.acceleration.z);

  if (a.acceleration.x == 0 && a.acceleration.y == 0 && a.acceleration.z == 0) {
    return;
  }

  mpuTestPassed = true;
}

void pdTest() {
  pinMode(PD_ENABLE, OUTPUT);
  analogReadResolution(12);
  digitalWrite(PD_ENABLE, HIGH);

  int count = 0;
  int totalValues = 0;

  while (count < 20) {
    uint16_t value = analogRead(PD_SENSE);
    totalValues += value;
    count++;
  }
  pdTestValue = totalValues / count;
  if (pdTestValue == 0) {
    Serial.println("Photodiode-Test nicht bestanden!!");
    return;
  }

  Serial.println("Photodiode Test erfolgreich!");
  pdTestPassed = true;
}

void wifiTest() {
  if (!WiFi.softAP(ssid, pass)) {
    Serial.println("WiFi-Test fehlgeschlagen.");
    return;
  }

  accessPointIP = WiFi.softAPIP();
  server.begin();
  Serial.println("Gerät erreichbar unter: ");
  Serial.print("AP IP Adresse: ");
  Serial.println(accessPointIP);
  wifiTestPassed = true;

}

void returnTestPage(WiFiClient testClient) {
  String currentLine = "";                // make a String to hold incoming data from the client
  Serial.println("Client verbunden");
  while(testClient.connected()){
    if(testClient.available()){
      char c = testClient.read();
      Serial.write(c);
      if( c == '\n') {
        if (currentLine.length() == 0) {
          testClient.println("HTTP/1.1 200 OK");
          testClient.println("Content-type:text/html");
          testClient.println();

          // Willkommensnachricht
            testClient.println("<h1>Willkommen zum senseBox MCU S2 (ESP32-S2)-Test!(Test V1.0)</h1>");
            // Ergebnisse des Sensortests
            testClient.println("<h2>Angeschlossene Sensoren</h2>");
            testClient.println("<div>");
            // Angeschlossene I2C Geräte
            testClient.print("<p>Angeschlossene I2C Geraete:");
            for (int i = 0; i < deviceCount; i++) {
              testClient.printf("0x%02X", deviceAddresses[i]);
              if(i != deviceCount - 1)
              {
                testClient.print(", "); 
              }
            }
            testClient.println("</p>");
            // MPU 

            testClient.print("<p> Beschleunigungssensor (X, Y, Z):");
            if(mpuTestPassed){
              testClient.print(a.acceleration.x);
              testClient.print(" ");
              testClient.print(a.acceleration.y);
              testClient.print(" ");
              testClient.print(a.acceleration.z);
              testClient.println("</p>");

            }
            else {
              testClient.println("Test fehlgeschlagen! </p>");
            }
            // PD 
            testClient.print("<p> Photodiode:"); 
            if(pdTestPassed){
              testClient.print(pdTestValue);
              testClient.print(" Spannungsabhaengige Lichtintensitaet");
              testClient.println("</p>");
            }
            else {
              testClient.println("Test fehlgeschlagen! </p>");
            }

            testClient.print("<p>SD Karte: ");
            if (sdTestPassed) {
              testClient.print(fileCounter);
              testClient.println(" Dateien auf der SD Karte </p>");
            }
            else {
              testClient.print("Keine SD Karte angeschlossen");
              testClient.println("</p>");
            }

            // 
            testClient.print("<p>Vergangene Zeit seit dem Hochfahren: "); 
            testClient.print(getUptimeInSeconds());
            testClient.print(" Sekunden");
            testClient.println("</p>");
            testClient.println("</div>");

              // Schaltflächen für RGB-LED
            testClient.println("<h2>RGB-LED-Steuerung:</h2>");
            testClient.println("<button><a href=\"/R\"> LED Rot faerben</a></button>");
            testClient.println("<button><a href=\"/Y\"> LED Gelb faerben</a></button>");
            testClient.println("<button><a href=\"/G\"> LED Gruen faerben</a></button>");
            testClient.println();
            testClient.println();

            break;
        }
        else {
          currentLine = "";
        }
       }
       else if (c != '\r') 
       {
        currentLine += c;
       }
        // Check to see if the client request was "GET /R" or "GET /Y" or "GET /G":
        if (currentLine.endsWith("GET /R")) {
          rgb_led.setPixelColor(0, rgb_led.Color(255, 0, 0));
        }
        if (currentLine.endsWith("GET /Y")) {
           rgb_led.setPixelColor(0, rgb_led.Color(255, 150, 0));
        }
        if (currentLine.endsWith("GET /G")) {
            rgb_led.setPixelColor(0, rgb_led.Color(0,255,0));
        }
        rgb_led.show();
    }
  }
    testClient.stop();
    Serial.println("Client disconnected");
}


void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.print("senseBox MCU S2 (ESP32-S2) ");
  Serial.print("Test Version: ");
  Serial.println(testVersion);
  rgb_led.begin();
  rgb_led.setBrightness(30);

  mpuTest();
  pdTest();
  wifiTest();
  sdTest();
  displayCheck();
  if (mpuTestPassed && pdTestPassed && wifiTestPassed) {
    // Grün
    rgb_led.setPixelColor(0, rgb_led.Color(0,255,0));
  } else if (!mpuTestPassed && !pdTestPassed && !wifiTestPassed) {
    // Rot
    rgb_led.setPixelColor(0, rgb_led.Color(255, 0, 0));
  } else {
    // Gelb
    rgb_led.setPixelColor(0, rgb_led.Color(255, 150, 0));
  }
  rgb_led.show();
  countI2cDevices();
  Serial.println("-------------------");

}

void loop() {
  time_startInterval = millis();
  WiFiClient client = server.available();
  if (client){
    returnTestPage(client);
  }
  if(displayTestPassed){
    switch(displayPage) {
      case 0: 
        displayTestResults();
        break;
      case 1: 
        displayI2CCount(); 
        break;
      case 2:
        // uptime sd card
        displayUptimeAndSDCard();
        break; 
      case 3: 
        displayWifiStatus();
      default: 
        break;
    }
  }
  // change display page smart delay 
  if(time_startInterval > time_actualInterval + intervalInterval){
    time_actualInterval = millis(); 
    if(displayPage == 3){
      displayPage = 0; 
    }
    else {
      displayPage++;
    }
  }
}