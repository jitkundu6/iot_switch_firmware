const int serialBaudRate = 115200;
const int sw[8] = {15,2,4,5,18,19,21,22};
bool value = true;

void setup() {
  Serial.begin(serialBaudRate);
  // initialize the I/O pins as outputs iterate over the pins:
  for (int thisPin = 0; thisPin < 8; thisPin++) {
    // initialize the output pins:
    pinMode(sw[thisPin], OUTPUT);
    // take the pin LOW to ensure that the LEDS are off:
    digitalWrite(sw[thisPin], LOW);
  }
}

void loop() { 
  // led/switch control here
  for (int i = 0; i < 8; i++) {
    digitalWrite(sw[i], value);
    delay(500);
  }
  
  delay(2000);
  value = !value;
}
