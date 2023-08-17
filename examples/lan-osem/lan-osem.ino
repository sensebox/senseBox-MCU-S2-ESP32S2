/*
  senseBox Prototype SSL testing

  Submit temperature & humidity data to OSeM via Ethernet 

  Hardwaresetup:
   senseBox MCU ESP32-S2-MINI Prototype
   senseBox Lan Bee
   senseBox Temperature & Humidity Sensor (HDC1080) on I2C (0x40)

  This code is in the public domain.
*/

#include <Ethernet.h> // http://librarymanager/All#Ethernet
#include <Adafruit_HDC1000.h> // http://librarymanager/All#Adafruit_HDC1000_Library
#include <ArduinoBearSSL.h>
#include <EthernetUdp.h>
#include <NTPClient.h>

EthernetClient eclient;
BearSSLClient client(eclient);
EthernetUDP Udp;
NTPClient timeClient(Udp);

const long intervalInterval = 60000; //in milliseconds
long time_startInterval = 0;
long time_actualInterval = 0;
unsigned int strLength = 0;
static const uint8_t NUM_SENSORS = 2;

const char* SENSEBOX_ID = "";
const char* SENSOR_TEMP = "";
const char* SENSOR_HUMI = "";

const char* server = "ingress.opensensemap.org";


Adafruit_HDC1000 hdc = Adafruit_HDC1000();
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// set ESP32 mode to station and connect to SSID
void initEthernet() {
   pinMode(PIN_XB1_CS, OUTPUT); //pin 34 if not defined in Variants
  pinMode(41, OUTPUT);
  digitalWrite(41, LOW);
 Ethernet.init(PIN_XB1_CS);
// start the Ethernet connection:
Ethernet.begin(mac);
// give the Ethernet shield a second to initialize:
delay(1000);
}

typedef struct measurement {
  const char *sensorId;
  float value;
} measurement;

char buffer[750]; //might be too short for many phenomenons
measurement measurements[NUM_SENSORS];
uint8_t num_measurements = 0;
const int lengthMultiplikator = 35;

void addMeasurement(const char *sensorId, float value) {
  measurements[num_measurements].sensorId = sensorId;
  measurements[num_measurements].value = value;
  num_measurements++;
}

unsigned long getTime() {
  timeClient.update();
  return timeClient.getEpochTime();
}

void writeMeasurementsToClient() {
  // iterate throug the measurements array
  for (uint8_t i = 0; i < num_measurements; i++) {
    sprintf_P(buffer, PSTR("%s,%9.2f\n"), measurements[i].sensorId, measurements[i].value);
    // transmit buffer to client
    client.print(buffer);
    Serial.print(buffer);
  }
  // reset num_measurements
  num_measurements = 0;
}

void submitValues() {

  if (client.connected()) {
    client.stop();
    delay(1000);
  }
  bool connected = false;
  for (uint8_t timeout = 2; timeout != 0; timeout--) {
    Serial.println(F("\nconnecting..."));
    connected = client.connect(server, 443);
    if (connected == true) {
      // construct the HTTP POST request:
      sprintf_P(buffer,
                PSTR("POST /boxes/%s/data HTTP/1.1\nAuthorization: \nHost: %s\nContent-Type: "
                     "text/csv\nConnection: close\nContent-Length: %i\n\n"),
                     SENSEBOX_ID, server, (num_measurements * lengthMultiplikator) - 1);
      // send the HTTP POST request:
      Serial.print(buffer);
      client.print(buffer);
      // send measurements
      writeMeasurementsToClient();
      // send empty line to end the request
      client.println();

      // read server answer
      while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
          Serial.println("headers received");
          break;
        }
      }

      // read them and print incoming bytes:
      while (client.available()) {
        char c = client.read();
        Serial.write(c);
      }
      Serial.println();
      client.stop();

      num_measurements = 0;
      break;
    }
    delay(1000);
  }
}

void setup()
{
  Serial.begin(115200);
  delay(10);
  //while (!Serial);

  //init WiFi and set certificate
  initEthernet();
  delay(500);
  timeClient.begin();
ArduinoBearSSL.onGetTime(getTime);
  Serial.println(getTime());
  
  //Wire.begin(39, 40);
  delay(500);
  hdc.begin(0x40);
}


void loop()
{
  time_startInterval = millis();

  if (time_startInterval > time_actualInterval + intervalInterval) {
    time_actualInterval = millis();
    addMeasurement(SENSOR_TEMP, hdc.readTemperature());
    addMeasurement(SENSOR_HUMI, hdc.readHumidity());
    submitValues();
  }
}
