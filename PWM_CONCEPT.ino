#include <WiFi.h>
#include <WebServer.h>

// ---------- AP DETAILS ----------
const char* ap_ssid = "ESP32_KAIF";
const char* ap_pass = "12345678";

// ---------- INBUILT LED ----------
#define LED_PIN 2     // ⭐ onboard LED
#define PWM_FREQ 5000
#define PWM_RES 8     // 0–255

WebServer server(80);

int brightness = 0;

// ---------- WEB PAGE ----------
String page() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>ESP32 LED Control</title>";
  html += "<style>";
  html += "body{background:#111;color:white;font-family:Arial;text-align:center;padding-top:50px}";
  html += ".box{background:#222;width:320px;margin:auto;padding:25px;border-radius:10px}";
  html += "button{padding:15px 30px;margin:10px;font-size:18px;border:none;border-radius:6px}";
  html += ".on{background:green;color:white}";
  html += ".off{background:red;color:white}";
  html += ".high{background:orange;color:black}";
  html += "input{width:100%}";
  html += "</style></head><body>";

  html += "<div class='box'>";
  html += "<h2>ESP32 INBUILT LED</h2>";
  html += "<h3>Brightness: " + String(brightness) + "</h3>";

  html += "<form action='/on' method='POST'><button class='on'>ON</button></form>";
  html += "<form action='/off' method='POST'><button class='off'>OFF</button></form>";
  html += "<form action='/high' method='POST'><button class='high'>HIGH</button></form>";

  html += "<form action='/dim' method='POST'>";
  html += "<input type='range' min='0' max='255' name='val' value='" + String(brightness) + "'>";
  html += "<br><button class='high'>SET DIM</button></form>";

  html += "</div></body></html>";
  return html;
}

// ---------- ROUTES ----------
void handleRoot() {
  server.send(200, "text/html", page());
}

void handleOn() {
  brightness = 120;              // safe ON
  ledcWrite(LED_PIN, brightness);
  server.send(200, "text/html", page());
}

void handleOff() {
  brightness = 0;
  ledcWrite(LED_PIN, 0);
  server.send(200, "text/html", page());
}

void handleHigh() {
  brightness = 255;
  ledcWrite(LED_PIN, 255);
  server.send(200, "text/html", page());
}

void handleDim() {
  if (server.hasArg("val")) {
    brightness = server.arg("val").toInt();
    ledcWrite(LED_PIN, brightness);
  }
  server.send(200, "text/html", page());
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  // ✅ NEW ESP32 CORE PWM
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RES);
  ledcWrite(LED_PIN, 0);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  Serial.print("Open: ");
  Serial.println(WiFi.softAPIP()); // 192.168.4.1

  server.on("/", handleRoot);
  server.on("/on", HTTP_POST, handleOn);
  server.on("/off", HTTP_POST, handleOff);
  server.on("/high", HTTP_POST, handleHigh);
  server.on("/dim", HTTP_POST, handleDim);

  server.begin();
}

void loop() {
  server.handleClient();
}
#include <WiFi.h>
#include <WebServer.h>

// ---------- AP DETAILS ----------
const char* ap_ssid = "ESP32_KAIF";
const char* ap_pass = "12345678";

// ---------- INBUILT LED ----------
#define LED_PIN 2     // ⭐ onboard LED
#define PWM_FREQ 5000
#define PWM_RES 8     // 0–255

WebServer server(80);

int brightness = 0;

// ---------- WEB PAGE ----------
String page() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>ESP32 LED Control</title>";
  html += "<style>";
  html += "body{background:#111;color:white;font-family:Arial;text-align:center;padding-top:50px}";
  html += ".box{background:#222;width:320px;margin:auto;padding:25px;border-radius:10px}";
  html += "button{padding:15px 30px;margin:10px;font-size:18px;border:none;border-radius:6px}";
  html += ".on{background:green;color:white}";
  html += ".off{background:red;color:white}";
  html += ".high{background:orange;color:black}";
  html += "input{width:100%}";
  html += "</style></head><body>";

  html += "<div class='box'>";
  html += "<h2>ESP32 INBUILT LED</h2>";
  html += "<h3>Brightness: " + String(brightness) + "</h3>";

  html += "<form action='/on' method='POST'><button class='on'>ON</button></form>";
  html += "<form action='/off' method='POST'><button class='off'>OFF</button></form>";
  html += "<form action='/high' method='POST'><button class='high'>HIGH</button></form>";

  html += "<form action='/dim' method='POST'>";
  html += "<input type='range' min='0' max='255' name='val' value='" + String(brightness) + "'>";
  html += "<br><button class='high'>SET DIM</button></form>";

  html += "</div></body></html>";
  return html;
}

// ---------- ROUTES ----------
void handleRoot() {
  server.send(200, "text/html", page());
}

void handleOn() {
  brightness = 120;              // safe ON
  ledcWrite(LED_PIN, brightness);
  server.send(200, "text/html", page());
}

void handleOff() {
  brightness = 0;
  ledcWrite(LED_PIN, 0);
  server.send(200, "text/html", page());
}

void handleHigh() {
  brightness = 255;
  ledcWrite(LED_PIN, 255);
  server.send(200, "text/html", page());
}

void handleDim() {
  if (server.hasArg("val")) {
    brightness = server.arg("val").toInt();
    ledcWrite(LED_PIN, brightness);
  }
  server.send(200, "text/html", page());
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  // ✅ NEW ESP32 CORE PWM
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RES);
  ledcWrite(LED_PIN, 0);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  Serial.print("Open: ");
  Serial.println(WiFi.softAPIP()); // 192.168.4.1

  server.on("/", handleRoot);
  server.on("/on", HTTP_POST, handleOn);
  server.on("/off", HTTP_POST, handleOff);
  server.on("/high", HTTP_POST, handleHigh);
  server.on("/dim", HTTP_POST, handleDim);

  server.begin();
}

void loop() {
  server.handleClient();
} 

==== ab ye ARDUINO ME KESE KRENGE YHA DEKHLO ARDUINO ME PWM KHUD SE CONTOL NI KR SKTE 

// -------- LED PIN --------
#define LED_PIN 2  // PWM pin

// -------- BRIGHTNESS --------
int brightness = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {

  // ---------- DIM ----------
  // LED ko dheere jala rahe hain
  for (brightness = 0; brightness <= 255; brightness++) {
    analogWrite(LED_PIN, brightness); // PWM
    delay(10);
  }

  delay(1000);

  // ---------- HIGH ----------
  analogWrite(LED_PIN, 255); // full brightness
  delay(2000);

  // ---------- DIM DOWN ----------
  for (brightness = 255; brightness >= 0; brightness--) {
    analogWrite(LED_PIN, brightness);
    delay(10);
  }

  delay(1000);
}

