#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

#define MODEM_RX 32
#define MODEM_TX 33
#define CONTROL_PIN 4

const char apn[]  = "airteliot.com";
const char user[] = "";
const char pass[] = "";

HardwareSerial SerialAT(2);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("ESP32 + SIM800 START");

  // GPIO 4 HIGH
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH);

  // Start SIM800 UART
  SerialAT.begin(9600, SERIAL_8N1, 32, 33);
  delay(3000);

  // ---------- BASIC AT TEST ----------
  Serial.println("Checking AT...");
  SerialAT.println("AT");
  delay(1000);
  readSIM800();

  // ---------- MODEM RESTART ----------
  Serial.println("Restarting modem...");
  if (!modem.restart()) {
    Serial.println("❌ SIM800 not responding");
    return;
  }
  Serial.println("✅ SIM800 Restarted");

  // ---------- NETWORK CHECK ----------
  Serial.println("Waiting for network...");
  if (!modem.waitForNetwork(60000L)) {
    Serial.println("❌ Network failed");
    return;
  }
  Serial.println("✅ Network OK");

  // ---------- SIGNAL ----------
  int csq = modem.getSignalQuality();
  Serial.print("Signal (CSQ): ");
  Serial.println(csq);

  // ---------- GPRS ----------
  Serial.println("Connecting to GPRS...");
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println("❌ GPRS failed");
    return;
  }
  Serial.println("✅ GPRS Connected");

  // ---------- HTTP TEST ----------
  const char server[] = "httpbin.org";
  const int port = 80;

  if (client.connect(server, port)) {
    client.println("GET /get?device=esp32&value=123 HTTP/1.1");
    client.println("Host: httpbin.org");
    client.println("Connection: close");
    client.println();

    Serial.println("📤 HTTP Data Sent");

    while (client.connected() || client.available()) {
      if (client.available()) {
        Serial.write(client.read());
      }
    }
    client.stop();
  } else {
    Serial.println("❌ Server connect failed");
  }

//   modem.gprsDisconnect();
//   Serial.println("GPRS Disconnected");
// }

void loop() {
  // ---------- AT COMMAND BRIDGE ----------
  // PC -> SIM800
  while (Serial.available()) {
    SerialAT.write(Serial.read());
  }

  // SIM800 -> PC
  while (SerialAT.available()) {
    Serial.write(SerialAT.read());
  }
}

// ---------- READ SIM800 RESPONSE ----------
void readSIM800() {
  while (SerialAT.available()) {
    Serial.write(SerialAT.read());
  }
}
