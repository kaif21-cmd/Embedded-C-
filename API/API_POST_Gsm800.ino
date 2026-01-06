#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <ArduinoJson.h>

// ---------- PINS ----------
#define MODEM_RX 32
#define MODEM_TX 33
#define CONTROL_PIN 4

// ---------- APN ----------
const char apn[]  = "airteliot.com";
const char user[] = "";
const char pass[] = "";

// ---------- SERVER ----------
const char* server = "customer.enggenv.com";
const int   port   = 80;
const char* path   = "/postdata.php";

// ---------- GSM ----------
HardwareSerial SerialAT(2);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH);

  SerialAT.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  Serial.println("Starting SIM800...");

  if (!modem.restart()) {
    Serial.println("Modem not responding");
    return;
  }

  if (!modem.waitForNetwork(60000L)) {
    Serial.println("Network failed");
    return;
  }

  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println("GPRS failed");
    return;
  }

  Serial.println("GPRS Connected");
}

void loop() {

  // ---------- JSON (SAME AS WIFI CODE) ----------
  StaticJsonDocument<256> doc;

  doc["spm"] = random(20, 41);  // 20 to 40
  doc["lg"]  = "30";
  doc["lt"]  = "0";
  doc["bv"]  = "0";
  doc["ts_client"] = "2025-10-10 16:39:09";
  doc["id"] = "ENE04912";

  String jsonData;
  serializeJson(doc, jsonData);

  Serial.println("POST JSON:");
  Serial.println(jsonData);

  // ---------- HTTP POST ----------
  if (!client.connect(server, port)) {
    Serial.println("Server connection failed");
    delay(5000);
    return;
  }

  client.println("POST " + String(path) + " HTTP/1.1");
  client.println("Host: " + String(server));
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonData.length());
  client.println("Connection: close");
  client.println();
  client.println(jsonData);

  Serial.println(" Data Sent");

  // ---------- RESPONSE ----------
  while (client.connected() || client.available()) {
    if (client.available()) {
      Serial.write(client.read());
    }
  }

  client.stop();
  Serial.println("\n POST Done");

  delay(5000);
}
