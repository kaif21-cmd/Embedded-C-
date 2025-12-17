#include <WiFi.h>

const char* ssid = "OfficeWiFi";
const char* password = "EnE@12345";

WiFiServer server(80);

// LED pins
#define LED1 2
#define LED2 4
#define LED3 5

bool led1State = false;
bool led2State = false;
bool led3State = false;

bool sequenceOn = false;
int sequenceDelay = 1000; // default 1000ms

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

    // Handle requests
    if (request.indexOf("/LED1") != -1) led1State = !led1State;
    if (request.indexOf("/LED2") != -1) led2State = !led2State;
    if (request.indexOf("/LED3") != -1) led3State = !led3State;
    if (request.indexOf("/SEQUENCE") != -1) sequenceOn = !sequenceOn;

    // Set sequence speed
    if (request.indexOf("/SPEED100") != -1) sequenceDelay = 100;
    if (request.indexOf("/SPEED1000") != -1) sequenceDelay = 1000;
    if (request.indexOf("/SPEED5000") != -1) sequenceDelay = 5000;

    // Turn all ON/OFF
    if (request.indexOf("/ALLON") != -1) { led1State = led2State = led3State = true; }
    if (request.indexOf("/ALLOFF") != -1) { led1State = led2State = led3State = false; }

    // ===== HTTP Response =====
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<!DOCTYPE html><html>");
    client.println("<head>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
    client.println("<style>");
    client.println("body{font-family:Arial;background:#0f172a;margin:0;padding:0;color:white;}");
    client.println(".card{max-width:400px;margin:20px auto;background:#020617;padding:20px;border-radius:15px;box-shadow:0 0 15px #38bdf8;}");
    client.println("h2{text-align:center;color:#38bdf8;}");
    client.println(".btn{width:100%;padding:12px;margin:5px;font-size:16px;border:none;border-radius:10px;cursor:pointer;}");
    client.println(".on{background:#22c55e;color:white;}");
    client.println(".off{background:#ef4444;color:white;}");
    client.println("</style></head><body>");
    client.println("<div class='card'>");
    client.println("<h2>ESP32 LED Dashboard Kaif</h2>");

    // Individual LEDs
    client.println("<a href='/LED1'><button class='btn "+String(led1State?"on":"off")+"'>LED1: "+String(led1State?"ON":"OFF")+"</button></a>");
    client.println("<a href='/LED2'><button class='btn "+String(led2State?"on":"off")+"'>LED2: "+String(led2State?"ON":"OFF")+"</button></a>");
    client.println("<a href='/LED3'><button class='btn "+String(led3State?"on":"off")+"'>LED3: "+String(led3State?"ON":"OFF")+"</button></a>");

    // Sequence toggle
    client.println("<a href='/SEQUENCE'><button class='btn "+String(sequenceOn?"on":"off")+"'>Sequence: "+String(sequenceOn?"RUNNING":"STOPPED")+"</button></a>");

    // Speed buttons
    client.println("<p>Sequence Speed:</p>");
    client.println("<a href='/SPEED100'><button class='btn'>100ms</button></a>");
    client.println("<a href='/SPEED1000'><button class='btn'>1000ms</button></a>");
    client.println("<a href='/SPEED5000'><button class='btn'>5000ms</button></a>");

    // All ON/OFF
    client.println("<a href='/ALLON'><button class='btn on'>ALL ON</button></a>");
    client.println("<a href='/ALLOFF'><button class='btn off'>ALL OFF</button></a>");

    client.println("</div></body></html>");
    client.stop();
  }

  // Update LEDs
  if (sequenceOn) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    delay(sequenceDelay);

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    delay(sequenceDelay);

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    delay(sequenceDelay);
  } else {
    digitalWrite(LED1, led1State ? HIGH : LOW);
    digitalWrite(LED2, led2State ? HIGH : LOW);
    digitalWrite(LED3, led3State ? HIGH : LOW);
  }
}
