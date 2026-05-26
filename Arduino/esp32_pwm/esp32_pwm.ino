const int ledPin = A19;

void setup() {
  // put your setup code here, to run once:
  ledcAttach(ledPin,12800,8); 
  ledcAttachChannel(ledPin,12800,8,0);
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint8_t brightness = 0;
  static int diff = 1;
  ledcWrite(ledPin,brightness);
  if(brightness == 0){
    diff = 1;
  }else if(brightness == 255){
    diff = -1;
  }
  brightness += diff;
  delay(10);
}
