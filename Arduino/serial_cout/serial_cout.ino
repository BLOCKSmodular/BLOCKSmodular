

//arduino m0
void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
}

void loop() {
    int data;
    if(Serial.available()>0){
      data = Serial.read();
      Serial.write(data);
    }
    delay(10);
}
