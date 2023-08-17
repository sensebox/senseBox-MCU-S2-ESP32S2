// SMT50 connected at GPIO A. Green temperature wire into IO3 and yellow humidity wire into IO2
#define TEMP_IO 3
#define HUMI_IO 2
#define IO_ENABLE 8


float getSMT50Temperature(int analogPin){
  int sensorValue = analogRead(analogPin);
    float voltage = sensorValue * (3.3 / 8190.0);
   return (voltage - 0.5) * 100;
}
float getSMT50Moisture(int analogPin){
  int sensorValue = analogRead(analogPin);
  float voltage = sensorValue * (3.3 / 8190.0);
  return (voltage * 50) / 3;
}


void setup() {
  Serial.begin(9600);
  //Enable IO and Vout
  pinMode(IO_ENABLE,OUTPUT);
  digitalWrite(IO_ENABLE,LOW);
  pinMode(TEMP_IO,INPUT);
  pinMode(HUMI_IO,INPUT);
  analogSetAttenuation(ADC_11db);
}

void loop() {
    Serial.print("Temperatur: ");
    Serial.println(getSMT50Temperature(TEMP_IO));
    Serial.print("Feuchte: ");
    Serial.println(getSMT50Moisture(HUMI_IO));
    delay(100);
}
