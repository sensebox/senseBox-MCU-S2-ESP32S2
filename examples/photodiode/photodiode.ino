
#define PD_SENSE 14
#define PD_ENABLE 21

void setup() {
  // put your setup code here, to run once:
  pinMode(PD_ENABLE, OUTPUT);
  analogReadResolution(12);
  digitalWrite(PD_ENABLE, HIGH);
  
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  Serial.println(analogRead(PD_SENSE));
  delay(1000);
}
