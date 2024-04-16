#include <ArduinoJson.h>
#include <Adafruit_HDC1000.h>
#include <sps30.h>
#include <Adafruit_DPS310.h>
#include <HttpClient.h>  // Install the ArduinoHttpClient library via Library Manager
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <hydreon.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "ids.h"
#include <Adafruit_LTR329_LTR303.h>
#include <Adafruit_VEML6070.h>
#include "esp_system.h"

const int wdtTimeout = 60000 * 2;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;


// LTR
bool lightsensortype = -1;  //0 for tsl - 1 for ltr
unsigned char gain = 1;
unsigned char integrationTime = 0;
unsigned char measurementRate = 3;
// SPS30
uint32_t auto_clean_days = 4;
const long intervalsps = 1000;
unsigned long time_startsps = 0;
unsigned long time_actualsps = 0;
// Upload info
char serverAddress[] = "ingress.opensensemap.org";  // server address
int port = 80;
WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;
const char *ap_ssid = "senseBox-AccessPoint";
const char *ap_password = "123456789";

AsyncWebServer APserver(80);
// Sensr variables
Adafruit_HDC1000 hdc = Adafruit_HDC1000();
Adafruit_DPS310 dps;
HYDREON rg_15(Serial1);
Adafruit_VEML6070 veml = Adafruit_VEML6070();
Adafruit_LTR329 ltr = Adafruit_LTR329();
SPIClass sdspi = SPIClass();
// value variables
sensors_event_t temp_event, pressure_event;
float temperature, humidity, uv, lux;
struct sps30_measurement m;
static unsigned long previousMillis = 0;




const int bufferSize = 50;


String jsonString;
String ssidString;
String passwordString;

bool wifiConnected = false; 
bool sdMounted = false;

void ARDUINO_ISR_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}

void initSensors() {
  if (!hdc.begin()) {
    Serial.println("HDC error");
  }  // Initialize the DPS310 sensor
  if (!dps.begin_I2C(0x76)) {
    Serial.println("DPS310 error");
  } else {
    dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
  }  
    // Initialize the VEML6070 sensor
  veml.begin(VEML6070_2_T);
  if (veml.readUV() == -1 || veml.readUV() == 65535) {
    Serial.println("VEML6070 error");
  }

  // Initialize the RG-15 sensor
  rg_15.begin();
  rg_15.readAllData();
  if (rg_15.getAccumulation() == -1) {
    Serial.println("RG-15 error");
  }

    if (!ltr.begin()) {
      Serial.println("LTR error");
  }
  else {
      ltr.setGain(LTR3XX_GAIN_2);
      ltr.setIntegrationTime(LTR3XX_INTEGTIME_100);
      ltr.setMeasurementRate(LTR3XX_MEASRATE_50);
  }
  if (veml.readUV() == -1 || veml.readUV() == 65535){
    Serial.println("VEML6070 error");
  }
  
  sensirion_i2c_init();
    int16_t ret;

  while (sps30_probe() != 0) {
    Serial.print("SPS sensor probing failed\n");
    delay(500);
  }
  Serial.print("SPS sensor probing successful\n");

  ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
  if (ret) {
    Serial.print("error setting the auto-clean interval: ");
    Serial.println(ret);
  }

  ret = sps30_start_measurement();
  if (ret < 0) {
    Serial.print("error starting measurement\n");
  }


}


void setSensors() {
  getSPS30Data();
  dps.getEvents(&temp_event, &pressure_event);
  temperature = hdc.readTemperature();  
  humidity = hdc.readHumidity();
  rg_15.readAllData();
  uv =  veml.readUV();
  uint16_t ch1 = 0;
  uint16_t ch2 = 0;
  ltr.readBothChannels(ch1, ch2);
    lux = ch1;
}


void getSPS30Data() {
  uint16_t data_ready;
  int16_t ret;
  int attempts = 0;
  do {
    ret = sps30_read_data_ready(&data_ready);
    if (ret < 0) {
      Serial.print("error reading data-ready flag: ");
      Serial.println(ret);
    } else if (!data_ready) {
      Serial.print("data not ready, no new measurement available\n");
      attempts++;
      if (attempts >= 5) {
        Serial.println("Exceeded maximum attempts. Stopping.");
        break;
      }
    } else {
      break;
    }
    delay(100); /* retry in 100ms */
  } while (1);
  ret = sps30_read_measurement(&m);
}


String printSensorValuesJSON() {
  // Create a JSON document
  StaticJsonDocument<256> doc;

  // Add sensor values to the JSON document
  doc[TEMP_ID] = roundf(temperature * 10) / 10.0 ;
  doc[HUMI_ID] = roundf(humidity *10) / 10.0;
  doc[PRESSURE_ID] = roundf(pressure_event.pressure * 10) / 10.0;
  doc[LUX_ID] = roundf(lux * 10) / 10.0;
  doc[UV_ID] = roundf(uv * 10) / 10.0 ;
  doc[PM1_ID] = roundf(m.mc_1p0 * 10) / 10.0; // Rundet auf eine Nachkommastelle
  doc[PM25_ID] = roundf(m.mc_2p5 * 10) / 10.0; // Rundet auf eine Nachkommastelle
  doc[PM4_ID] = roundf(m.mc_4p0 * 10) / 10.0; // Rundet auf eine Nachkommastelle
  doc[PM10_ID] = roundf(m.mc_10p0 * 10) / 10.0; // Rundet auf eine Nachkommastelle
  doc[RAINEVENT_ID] = roundf(rg_15.getEventAccumulation() * 10) /10.0;
  doc[RAININTENSITY_ID] = roundf(rg_15.getRainfallIntensity() * 10) / 10.0 ;
  doc[RAINTOTALACC_ID] = roundf(rg_15.getTotalAccumulation() * 10) / 10.0;

  // Serialize the JSON document to a string
  serializeJson(doc, jsonString);

  return jsonString;
  // Send JSON data to OpenSenseMap API
}


void sendToOpenSenseMap(String data) {
  // Überprüfen Sie den Status der WLAN-Verbindung
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No WLAN connection available. Cannot send data.");
    return; // Beenden Sie die Funktion, wenn keine WLAN-Verbindung besteht
  }
  // Prepare headers
  String contentType = "application/json";
  String url = "/boxes/" + String(SENSEBOX_ID) + "/data";
  Serial.println("Making POST request");
  Serial.println(contentType);
  Serial.println(url);
  Serial.println(data);
  client.post(url, contentType, data);

  // Read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}


void readWiFi(fs::FS &fs, const char *path) {
  size_t maxLen = 100;
    Serial.println("Reading WiFi Information:");
  File file = fs.open("/wifi.cfg");
  String tmp;
  if (file) {
    while (file.available()) {
      char c = file.read();
        tmp += c;
    }
    file.close();
  }
  else {
    Serial.println("Failed to open file for reading");
  }
  tmp.trim();
  tmp = decryptString(tmp);
  splitStringByComma(tmp, ssidString, passwordString);

  }

void updateWiFiConnection(fs::FS &fs, String newSSID, String newPassword) {
  // Versuche, die Verbindung zu aktualisieren, ohne die aktuelle Verbindung zu trennen
   Serial.println("Updating WiFi Information:");
   Serial.print("New SSID: ");
    Serial.println(newSSID);
    Serial.print("New Password: ");
    Serial.println(newPassword);
  newSSID.trim();
  String newWifi = newSSID + "," + newPassword;
  newWifi = encryptString(newWifi);
  char newWifiArray[newWifi.length() + 1];
  newWifi.toCharArray(newWifiArray, newWifi.length() + 1);
  Serial.println(newWifiArray);
  File wifiFile = fs.open("/wifi.cfg", FILE_WRITE);
  if (!wifiFile) {
    Serial.println("Failed to open file for writing");
    return;
  }
  wifiFile.println(newWifi);
  wifiFile.close();
  Serial.println("Updated WiFi Information");
}


void setup() {

  // setup watchdog 
  timer = timerBegin(0, 80, true);                   //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);   //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, false);  //set time in us
  timerAlarmEnable(timer);                           //enable interrupt

  // Enable SD 
  pinMode(SD_ENABLE, OUTPUT);
  digitalWrite(SD_ENABLE, LOW);
  delay(2000);
  sdspi.begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);
  if (!SD.begin(VSPI_SS, sdspi)) {
    Serial.println("Card Mount Failed");
  }
  else {
    sdMounted = true;
  }
  WiFi.softAP(ap_ssid, ap_password);
  // Set WiFi configs based on SD readings
  readWiFi(SD, "/wifi.cfg");
  // Setze Routen für den Webserver
  APserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Lese den aktuellen WiFi-Status
    String html = buildHTMLString();
    // Sende die HTML-Seite an den Cient
    request->send(200, "text/html", html);
  });

  // Route für das Speichern der SSID und des Passworts
  APserver.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Lese die Werte von den Formulareingaben
    // Versuche, zur neuen SSID zu verbinden
    updateWiFiConnection(SD, request->arg("ssid"), request->arg("password"));
    // Antworte dem Client mit einer Bestätigungsseite
    String saveHtml = buildSaveString();
    request->send(200, "text/html", saveHtml);
    delay(5000);
    esp_restart();
  });
  
  APserver.begin();
  Serial.println("Init sensors");
  initSensors();
  Serial.println("Set sensors");
  setSensors();
  Serial.println("Connecting to WiFi");
  Serial.print("SSID: ");
  Serial.println(ssidString);
  Serial.print("Password: ");
  Serial.println(passwordString);
  WiFi.begin(ssidString.c_str(), passwordString.c_str());

  // Warte, bis die Verbindung hergestellt ist
  unsigned long previousMillisWifi = 0;
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long currentMillisWifi = millis();
    // Überprüfe, ob das Intervall vergangen ist
    if (currentMillisWifi - previousMillisWifi >= 5000) {
      // Speichere die aktuelle Zeit als Referenz für das nächste Intervall
      Serial.println("Connecting to WiFi...");
      previousMillisWifi = currentMillisWifi;
      // Führe die Verbindungskontrolle aus
    }
  }
  Serial.println("Connected to WiFi");
  wifiConnected= true;
}

void loop() {

  // Check if 60 seconds have passed
  const long interval = 300000 ;  // 5 Minutes
  unsigned long currentMillis = millis();
  timerWrite(timer, 0);  //reset timer (feed watchdog)

  if (currentMillis - previousMillis >= interval || previousMillis == 0) {
    wifiConnected = WiFi.status() == WL_CONNECTED;
    setSensors();
    // Print sensor values in JSON format
    jsonString = printSensorValuesJSON();
    sendToOpenSenseMap(jsonString);
    // Reset the timer
    previousMillis = currentMillis;
  }

  // Add any other code for your application here
}