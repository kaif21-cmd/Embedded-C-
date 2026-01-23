#include <DMD32Plus.h>
#include "fonts/SystemFont5x7.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

/******************** DISPLAY CONFIG ********************/
#define DISPLAYS_ACROSS 6
#define DISPLAYS_DOWN   8
#define PANEL_W 32
#define PANEL_H 16
#define SCREEN_W (DISPLAYS_ACROSS * PANEL_W)
#define SCREEN_H (DISPLAYS_DOWN   * PANEL_H)

/******************** PIN CONFIG ********************/
#define OE_PIN 22
#define A_PIN  19
#define B_PIN  21
#define CLK_PIN 18
#define LAT_PIN 4
#define R_DATA_PIN 23

/******************** WIFI ********************/
const char* ssid="DMD";
const char* password="12345678";

/******************** DMD ********************/
DMD dmd(DISPLAYS_ACROSS,DISPLAYS_DOWN,
        OE_PIN,A_PIN,B_PIN,CLK_PIN,LAT_PIN,R_DATA_PIN);

WebServer server(80);

/******************** EEPROM ********************/
#define EEPROM_SIZE 4096

/******************** TEXT SYSTEM ********************/
#define MAX_TEXT 20
struct TextItem {
  char text[30];
  int x, y;
  bool enable;
};
TextItem texts[MAX_TEXT];

/******************** LINE SYSTEM ********************/
#define MAX_LINES 20
struct LineItem {
  int x1,y1,x2,y2;
  bool enable;
};
LineItem lines[MAX_LINES];

/******************** GAP LINE SYSTEM ********************/
bool hGapEnable = false;
bool vGapEnable = false;

int h_startY = 30, h_gap = 15, h_count = 3;
int v_startX = 60, v_gap = 30, v_count = 2;

/******************** BRIGHTNESS ********************/
uint8_t brightnessValue = 40;

/******************** DRAW ********************/
void printXY(int x,int y,const char* t){
  dmd.selectFont(SystemFont5x7);
  dmd.drawString(x,y,t,strlen(t),GRAPHICS_NORMAL);
}

void drawDashboard() {

  dmd.clearScreen(true);

  // Manual Lines
  for(int i=0;i<MAX_LINES;i++){
    if(lines[i].enable){
      dmd.drawLine(lines[i].x1, lines[i].y1,
                   lines[i].x2, lines[i].y2,
                   GRAPHICS_NORMAL);
    }
  }

  // Gap Horizontal Lines
  if(hGapEnable){
    for(int i=0;i<h_count;i++){
      int y = h_startY + (i * h_gap);
      dmd.drawLine(0, y, SCREEN_W-1, y, GRAPHICS_NORMAL);
    }
  }

  // Gap Vertical Lines
  if(vGapEnable){
    for(int i=0;i<v_count;i++){
      int x = v_startX + (i * v_gap);
      dmd.drawLine(x, 0, x, SCREEN_H-1, GRAPHICS_NORMAL);
    }
  }

  // Texts
  for(int i=0;i<MAX_TEXT;i++){
    if(texts[i].enable){
      printXY(texts[i].x, texts[i].y, texts[i].text);
    }
  }
}

/******************** EEPROM ********************/
void saveEEPROM(){
  int a=0;
  EEPROM.put(a,brightnessValue); a+=sizeof(brightnessValue);

  for(int i=0;i<MAX_TEXT;i++){ EEPROM.put(a,texts[i]); a+=sizeof(TextItem); }
  for(int i=0;i<MAX_LINES;i++){ EEPROM.put(a,lines[i]); a+=sizeof(LineItem); }

  EEPROM.put(a,hGapEnable); a+=sizeof(bool);
  EEPROM.put(a,h_startY); a+=sizeof(int);
  EEPROM.put(a,h_gap); a+=sizeof(int);
  EEPROM.put(a,h_count); a+=sizeof(int);

  EEPROM.put(a,vGapEnable); a+=sizeof(bool);
  EEPROM.put(a,v_startX); a+=sizeof(int);
  EEPROM.put(a,v_gap); a+=sizeof(int);
  EEPROM.put(a,v_count); a+=sizeof(int);

  EEPROM.commit();
}

void loadEEPROM(){
  int a=0;
  EEPROM.get(a,brightnessValue); a+=sizeof(brightnessValue);

  for(int i=0;i<MAX_TEXT;i++){ EEPROM.get(a,texts[i]); a+=sizeof(TextItem); }
  for(int i=0;i<MAX_LINES;i++){ EEPROM.get(a,lines[i]); a+=sizeof(LineItem); }

  EEPROM.get(a,hGapEnable); a+=sizeof(bool);
  EEPROM.get(a,h_startY); a+=sizeof(int);
  EEPROM.get(a,h_gap); a+=sizeof(int);
  EEPROM.get(a,h_count); a+=sizeof(int);

  EEPROM.get(a,vGapEnable); a+=sizeof(bool);
  EEPROM.get(a,v_startX); a+=sizeof(int);
  EEPROM.get(a,v_gap); a+=sizeof(int);
  EEPROM.get(a,v_count); a+=sizeof(int);
}

/******************** CLEAR ALL SYSTEM ********************/
void clearAllData() {

  for(int i=0;i<MAX_TEXT;i++)  texts[i].enable = false;
  for(int i=0;i<MAX_LINES;i++) lines[i].enable = false;

  hGapEnable = false;
  vGapEnable = false;

  brightnessValue = 40;
  dmd.setBrightness(brightnessValue);

  for(int i=0;i<EEPROM_SIZE;i++) EEPROM.write(i, 0);
  EEPROM.commit();

  drawDashboard();
}

/******************** WEB PAGE ********************/
const char* htmlPage = R"rawliteral(
<!DOCTYPE html><html><head><title>DMD MATRIX DESIGNER</title>
<style>
body{background:#020617;color:white;font-family:Segoe UI;padding:20px}
.card{background:#0f172a;padding:20px;border-radius:12px;max-width:900px;margin:auto}
input,select{width:100%;padding:6px;margin:4px;border-radius:6px;border:none}
button{background:#22c55e;color:black;padding:10px;font-size:16px;width:100%;border:none;border-radius:8px}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:8px}
h3{color:#93c5fd}
.red{background:#ef4444;color:white}
</style></head>

<body><div class="card">
<h2>DMD MATRIX DESIGNER</h2>

<form id="matrixForm">

<h3>BRIGHTNESS</h3>
<input name="brightness" placeholder="1-255">

<h3>ADD / UPDATE TEXT</h3>
<input name="tid" placeholder="Text ID (0-19)">
<input name="text" placeholder="Text">
<div class="grid">
<input name="tx" placeholder="X"><input name="ty" placeholder="Y">
</div>

<h3>REMOVE TEXT</h3>
<div class="grid">
  <input name="tremove" placeholder="Text ID (0-19)">
  <button type="button" onclick="removeText()" class="red">REMOVE TEXT</button>
</div>

<h3>ADD / UPDATE LINE</h3>
<input name="lid" placeholder="Line ID (0-19)">
<div class="grid">
<input name="x1" placeholder="X1"><input name="y1" placeholder="Y1">
<input name="x2" placeholder="X2"><input name="y2" placeholder="Y2">
</div>

<h3>GAP H LINES</h3>
<select name="hen"><option value="1">ENABLE</option><option value="0">DISABLE</option></select>
<input name="hstart" placeholder="Start Y">
<input name="hgap" placeholder="Gap">
<input name="hcount" placeholder="Count">

<h3>GAP V LINES</h3>
<select name="ven"><option value="1">ENABLE</option><option value="0">DISABLE</option></select>
<input name="vstart" placeholder="Start X">
<input name="vgap" placeholder="Gap">
<input name="vcount" placeholder="Count">

<h3>REMOVE LINE</h3>
<div class="grid">
  <input name="lremove" placeholder="Line ID (0-19)">
  <button type="button" onclick="removeLine()" class="red">REMOVE LINE</button>
</div>

<h3>FULL RESET</h3>
<button type="button" onclick="clearAll()" style="background:#dc2626;color:white">
  🧹 CLEAR ALL (FRESH)
</button>

<button type="submit">SAVE MATRIX</button>
<p id="status" style="color:lime"></p>

</form>

<script>
document.getElementById("matrixForm").addEventListener("submit", function(e){
  e.preventDefault();
  const fd = new FormData(this);
  fetch("/set",{method:"POST",body:fd})
  .then(()=>{document.getElementById("status").innerHTML="✅ Saved";});
});

function removeLine(){
  const fd = new FormData();
  fd.append("action","removeLine");
  fd.append("lremove", document.querySelector('input[name="lremove"]').value);

  fetch("/set",{method:"POST",body:fd})
  .then(()=>{document.getElementById("status").innerHTML="🟥 Line Removed";});
}

function removeText(){
  const fd = new FormData();
  fd.append("action","removeText");
  fd.append("tremove", document.querySelector('input[name="tremove"]').value);

  fetch("/set",{method:"POST",body:fd})
  .then(()=>{document.getElementById("status").innerHTML="🟥 Text Removed";});
}

function clearAll(){
  if(!confirm("Are you sure? This will remove EVERYTHING!")) return;

  const fd = new FormData();
  fd.append("action","clearAll");

  fetch("/set",{method:"POST",body:fd})
  .then(()=>{
    document.getElementById("status").innerHTML="🧹 All Cleared - Fresh Matrix";
  });
}
</script>

</div></body></html>
)rawliteral";

/******************** HANDLERS ********************/
void handleRoot(){ server.send(200,"text/html",htmlPage); }

void handleSet(){

  String action = "save";
  if(server.hasArg("action")) action = server.arg("action");

  // CLEAR ALL
  if(action=="clearAll"){
    clearAllData();
    server.send(200,"text/plain","CLEARED");
    return;
  }

  // REMOVE LINE
  if(action=="removeLine" && server.hasArg("lremove")){
    int id=server.arg("lremove").toInt();
    if(id>=0 && id<MAX_LINES) lines[id].enable=false;
    saveEEPROM(); drawDashboard(); server.send(200,"text/plain","OK"); return;
  }

  // REMOVE TEXT
  if(action=="removeText" && server.hasArg("tremove")){
    int id=server.arg("tremove").toInt();
    if(id>=0 && id<MAX_TEXT) texts[id].enable=false;
    saveEEPROM(); drawDashboard(); server.send(200,"text/plain","OK"); return;
  }

  // BRIGHTNESS
  if(server.hasArg("brightness")){
    brightnessValue = constrain(server.arg("brightness").toInt(),1,255);
    dmd.setBrightness(brightnessValue);
  }

  // TEXT SAVE
  if(server.hasArg("tid")){
    int id=server.arg("tid").toInt();
    if(id>=0 && id<MAX_TEXT){
      server.arg("text").toCharArray(texts[id].text,30);
      texts[id].x=server.arg("tx").toInt();
      texts[id].y=server.arg("ty").toInt();
      texts[id].enable=true;
    }
  }

  // LINE SAVE
  if(server.hasArg("lid")){
    int id=server.arg("lid").toInt();
    if(id>=0 && id<MAX_LINES){
      lines[id].x1=server.arg("x1").toInt();
      lines[id].y1=server.arg("y1").toInt();
      lines[id].x2=server.arg("x2").toInt();
      lines[id].y2=server.arg("y2").toInt();
      lines[id].enable=true;
    }
  }

  // GAP H
  if(server.hasArg("hen")) hGapEnable=server.arg("hen").toInt();
  if(server.hasArg("hstart")) h_startY=server.arg("hstart").toInt();
  if(server.hasArg("hgap")) h_gap=server.arg("hgap").toInt();
  if(server.hasArg("hcount")) h_count=server.arg("hcount").toInt();

  // GAP V
  if(server.hasArg("ven")) vGapEnable=server.arg("ven").toInt();
  if(server.hasArg("vstart")) v_startX=server.arg("vstart").toInt();
  if(server.hasArg("vgap")) v_gap=server.arg("vgap").toInt();
  if(server.hasArg("vcount")) v_count=server.arg("vcount").toInt();

  saveEEPROM();
  drawDashboard();
  server.send(200,"text/plain","OK");
}

/******************** SETUP ********************/
void setup(){

  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED) delay(300);

  ArduinoOTA.begin();

  for(int i=0;i<MAX_LINES;i++) lines[i].enable=false;
  for(int i=0;i<MAX_TEXT;i++) texts[i].enable=false;

  loadEEPROM();
  dmd.setBrightness(brightnessValue);

  server.on("/",handleRoot);
  server.on("/set",HTTP_POST,handleSet);
  server.begin();

  Serial.print("Open Matrix: http://");
  Serial.println(WiFi.localIP());

  drawDashboard();
}

/******************** LOOP ********************/
void loop(){
  dmd.scanDisplayBySPI();
  ArduinoOTA.handle();
  server.handleClient();
}
