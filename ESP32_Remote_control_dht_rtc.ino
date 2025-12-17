#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include "RTClib.h"

// ===== WIFI =====
const char* ssid = "i";
const char* password = "";
WiFiServer server(80);

// ===== DHT =====
#define DHTPIN 4
#define DHTTYPE DHT11      // DHT22 use karna ho to change
DHT dht(DHTPIN, DHTTYPE);

// ===== RTC =====
RTC_DS3231 rtc;

// ===== TIMING =====
unsigned long lastRead = 0;
float temperature = 0;
float humidity = 0;
String dateTime = "";

void setup() {
  Serial.begin(115200);

  // DHT
  dht.begin();

  // RTC
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("RTC NOT FOUND");
    while (1);
  }

  // Uncomment ONLY ONCE to set time
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // WiFi
  Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  // ===== READ SENSOR EVERY 2 SEC =====
  if (millis() - lastRead >= 2000) {
    lastRead = millis();

    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    DateTime now = rtc.now();
    dateTime = String(now.day()) + "/" +
               String(now.month()) + "/" +
               String(now.year()) + " " +
               String(now.hour()) + ":" +
               String(now.minute()) + ":" +
               String(now.second());

    Serial.println("Updated Data");
  }

  // ===== WEB SERVER =====
  WiFiClient client = server.available();
  if (client) {
    client.readStringUntil('\r');
    client.flush();

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    
    client.println("<!DOCTYPE html><html>");
    client.println("<head>");
    client.println("<meta http-equiv='refresh' content='2'>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
    client.println("<style>");
    client.println("body{font-family:Arial;background:#020617;color:white;}");
    client.println(".card{max-width:350px;margin:40px auto;padding:20px;");
    client.println("background:#0f172a;border-radius:15px;");
    client.println("box-shadow:0 0 15px #38bdf8;}");
    client.println("h2{text-align:center;color:#38bdf8;}");
    client.println(".box{margin:15px 0;font-size:18px;}");
    client.println("</style>");
    client.println("</head>");

    client.println("<body>");
    client.println("<div class='card'>");
    client.println("<h2>ESP32 Weather Station</h2>");

    client.println("<div class='box'>🌡 Temperature: <b>");
    client.println(String(temperature) + " °C</b></div>");

    client.println("<div class='box'>💧 Humidity: <b>");
    client.println(String(humidity) + " %</b></div>");

    client.println("<div class='box'>⏰ Date & Time:<br><b>");
    client.println(dateTime + "</b></div>");

    client.println("</div>");
    client.println("</body></html>");

    client.stop();
  }
}
