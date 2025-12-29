HardwareSerial UART(1);

void setup() {
  Serial.begin(115200);
  UART.begin(115200, SERIAL_8N1, 32, 33);
  delay(1000);
  Serial.println("READY");
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    UART.println(msg);
    Serial.println("Sent: " + msg);
  }

  if (UART.available()) {
    String r = UART.readStringUntil('\n');
    r.trim();
    Serial.println("RX: " + r);
  }
}

com different 
----------------------------------------------------


  HardwareSerial UART(1);

void setup() {
  Serial.begin(115200);
  UART.begin(115200, SERIAL_8N1, 16, 17);
  delay(1000);
  Serial.println("READY");
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    UART.println(msg);
    Serial.println("Sent: " + msg);
  }

  if (UART.available()) {
    String r = UART.readStringUntil('\n');
    r.trim();
    Serial.println("RX: " + r);
  }
}

com different 

