HardwareSerial UART(1);

#define LED_PIN 2   // Onboard LED

void setup() {
  Serial.begin(115200);

  // UART1 → RX = 16 , TX = 17
  UART.begin(115200, SERIAL_8N1, 32, 33);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("READY");
  Serial.println("Send 1 = ON (OTHER ESP32)");
  Serial.println("Send 2 = OFF (OTHER ESP32)");
}

void loop() {

  // ---------- PC → ESP32 → OTHER ESP32 ----------
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == '1') {
      UART.write('1');   // sirf dusre ko bhejo
      Serial.println("Sent: LED ON (OTHER ESP32)");
    }
    else if (cmd == '2') {
      UART.write('2');
      Serial.println("Sent: LED OFF (OTHER ESP32)");
    } 
  }

  // ---------- OTHER ESP32 → THIS ESP32 ----------
  if (UART.available()) {
    char r = UART.read();

    if (r == '1') {
      digitalWrite(LED_PIN, HIGH); 
      Serial.println("LED ON (FROM OTHER ESP32)");
    }
    else if (r == '2') {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF (FROM OTHER ESP32)");
    }
  }
}
