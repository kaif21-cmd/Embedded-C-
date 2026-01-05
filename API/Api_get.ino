#include <WiFi.h>
#include <HTTPClient.h>  // HTTP/HTTPS request bhejne ke liye (GET, POST)

//connecting the wifi
const char* ssid = "OfficeWiFi";
const char* password = "EnE@12345";
// api address
const char* apiUrl = "https://api.enggenv.com/api/fetchdata?id=ENE04912";

void setup() {
  Serial.begin(115200);
  delay(1000);

  //connecting the wifi
  Serial.println("Connecting to WiFi..");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n WiFi Connected");
  // Serial.print("IP Address: ");
  // Serial.println(WiFi.localIP());
}

void loop() {

  //WiFi connected check
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;  // HTTP client banaya get rek behjega or sever se respond lega

    http.begin(apiUrl);  // Is URL pe request bhejni hai . begin se request

    int httpCode = http.GET();  // server ko get request bhejega return mr httpcode milega 200 2oo-ok succed
    if (httpCode > 0) {
      Serial.println("\nAPI DATA ");
      // Serial.print("HTTP Code: ");
      String response = http.getString();  //server se jo bhi data ayega wo hme string form me mile jayega
      Serial.println(response);
    } else {
      Serial.print("GET Failed, Error: ");
      // Serial.println(httpCode);
    }
    http.end();  // close connection
  } else {
    Serial.println("WiFi Disconnected");
  }
  delay(5000);  // api hit every 5 sec
}
