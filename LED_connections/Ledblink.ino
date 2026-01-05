int led = 13;  // LED pin (built-in LED on Arduino)

void setup() {
  pinMode(led, OUTPUT);   // set LED pin as output
}

void loop() {
  digitalWrite(led, HIGH); // LED ON
  delay(3000);             // wait 1 second
  digitalWrite(led, LOW);  // LED OFF
  delay(3000);             // wait 1 second
}
 
