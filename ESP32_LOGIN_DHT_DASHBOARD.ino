#include <WiFi.h> // esp32 se wifi connect krne ke liye 
#include <WebServer.h> // esp32 ko web server banane ke liye 
#include <DHT.h> // DHT sensor 
#include <time.h> // internet se time lene ke liye 

// -------- WiFi --------
const char* ssid = "OfficeWiFi";
const char* password = "EnE@12345";

// -------- DHT --------
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// -------- Web --------
WebServer server(80);

// -------- Login / Session --------
String correctUser = "admin";
String correctPass = "1234";
bool isLoggedIn = false;

// -------- Time --------
const char* ntpServer = "pool.ntp.org"; // internet se direct time mil rha he 
const long gmtOffset_sec = 19800; //  19800 sec = India Time (UTC +5:30)
const int daylightOffset_sec = 0;

// -------- History Table --------
String tableRows = ""; // html row table storing 
int recordCount = 0;  // count the entry number 
const int MAX_RECORDS = 25;

// -------- Graph Data --------
float tempArr[10]; //last 10 array data show hoga graph me 
float humArr[10]; // same last 10
int graphIndex = 0; //[0-9] 

// ---------- LOGIN PAGE ----------
String loginPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>ESP32 Login</title>
<style>
body{
  background:linear-gradient(135deg,#667eea,#764ba2);
  height:100vh;display:flex;align-items:center;justify-content:center;
  font-family:Arial;
}
.card{
  background:white;padding:30px;width:300px;border-radius:10px;
}
input,button{
  width:100%;padding:10px;margin-top:10px;
}
button{
  background:#667eea;color:white;border:none;border-radius:5px;
}
</style>
</head>
<body>
<div class="card">
<h2 align="center">ESP32 Login</h2>
<form action="/login" method="POST">
<input name="user" placeholder="Username" required>
<input type="password" name="pass" placeholder="Password" required>
<button>Login</button>
</form>
</div>
</body>
</html>
)rawliteral";
}

// ---------- DASHBOARD ----------
String dashboardPage() { //c++ array ko js me convert kr rhe he 

  String tempJS = "";
  String humJS  = "";
  for (int i = 0; i < 10; i++) {
    tempJS += String(tempArr[i]) + ","; 
    humJS  += String(humArr[i]) + ",";
  }

  String page = "<!DOCTYPE html><html><head><title>Dashboard</title>";
  page += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  page += "<style>";
  page += "body{font-family:Arial;background:#f4f6f8;margin:0}";
  page += ".header{background:#667eea;color:white;padding:15px;text-align:center}";
  page += ".btn{padding:10px 15px;border:none;border-radius:5px;color:white;cursor:pointer}";
  page += ".logout{background:#333}";
  page += ".clear{background:red}";
  page += "table{width:90%;margin:20px auto;border-collapse:collapse;background:white}";
  page += "th,td{border:1px solid #ccc;padding:8px;text-align:center}";
  page += "th{background:#667eea;color:white}";
  page += "</style></head><body>";

  page += "<div class='header'>";
  page += "<h2>ESP32 Live Temp & Humidity</h2>";
  page += "<form action='/logout' method='POST' style='display:inline'>";
  page += "<button class='btn logout'>Logout</button></form>";
  page += "</div>";

  page += "<div style='width:90%;margin:20px auto'><canvas id='chart'></canvas></div>";

  page += "<div style='text-align:center'>";
  page += "<form action='/clear' method='POST'>";
  page += "<button class='btn clear'>Clear History</button></form></div>";

  page += "<table>";
  page += "<tr><th>#</th><th>Date & Time</th><th>Temp</th><th>Humidity</th></tr>";
  page += tableRows;
  page += "</table>";

  page += "<script>";
  page += "new Chart(document.getElementById('chart'),{";
  page += "type:'line',data:{labels:[1,2,3,4,5,6,7,8,9,10],";
  page += "datasets:[{label:'Temp °C',data:[" + tempJS + "],borderWidth:2},";
  page += "{label:'Humidity %',data:[" + humJS + "],borderWidth:2}]}});";
  page += "setTimeout(()=>{location.reload()},5000);";
  page += "</script>";

  page += "</body></html>";
  return page;
}

// ---------- ROUTES ----------
void handleRoot() {
  if (isLoggedIn) // if userlogin in
    server.send(200, "text/html", dashboardPage()); //200 is response ok // text/html is data is coming in html format and calling dashboard()
  else
    server.send(200, "text/html", loginPage());
}

void handleLogin() {
  if (server.arg("user") == correctUser &&
      server.arg("pass") == correctPass) {

    isLoggedIn = true;

    float t = dht.readTemperature(); //sensor reading
    float h = dht.readHumidity(); //sensor reading
    if (isnan(t) || isnan(h)) return; // if data or humdity ki value nan hai to function wahi ruk jayega 

    tempArr[graphIndex] = t;
    humArr[graphIndex]  = h; 
    graphIndex = (graphIndex + 1) % 10; //[0-9]

    struct tm timeinfo;
    getLocalTime(&timeinfo); // getting local time 
    char buf[30];
    strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", &timeinfo);

    recordCount++; // record add hota jayega and table me show 
    tableRows += "<tr><td>" + String(recordCount) + "</td><td>" + String(buf) +
                 "</td><td>" + String(t) + "</td><td>" + String(h) + "</td></tr>";

    server.send(200, "text/html", dashboardPage());
  } else {
    server.send(401, "text/html", "<h2>Wrong Login</h2>");
  }
}

void handleClear() {
  tableRows = "";
  recordCount = 0;
  server.send(200, "text/html", dashboardPage());
}

void handleLogout() {
  isLoggedIn = false;
  server.send(200, "text/html", loginPage());
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  server.on("/", handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/clear", HTTP_POST, handleClear);
  server.on("/logout", HTTP_POST, handleLogout);

  server.begin();
}

// ---------- LOOP ----------
void loop() {
  server.handleClient();
}
