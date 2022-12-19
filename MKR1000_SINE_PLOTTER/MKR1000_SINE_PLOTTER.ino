
void setup() {
  Serial.begin(9600);
  pinMode(34,OUTPUT);
}

void loop() {
  Serial.println(analogRead(34));
  //delay(100);
}