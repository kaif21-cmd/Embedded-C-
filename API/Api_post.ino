#include <WiFi.h> // wifi se connect krne ke liye..../
#include <HTTPClient.h> //rek or response handel krne ke liye 
#include <ArduinoJson.h> // JSON data banane aur send karne ke liye

// ---------- WiFi ----------
const char* ssid = "OfficeWiFi";
const char* password = "EnE@12345";

// ---------- POST API ----------
const char* postUrl = "https://customer.enggenv.com/postdata.php";

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // jab tak wifi connect ni hoga 5 sec tak chalega .. print hota jayega 
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected"); // wifi connected 
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    return;
  }

  HTTPClient http;
  http.begin(postUrl);
  http.addHeader("Content-Type", "application/json");

  // json format data
  StaticJsonDocument<256> doc; // 256 bytes ka JSON container

  doc["spm"] = "20";
  doc["lg"]  = "30";
  doc["lt"]  = "0";
  doc["bv"]  = "0";
  doc["ts_client"] = "2025-10-10 16:39:09";
  doc["id"]="ENE04912";

  String jsonData; // ArduinoJson - String me change
  serializeJson(doc, jsonData); //HTTP POST me bhejne ke liye

  Serial.println("POST JSON:");
  Serial.println(jsonData);

  int httpCode = http.POST(jsonData); // server response 200 ok 
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);

  String response = http.getString(); //  server ka jawab suno
  Serial.println("Server Response:"); 
  Serial.println(response); // server ka response serial pe dikhane ke liye 
  http.end(); // connection end 

  delay(5000);
}




