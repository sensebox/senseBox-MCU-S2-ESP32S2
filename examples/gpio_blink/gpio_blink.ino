
#define IO_ENABLE 8

void setup() {
  // put your setup code here, to run once:

  
  pinMode(IO_ENABLE, OUTPUT);
  digitalWrite(IO_ENABLE,LOW);

  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  digitalWrite(6,HIGH);
}

bool state = false;
void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(6, !state);
  digitalWrite(7, state);
  state = !state;
  
  delay(1000);
}
