#define IO_ENABLE 8

int analogVal = 0;
int analogmV = 0;

void setup(){
    //Enable IO and Vout
    pinMode(IO_ENABLE,OUTPUT);
    digitalWrite(IO_ENABLE,LOW);

    pinMode(6,INPUT);
    Serial.begin(9600);
    analogSetAttenuation(ADC_11db);
}

void loop(){
    analogVal = analogRead(6);
    Serial.println("RAW Value: " + String(analogVal));
    analogmV = analogReadMilliVolts(6);
    Serial.println("Millivolts: " + String(analogmV));
    delay(1);
}