/*
Pin mapping (vl53l8cx -> sensebox):
5VO -> 5V (ext. on)
PWREN -> IO2 (GPIOA)
MCLK_SCL -> SCL (I2C)
MOSI_SDA -> SDA (I2C)
LPn -> IO4 (GPIOB)
SPI_I2C_N -> GND (I2C)
GPIO1 -> IO3 (GPIOA)
GND -> GND (GPIOA)
Pin mapping (CJMCU-8*8 -> sensebox):
DOUT -> IO6 (GPIOC)
+5V -> 5V (ext. 5V)
GND -> GND (ext. 5V)
*/
#include <Wire.h>
#include <vl53l8cx.h>
#include <math.h>

#define DEV_I2C Wire
#define SerialPort Serial

#define LPN_PIN 4
#define I2C_RST_PIN -1
#define PWREN_PIN 2

#include <SD.h>
#include "SPI.h"

# include <Adafruit_NeoMatrix.h>
# define RGBMatrixPin 6
Adafruit_NeoMatrix RGBMatrix = Adafruit_NeoMatrix(8, 8, RGBMatrixPin, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800);

File file;
String dataStr = "";

void print_result(VL53L8CX_ResultsData *Result);
void clear_screen(void);
void handle_cmd(uint8_t cmd);
void display_commands_banner(void);

long measurements = 0;         // Used to calculate actual output rate
long measurementStartTime = 0;

// Components.
VL53L8CX sensor_vl53l8cx_top(&DEV_I2C, LPN_PIN, I2C_RST_PIN);

bool EnableAmbient = false;
bool EnableSignal = false;
uint8_t res = VL53L8CX_RESOLUTION_4X4;
char report[256];
int start = 0;


void receiveEvent(int bytes) {
  Serial.print("received: ");
  start = Wire.read();    // read one character from the I2C
  Serial.println(start);
}

/* Setup ---------------------------------------------------------------------*/
void setup()
{
  // Initialize serial for output.
  Serial.begin(9600);
  // delay(2000);
  while(!Serial) ;

  uint8_t status;

  Serial.println("Initializing sensor...");

  // Enable PWREN pin if present
  if (PWREN_PIN >= 0) {
    pinMode(PWREN_PIN, OUTPUT);
    digitalWrite(PWREN_PIN, HIGH);
    delay(10);
  }
  
  Serial.println("I2C Initialized");

  DEV_I2C.begin(39,40);
  DEV_I2C.setClock(1000000); //Sensor has max I2C freq of 1MHz

  sensor_vl53l8cx_top.set_i2c_address(0x51); // the sensor shares an I2C address with the LTR329 so we need to change it

  // Configure VL53L8CX component.
  sensor_vl53l8cx_top.begin();
  Serial.println("Sensor library started");
  sensor_vl53l8cx_top.init();
  Serial.println("Sensor initialized");

  // sensor_vl53l8cx_top.vl53l8cx_set_ranging_mode(VL53L8CX_RANGING_MODE_AUTONOMOUS);
  // if (status) {
  //   snprintf(report, sizeof(report), "vl53l5cx_set_ranging_mode failed, status %u\r\n", status);
  //   SerialPort.print(report);
  // }
  sensor_vl53l8cx_top.set_ranging_frequency_hz(30);
  if (status) {
    snprintf(report, sizeof(report), "vl53l8cx_set_ranging_frequency_hz failed, status %u\r\n", status);
    SerialPort.print(report);
  }
  Serial.println("ranging mode and frequency was set");
  delay(3000);

  // Start Measurements
  sensor_vl53l8cx_top.start_ranging();

  toggle_resolution();
  toggle_signal_and_ambient();

  Serial.println("Success");

  RGBMatrix.setBrightness(10);
  // RGBMatrix starten
  RGBMatrix.begin();
  
  // digitalWrite(8,HIGH);
}

void loop()
{
  VL53L8CX_ResultsData Results;
  uint8_t NewDataReady = 0;
  uint8_t status;

  do {
    status = sensor_vl53l8cx_top.check_data_ready(&NewDataReady);
  } while (!NewDataReady);

  if ((!status) && (NewDataReady != 0)) {
    status = sensor_vl53l8cx_top.get_ranging_data(&Results);
    print_result(&Results);
  }
}

void print_result(VL53L8CX_ResultsData *Result)
{
  int8_t i, j, k, l;
  uint8_t zones_per_line;
  uint8_t number_of_zones = res;

  zones_per_line = (number_of_zones == 16) ? 4 : 8;
  dataStr = "";
  //convert floats to string and assemble c-type char string for writing:
  // dataStr += String(millis()-measurementStartTime) + ":";//add it onto the end

  for (j = 0; j < number_of_zones; j += zones_per_line)
  {
    for (l = 0; l < VL53L8CX_NB_TARGET_PER_ZONE; l++)
    {
      for (k = (zones_per_line - 1); k >= 0; k--)
      {
        //perform data processing here...
        if((long)Result->target_status[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l] ==255){
          dataStr += String(5000) + ",";
          RGBMatrix.drawPixel((j+1)/8, k, RGBMatrix.Color(0, 0, 0));
        } else {
          long distance = (long)Result->distance_mm[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l];
          int maxDist = distance;
          if (maxDist > 2000) {
            maxDist = 2000;
          }
          int colVal = map(maxDist,0,2000,1,360);
          dataStr += String(distance) + ",";
          setLedColorHSV(colVal,1,1,(j+1)/8, k);
        }
        dataStr += String((long)Result->target_status[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l]) + ",";
        dataStr += String((long)Result->signal_per_spad[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l]) + ",";
        dataStr += String((long)Result->ambient_per_spad[j+k]) + "; ";
      }
    }
  }
  dataStr += "\n";
  Serial.print(dataStr);
  RGBMatrix.show();
}

void toggle_resolution(void)
{
  sensor_vl53l8cx_top.stop_ranging();

  switch (res)
  {
    case VL53L8CX_RESOLUTION_4X4:
      res = VL53L8CX_RESOLUTION_8X8;
      break;

    case VL53L8CX_RESOLUTION_8X8:
      res = VL53L8CX_RESOLUTION_4X4;
      break;

    default:
      break;
  }
  sensor_vl53l8cx_top.set_resolution(res);
  sensor_vl53l8cx_top.start_ranging();
}

void toggle_signal_and_ambient(void)
{
  EnableAmbient = (EnableAmbient) ? false : true;
  EnableSignal = (EnableSignal) ? false : true;
}

void clear_screen(void)
{
  snprintf(report, sizeof(report),"%c[2J", 27); /* 27 is ESC command */
  SerialPort.print(report);
}

void display_commands_banner(void)
{
  snprintf(report, sizeof(report),"%c[2H", 27); /* 27 is ESC command */
  SerialPort.print(report);

  Serial.print("53L7A1 Simple Ranging demo application\n");
  Serial.print("--------------------------------------\n\n");

  Serial.print("Use the following keys to control application\n");
  Serial.print(" 'r' : change resolution\n");
  Serial.print(" 's' : enable signal and ambient\n");
  Serial.print(" 'c' : clear screen\n");
  Serial.print("\n");
}

void handle_cmd(uint8_t cmd)
{
  switch (cmd)
  {
    case 'r':
      toggle_resolution();
      clear_screen();
      break;

    case 's':
      toggle_signal_and_ambient();
      clear_screen();
      break;

    case 'c':
      clear_screen();
      break;

    default:
      break;
  }
}

void setLedColorHSV(int h, double s, double v, int x, int y) {
  //this is the algorithm to convert from RGB to HSV
  double r=0; 
  double g=0; 
  double b=0;

  double hf=h/60.0;

  int i=(int)floor(h/60.0);
  double f = h/60.0 - i;
  double pv = v * (1 - s);
  double qv = v * (1 - s*f);
  double tv = v * (1 - s * (1 - f));

  switch (i)
  {
  case 0: //rojo dominante
    r = v;
    g = tv;
    b = pv;
    break;
  case 1: //verde
    r = qv;
    g = v;
    b = pv;
    break;
  case 2: 
    r = pv;
    g = v;
    b = tv;
    break;
  case 3: //azul
    r = pv;
    g = qv;
    b = v;
    break;
  case 4:
    r = tv;
    g = pv;
    b = v;
    break;
  case 5: //rojo
    r = v;
    g = pv;
    b = qv;
    break;
  }

  //set each component to a integer value between 0 and 255
  int red=constrain((int)255*r,0,255);
  int green=constrain((int)255*g,0,255);
  int blue=constrain((int)255*b,0,255);

  RGBMatrix.drawPixel(x,y, RGBMatrix.Color(red, green, blue));
}