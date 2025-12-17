#include <WiFi.h>

const char* ssid = "OfficeWiFi";
const char* password = "EnE@12345";

WiFiServer server(80);

// LED pins
#define LED1 2
#define LED2 4
#define LED3 5

bool sequenceOn = false;

void setup() {
  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("/TOGGLE") != -1) {
      sequenceOn = !sequenceOn;
    }

    // ===== HTTP HEADER =====
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    
    // ===== HTML + CSS =====
    client.println("<!DOCTYPE html><html>");
    client.println("<head>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
    client.println("<style>");
    client.println("body{font-family:Arial;background:#0f172a;margin:0;padding:0;color:white;}");
    client.println(".card{max-width:350px;margin:50px auto;background:#020617;padding:20px;border-radius:15px;box-shadow:0 0 15px #38bdf8;}");
    client.println("h2{text-align:center;color:#38bdf8;}");
    client.println(".status{font-size:20px;text-align:center;margin:20px 0;}");
    client.println(".btn{width:100%;padding:15px;font-size:18px;border:none;border-radius:10px;cursor:pointer;}");
    
    if (sequenceOn) {
      client.println(".btn{background:#ef4444;color:white;}");
    } else {
      client.println(".btn{background:#22c55e;color:white;}");
    }

    client.println("</style>");
    client.println("</head>");

    client.println("<body>");
    client.println("<div class='card'>");
    client.println("<h2>ESP32 LED Controller By Kaif </h2>");

    if (sequenceOn) {
      client.println("<div class='status'>Status: <b style='color:#22c55e'>RUNNING</b></div>");
      client.println("<a href='/TOGGLE'><button class='btn'>STOP</button></a>");
    } else {
      client.println("<div class='status'>Status: <b style='color:#ef4444'>STOPPED</b></div>");
      client.println("<a href='/TOGGLE'><button class='btn'>START</button></a>");
    }

    client.println("</div>");
    client.println("</body></html>");

    client.stop();
  }

  // ===== LED SEQUENCE =====
  if (sequenceOn) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    delay(150);

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    delay(150);

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    delay(150);
  } else {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
  }
}
