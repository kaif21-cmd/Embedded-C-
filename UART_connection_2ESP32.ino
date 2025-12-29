HardwareSerial UART(1);

#define LED_PIN 2   // Onboard LED lagi huhe he 

void setup() {
  Serial.begin(115200);
  UART.begin(115200, SERIAL_8N1, 32, 33); // pin decalare krdiye mene 

  pinMode(LED_PIN, OUTPUT); 
  digitalWrite(LED_PIN, LOW);

  Serial.println("READY");
  Serial.println("Send: 1 = ON , 2 = OFF");
}

void loop() {

  // ----- USB SERIAL -----
  if (Serial.available()) {
    char cmd = Serial.read();   // ⭐ STRING ki jagah CHAR

    if (cmd == '1') {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON");
      UART.println("1");
    }
    else if (cmd == '2') {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF");
      UART.println("2");
    }
  }

  // ----- UART SERIAL -----
  if (UART.available()) {
    char r = UART.read();

    if (r == '1') {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON (UART)");
    }
    else if (r == '2') {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF (UART)");
    }
  }
}
