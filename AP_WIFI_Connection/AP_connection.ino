#include <WiFi.h> // esp32 ko wifi connect krne ke liye 
#include <WebServer.h> //esp32 ko weberver banata he 
#include <DHT.h> // sensor connect krne ke liye 
#include <time.h> // time dega internet se 

// -------- AP DETAILS -------- // esp32apna hotspot wifi bnayega router ki jarurat ni he 
const char* ap_ssid = "ESP32_KAIF";  
const char* ap_pass = "12345678";

// -------- DHT --------
#define DHTPIN 4 //dht ki pin declare krdi 
#define DHTTYPE DHT11 // dht ka type
DHT dht(DHTPIN, DHTTYPE); 

// -------- Web --------
WebServer server(80); // webserver ka port dediya ja servre host hoga 

// -------- Login --------
String correctUser = "admin";
String correctPass = "1234";
bool isLoggedIn = false;

// -------- History --------
String tableRows = "";
int recordCount = 0;

// -------- Graph Data --------
float tempArr[10], humArr[10], soxArr[10], noxArr[10];
int graphIndex = 0;

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
  height:100vh;
  display:flex;
  justify-content:center;
  align-items:center;
  font-family:Arial;
}
.card{
  background:white;
  padding:30px;
  width:300px;
  border-radius:10px;
}
input,button{
  width:100%;
  padding:10px;
  margin-top:10px;
}
button{
  background:#667eea;
  color:white;
  border:none;
  border-radius:5px;
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
String dashboardPage() {

  String tJS="", hJS="", sJS="", nJS="";
  for(int i=0;i<10;i++){
    tJS+=String(tempArr[i])+",";
    hJS+=String(humArr[i])+",";
    sJS+=String(soxArr[i])+",";
    nJS+=String(noxArr[i])+",";
  }

  String page="<html><head><title>Dashboard</title>";
  page+="<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  page+="<style>";
  page+="body{font-family:Arial;background:#f4f6f8;margin:0}";
  page+=".header{background:#667eea;color:white;padding:15px;text-align:center}";
  page+=".btn{padding:8px 14px;margin:5px;border:none;border-radius:5px;color:white;cursor:pointer}";
  page+=".logout{background:#333}.clear{background:red}";
  page+="table{width:95%;margin:20px auto;border-collapse:collapse;background:white;font-size:14px}";
  page+="th,td{border:1px solid #ccc;padding:6px;text-align:center}";
  page+="th{background:#667eea;color:white}";
  page+="</style></head><body>";

  page+="<div class='header'><h2>ESP32 Kaif Air Quality Dashboard</h2>";
  page+="<form action='/logout' method='POST' style='display:inline'>";
  page+="<button class='btn logout'>Logout</button></form></div>";

  page+="<div style='width:500px;margin:15px auto'>";
  page+="<canvas id='chart' height='120'></canvas></div>";

  page+="<div style='text-align:center'>";
  page+="<form action='/clear' method='POST' style='display:inline'>";
  page+="<button class='btn clear'>Clear History</button></form></div>";

  page+="<table>";
  page+="<tr><th>#</th><th>Temp</th><th>Humidity</th><th>SOx</th><th>NOx</th></tr>";
  page+=tableRows;
  page+="</table>";

  page+="<script>";
  page+="new Chart(document.getElementById('chart'),{";
  page+="type:'line',data:{labels:[1,2,3,4,5,6,7,8,9,10],datasets:[";
  page+="{label:'Temp',data:[" + tJS + "],borderWidth:2,tension:0.3},";
  page+="{label:'Humidity',data:[" + hJS + "],borderWidth:2,tension:0.3},";
  page+="{label:'SOx',data:[" + sJS + "],borderWidth:2,tension:0.3},";
  page+="{label:'NOx',data:[" + nJS + "],borderWidth:2,tension:0.3}";
  page+="]}});";
  page+="setTimeout(()=>{location.reload()},5000);</script>";

  page+="</body></html>";
  return page;
}

// ---------- ROUTES ----------
void handleRoot(){
  server.send(200,"text/html", isLoggedIn ? dashboardPage() : loginPage());
}

void handleLogin(){
  if(server.arg("user")==correctUser && server.arg("pass")==correctPass){
    isLoggedIn=true;

    float t=dht.readTemperature();
    float h=dht.readHumidity();
    float sox=random(30,61)/100.0;
    float nox=random(20,51)/100.0;

    tempArr[graphIndex]=t;
    humArr[graphIndex]=h;
    soxArr[graphIndex]=sox;
    noxArr[graphIndex]=nox;
    graphIndex=(graphIndex+1)%10;

    recordCount++;
    tableRows+="<tr><td>"+String(recordCount)+"</td><td>"+String(t)+
               "</td><td>"+String(h)+"</td><td>"+String(sox)+
               "</td><td>"+String(nox)+"</td></tr>";

    server.send(200,"text/html",dashboardPage());
  } else {
    server.send(401,"text/html","<h2>Wrong Login</h2>");
  }
}

void handleClear(){
  tableRows="";
  recordCount=0;
  server.send(200,"text/html",dashboardPage());
}

void handleLogout(){
  isLoggedIn=false;
  server.send(200,"text/html",loginPage());
}

// ---------- SETUP ----------
void setup(){
  Serial.begin(115200);
  dht.begin();
  randomSeed(esp_random());

  // AP MODE
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  Serial.println("ESP32 AP Started");
  Serial.println(WiFi.softAPIP()); // 192.168.4.1

  server.on("/",handleRoot);
  server.on("/login",HTTP_POST,handleLogin);
  server.on("/clear",HTTP_POST,handleClear);
  server.on("/logout",HTTP_POST,handleLogout);

  server.begin();
}

// ---------- LOOP ----------
void loop(){
  server.handleClient();
}