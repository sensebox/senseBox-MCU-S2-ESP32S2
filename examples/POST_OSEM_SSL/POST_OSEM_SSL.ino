/*
  senseBox Prototype SSL testing

  Submit temperature & humidity data to OSeM via SSL

  Hardwaresetup:
   senseBox MCU ESP32-S2-MINI Prototype
   senseBox Temperature & Humidity Sensor (HDC1080) on I2C (0x40)

  This code is in the public domain.
*/


#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_HDC1000.h> // http://librarymanager/All#Adafruit_HDC1000_Library


const char* ssid = "";
const char* password = "";

const long intervalInterval = 60000; //in milliseconds
long time_startInterval = 0;
long time_actualInterval = 0;
unsigned int strLength = 0;
static const uint8_t NUM_SENSORS = 2;

const char* SENSEBOX_ID = "";
const char* SENSOR_TEMP = "";
const char* SENSOR_HUMI = "";

const char* server = "ingress.opensensemap.org";

WiFiClientSecure client;
Adafruit_HDC1000 hdc = Adafruit_HDC1000();

// SHA1 fingerprint is broken. using root SDRG Root X1 valid until 04 Jun 2035 11:04:38 GMT
// ISRGRootX1.crt
const char* root_ca = \
                      "-----BEGIN CERTIFICATE-----\n" \
                      "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n" \
                      "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
                      "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n" \
                      "WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n" \
                      "ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n" \
                      "MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n" \
                      "h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n" \
                      "0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n" \
                      "A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n" \
                      "T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n" \
                      "B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n" \
                      "B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n" \
                      "KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n" \
                      "OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n" \
                      "jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n" \
                      "qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n" \
                      "rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n" \
                      "HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n" \
                      "hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n" \
                      "ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n" \
                      "3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n" \
                      "NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n" \
                      "ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n" \
                      "TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n" \
                      "jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n" \
                      "oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n" \
                      "4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n" \
                      "mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n" \
                      "emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n" \
                      "-----END CERTIFICATE-----\n" ;

// set ESP32 mode to station and connect to SSID
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("ok.");
  Serial.println(WiFi.localIP());
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
  //check if still connected 
  //ToDo: replace for energy saving mode
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection to WiFi lost. Reconnecting.");
    WiFi.disconnect();
    WiFi.reconnect();
    delay(5000); // wait 5s
    }

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
                PSTR("POST /boxes/%s/data HTTP/1.1\nHost: %s\nContent-Type: "
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
  initWiFi();
  delay(500);
  client.setCACert(root_ca);
  
  Wire.begin(39, 40);
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
