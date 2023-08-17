#include <hcsr04.h>

#define TRIG_PIN 2
#define ECHO_PIN 3
#define IO_ENABLE 8

HCSR04 hcsr04(TRIG_PIN, ECHO_PIN, 20, 4000);

void setup(){  
  pinMode(IO_ENABLE, OUTPUT);
  digitalWrite(IO_ENABLE,LOW);
  Serial.begin(9600);
}

void loop() {
  Serial.println(hcsr04.ToString());

  delay(250);
}
