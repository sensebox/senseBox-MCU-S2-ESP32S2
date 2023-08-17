// Sensor connected at UART

#include <SdsDustSensor.h> // http://librarymanager/All#Nova_Fitness_Sds_dust_sensors_library
SdsDustSensor sds(Serial1);

void setup() {
Serial.begin(9600);
Serial1.begin(115200,SERIAL_8N1, RX, TX);
sds.begin();
sds.setQueryReportingMode();
}


void loop() {
PmResult pm = sds.queryPm();
Serial.println(pm.pm10);
}