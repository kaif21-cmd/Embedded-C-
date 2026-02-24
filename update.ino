#include <DMD32Plus.h>
#include "fonts/SystemFont5x7.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <LITTLEFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

/******************** DISPLAY CONFIG ********************/
#define DISPLAYS_ACROSS 6
#define DISPLAYS_DOWN 8
#define PANEL_W 32
#define PANEL_H 16
#define SCREEN_W (DISPLAYS_ACROSS * PANEL_W)
#define SCREEN_H (DISPLAYS_DOWN * PANEL_H)

/******************** PIN CONFIG ********************/
#define OE_PIN 22
#define A_PIN 19
#define B_PIN 21
#define CLK_PIN 18
#define LAT_PIN 4
#define R_DATA_PIN 23
#define MAX_PAGES 4


/******************** WIFI ********************/
const char* ssid = "DMD";
const char* password = "XSW@xsw2ZAQ!zaq1";

/******************** DMD ********************/
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN,
        OE_PIN, A_PIN, B_PIN, CLK_PIN, LAT_PIN, R_DATA_PIN);

WebServer server(80);

/******************** FILE ********************/
#define DATA_FILE "/layout.bin"

/******************** TEXT SYSTEM ********************/
#define MAX_TEXT 50   // max text put on dmd
#define MAX_LINES 50  // max lines put on dmd


/******** TEXT SYSTEM ********/
struct TextItem {
  char text[50];
  int x, y;
  bool enable;
  bool marquee;
  bool isApi;     
  int offset;
};


/******** LINE SYSTEM ********/
struct LineItem {
  int x1, y1, x2, y2;
  bool enable;
};

/******** PAGE SYSTEM ********/
struct Page {
  TextItem texts[MAX_TEXT];
  LineItem lines[MAX_LINES];
};

/******** GLOBAL PAGE ARRAYS ********/
Page pages[MAX_PAGES];
Page backupPages[MAX_PAGES];
int currentPage = 0;

/******** PAGE AUTO LOOP ********/

bool pageLoopEnabled = false;

// ===== API PAGE ENABLE FLAGS =====
bool apiEnabledForPage[MAX_PAGES] = { false };

unsigned long pageInterval = 5000;  // 5 sec default
unsigned long lastPageSwitch = 0;

/******************** BRIGHTNESS ********************/
uint8_t brightnessValue = 40, backupBrightness = 40;

/******************** IMPORT BUFFER ********************/
String uploadedJson = "";

/******************** DRAW ********************/
void printXY(int x, int y, const char* t) {
  dmd.selectFont(SystemFont5x7);
  dmd.drawString(x, y, t, strlen(t), GRAPHICS_NORMAL);  // x ,y position t- text string ,strlen(t)text lenght
}

void drawDashboard() {
  dmd.clearScreen(true);

  // -------- LINES --------
  for (int i = 0; i < MAX_LINES; i++) {
    if (pages[currentPage].lines[i].enable) {
      dmd.drawLine(
        pages[currentPage].lines[i].x1,
        pages[currentPage].lines[i].y1,
        pages[currentPage].lines[i].x2,
        pages[currentPage].lines[i].y2,
        GRAPHICS_NORMAL);
    }
  }

  // -------- TEXTS --------
 for (int i = 0; i < MAX_TEXT; i++) {

  // sirf API text hide karo jab API OFF ho
  
  if (pages[currentPage].texts[i].isApi &&
      !apiEnabledForPage[currentPage]) {
    continue;
  }

  if (pages[currentPage].texts[i].enable) {
    printXY(
      pages[currentPage].texts[i].x,
      pages[currentPage].texts[i].y,
      pages[currentPage].texts[i].text);
  }
}
}

/******************** BACKUP / UNDO ********************/
void backupState() {
  memcpy(backupPages, pages, sizeof(pages));
  backupBrightness = brightnessValue;
}

void undoLast() {
  memcpy(pages, backupPages, sizeof(pages));
  brightnessValue = backupBrightness;
  dmd.setBrightness(brightnessValue);
  drawDashboard();
}

/******************** LITTLEFS SAVE ********************/
void saveToFS() {
  File f = LITTLEFS.open(DATA_FILE, "w");
  if (!f) return;

  // Brightness
  f.write((uint8_t*)&brightnessValue, sizeof(brightnessValue));

  // All Pages
  f.write((uint8_t*)pages, sizeof(pages));

  f.close();
}


/******************** LITTLEFS LOAD ********************/
void loadFromFS() {
  if (!LITTLEFS.exists(DATA_FILE)) return;

  File f = LITTLEFS.open(DATA_FILE, "r");
  if (!f) return;

  // Brightness
  f.read((uint8_t*)&brightnessValue, sizeof(brightnessValue));

  // All Pages
  f.read((uint8_t*)pages, sizeof(pages));

  f.close();
}

void clearApiTextForPage(int p) {
  for (int i = 0; i < MAX_TEXT; i++) {
    if (pages[p].texts[i].isApi) {
      pages[p].texts[i].enable = false;
    }
  }
}

/******************** CLEAR ALL ********************/
void clearAllData() {
  backupState();

  // ---- CLEAR CURRENT PAGE TEXTS ----
  for (int i = 0; i < MAX_TEXT; i++) {
    pages[currentPage].texts[i].enable = false;
    pages[currentPage].texts[i].marquee = false;
    pages[currentPage].texts[i].offset = 0;
  }

  // ---- CLEAR CURRENT PAGE LINES ----
  for (int i = 0; i < MAX_LINES; i++) {
    pages[currentPage].lines[i].enable = false;
  }

  brightnessValue = 40;
  dmd.setBrightness(brightnessValue);

  if (LITTLEFS.exists(DATA_FILE))
    LITTLEFS.remove(DATA_FILE);

  drawDashboard();
}

//===============================================///
void handleExport() {
  StaticJsonDocument<4096> doc;

  doc["brightness"] = brightnessValue;
  doc["page"] = currentPage;  // optional but useful

  doc["loop"] = pageLoopEnabled;
  doc["looptime"] = pageInterval / 1000;  // seconds

  // -------- TEXTS --------
  JsonArray t = doc.createNestedArray("texts");
  for (int i = 0; i < MAX_TEXT; i++) {
    if (pages[currentPage].texts[i].enable) {

      JsonObject o = t.createNestedObject();
      o["id"] = i;
      o["text"] = pages[currentPage].texts[i].text;
      o["x"] = pages[currentPage].texts[i].x;
      o["y"] = pages[currentPage].texts[i].y;
      o["enable"] = true;
      o["marquee"] = pages[currentPage].texts[i].marquee;
    }
  }

  // -------- LINES --------
  JsonArray l = doc.createNestedArray("lines");
  for (int i = 0; i < MAX_LINES; i++) {
    if (pages[currentPage].lines[i].enable) {

      JsonObject o = l.createNestedObject();
      o["id"] = i;
      o["x1"] = pages[currentPage].lines[i].x1;
      o["y1"] = pages[currentPage].lines[i].y1;
      o["x2"] = pages[currentPage].lines[i].x2;
      o["y2"] = pages[currentPage].lines[i].y2;
      o["enable"] = true;
    }
  }

  String out;
  serializeJsonPretty(doc, out);

  server.sendHeader("Content-Disposition", "attachment; filename=layout.json");
  server.send(200, "application/json", out);
}




/******************** IMPORT JSON (FIXED) ********************/
void handleImport() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    uploadedJson = "";
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    for (size_t i = 0; i < upload.currentSize; i++) {
      uploadedJson += (char)upload.buf[i];
    }
  } else if (upload.status == UPLOAD_FILE_END) {

    StaticJsonDocument<8192> doc;
    DeserializationError err = deserializeJson(doc, uploadedJson);

    if (err) {
      server.send(400, "text/plain", "JSON Parse Error");
      return;
    }

    backupState();

    // -------- BRIGHTNESS --------
    brightnessValue = doc["brightness"] | brightnessValue;
    dmd.setBrightness(brightnessValue);

    // -------- LOOP IMPORT --------
    pageLoopEnabled = doc["loop"] | false;

    int sec = doc["looptime"] | 5;
    pageInterval = sec * 1000UL;

    lastPageSwitch = millis();  // restart timer


    // -------- CLEAR CURRENT PAGE FIRST --------
    for (int i = 0; i < MAX_TEXT; i++)
      pages[currentPage].texts[i].enable = false;

    for (int i = 0; i < MAX_LINES; i++)
      pages[currentPage].lines[i].enable = false;

    // -------- TEXT IMPORT --------
    for (int i = 0; i < MAX_TEXT; i++) {
      strlcpy(
        pages[currentPage].texts[i].text,
        doc["texts"][i]["text"] | "",
        30);

      pages[currentPage].texts[i].x =
        doc["texts"][i]["x"] | 0;

      pages[currentPage].texts[i].y =
        doc["texts"][i]["y"] | 0;

      pages[currentPage].texts[i].enable =
        doc["texts"][i]["enable"] | false;

      pages[currentPage].texts[i].marquee =
        doc["texts"][i]["marquee"] | false;

      pages[currentPage].texts[i].offset = 0;
    }

    // -------- LINE IMPORT --------
    for (int i = 0; i < MAX_LINES; i++) {
      pages[currentPage].lines[i].x1 =
        doc["lines"][i]["x1"] | 0;

      pages[currentPage].lines[i].y1 =
        doc["lines"][i]["y1"] | 0;

      pages[currentPage].lines[i].x2 =
        doc["lines"][i]["x2"] | 0;

      pages[currentPage].lines[i].y2 =
        doc["lines"][i]["y2"] | 0;

      pages[currentPage].lines[i].enable =
        doc["lines"][i]["enable"] | false;
    }

    saveToFS();
    drawDashboard();

    server.send(200, "text/plain", "IMPORT OK");
  }
}



/******************** WEB PAGE (FULL UI) ********************/
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>DMD MATRIX DESIGNER</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>

</style>
</head>

<body>
<div class="dashboard">

<div class="header">

<br>
</br>
<div id="pageBar">Pages</div>
<br>
</br>
</div>

<form id="matrixForm">

<div class="section">

</form>

<script>
const status = document.getElementById("status");

let currentPage = 0;
const MAX_PAGES = 4;   // jitne pages chahiye

/* ---------- FORM SAVE ---------- */
document.getElementById("matrixForm").addEventListener("submit", e=>{
  e.preventDefault();
  fetch("/set",{method:"POST",body:new FormData(e.target)})
  .then(()=>{
    status.innerHTML="Saved";
    loadTable();
  });
});

/* ---------- REMOVE TEXT ---------- */
function removeText(){
  let f=new FormData();
  f.append("action","removeText");
  f.append("tremove",document.querySelector("[name=tremove]").value);
  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- REMOVE LINE ---------- */
function removeLine(){
  let f=new FormData();
  f.append("action","removeLine");
  f.append("lremove",document.querySelector("[name=lremove]").value);
  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- CLEAR ---------- */
function clearAll(){
  let f=new FormData();
  f.append("action","clearAll");
  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- UNDO ---------- */
function undo(){
  let f=new FormData();
  f.append("action","undo");
  fetch("/set",{method:"POST",body:f}).then(loadTable);
}



/* ---------- EXPORT ---------- */
function exportData(){
  window.location.href="/export";
}

/* ---------- IMPORT ---------- */
function importData(input){
  let f=new FormData();
  f.append("file",input.files[0]);
  fetch("/import",{method:"POST",body:f})
  .then(()=>{
    status.innerHTML="Imported Successfully";
    loadTable();
  });
}

/* ---------- PAGE BUTTONS ---------- */
function createPages(){
  let bar = document.getElementById("pageBar");
  bar.innerHTML = "";

  for(let i=0;i<MAX_PAGES;i++){
    bar.innerHTML +=
      `<button class="pageBtn" id="p${i}"
        onclick="setPage(${i})">
        Page ${i+1}
      </button>`;
  }
  highlightPage();
}

function setPage(p){
  let f=new FormData();
  f.append("p",p);

  fetch("/page",{method:"POST",body:f})
  .then(()=>{
    currentPage = p;
    highlightPage();
    loadTable();
  });
}

function highlightPage(){
  for(let i=0;i<MAX_PAGES;i++){
    let btn=document.getElementById("p"+i);
    if(btn) btn.classList.remove("active");
  }
  let active=document.getElementById("p"+currentPage);
  if(active) active.classList.add("active");
}

/* ---------- LOAD TABLE ---------- */
function loadTable(){
  fetch("/data")
  .then(r=>r.json())
  .then(d=>{

    /* TEXT TABLE */
    let tb = document.querySelector("#textTable tbody");
    tb.innerHTML = "";

    d.texts.forEach(t=>{
      tb.innerHTML += `
        <tr>
          <td>${t.id}</td>
          <td><input value="${t.text}" id="t${t.id}"></td>
          <td><input value="${t.x}" id="tx${t.id}" size="3"></td>
          <td><input value="${t.y}" id="ty${t.id}" size="3"></td>
          <td>
            <button onclick="updateText(${t.id})">Save</button>
            <button onclick="deleteText(${t.id})">Delete</button>
          </td>
        </tr>`;
    });

    /* LINE TABLE */
    let lb = document.querySelector("#lineTable tbody");
    lb.innerHTML = "";

    d.lines.forEach(l=>{
      lb.innerHTML += `
        <tr>
          <td>${l.id}</td>
          <td><input value="${l.x1}" id="x1${l.id}" size="3"></td>
          <td><input value="${l.y1}" id="y1${l.id}" size="3"></td>
          <td><input value="${l.x2}" id="x2${l.id}" size="3"></td>
          <td><input value="${l.y2}" id="y2${l.id}" size="3"></td>
          <td>
            <button onclick="updateLine(${l.id})">Save</button>
            <button onclick="deleteLine(${l.id})">Delete</button>
          </td>
        </tr>`;
    });

  });
}

/* ---------- UPDATE TEXT ---------- */
function updateText(id){
  let f=new FormData();
  f.append("tid",id);
  f.append("text",document.getElementById("t"+id).value);
  f.append("tx",document.getElementById("tx"+id).value);
  f.append("ty",document.getElementById("ty"+id).value);

  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- DELETE TEXT ---------- */
function deleteText(id){
  let f=new FormData();
  f.append("action","removeText");
  f.append("tremove",id);
  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- UPDATE LINE ---------- */
function updateLine(id){
  let f=new FormData();
  f.append("lid",id);
  f.append("x1",document.getElementById("x1"+id).value);
  f.append("y1",document.getElementById("y1"+id).value);
  f.append("x2",document.getElementById("x2"+id).value);
  f.append("y2",document.getElementById("y2"+id).value);

  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- DELETE LINE ---------- */
function deleteLine(id){
  let f=new FormData();
  f.append("action","removeLine");
  f.append("lremove",id);
  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- PAGE LOAD ---------- */
window.onload = ()=>{
  createPages();   // page buttons
  loadTable();     // data load
};
</script>


</div>
</body>
</html>
)rawliteral";


/***********tabular data*****************/
const char* tablePage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>DMD TABLE VIEW</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
*{
  box-sizing:border-box;
  font-family:'Segoe UI',sans-serif;
}

/* ---------- BODY ---------- */
body{
  background:linear-gradient(135deg,#020617,#0f172a);
  color:#e5e7eb;
  padding:30px;
}

/* ---------- HEADER ---------- */
.header{
  text-align:center;
  margin-bottom:32px;
}
.header h1{
  margin:0;
  font-size:34px;
  color:#93c5fd;
  letter-spacing:1px;
  text-shadow:0 0 15px rgba(147,197,253,0.25);
}
.header p{
  margin-top:6px;
  color:#9ca3af;
  font-size:14px;
}

/* ---------- PAGE SWITCH ---------- */
#pageBar{
  display:flex;
  justify-content:center;
  gap:14px;
  margin-bottom:32px;
}
.pageBtn{
  padding:10px 22px;
  background:#1f2937;
  color:white;
  border:none;
  border-radius:14px;
  cursor:pointer;
  font-weight:600;
  transition:0.25s ease;
}
.pageBtn:hover{
  background:#334155;
  transform:translateY(-1px);
}
.pageBtn.active{
  background:#22c55e;
  color:black;
  box-shadow:0 0 0 3px rgba(34,197,94,0.25);
}

/* ---------- SECTIONS ---------- */
.section{
  background:rgba(255,255,255,0.05);
  backdrop-filter:blur(12px);
  padding:24px;
  border-radius:22px;
  margin-bottom:30px;
  border:1px solid rgba(255,255,255,0.08);
  transition:0.25s ease;
}
.section:hover{
  box-shadow:0 12px 38px rgba(0,0,0,0.4);
  transform:translateY(-2px);
}

/* Section header row */
.section-header{
  display:flex;
  justify-content:space-between;
  align-items:center;
  margin-bottom:14px;
}

.section h2{
  margin:0;
  color:#60a5fa;
}
/* --------- FORCE PILL INPUT IN TABLE ---------- */
td{
  padding:14px;                 /* space around input */
}

td input{
  width:90% !important;         /* not full width */
  height:42px;                  /* fixed height */
  border-radius:999px !important;
  background:#ffffff !important;
  color:#020617 !important;
  border:1px solid #e5e7eb !important;
  box-shadow:0 4px 10px rgba(0,0,0,0.25);
  display:block;
  margin:auto;
}

/* ---------- TABLE ---------- */
table{
  width:100%;
  border-collapse:collapse;
}
th,td{
  border:1px solid #334155;
  padding:12px;
  text-align:center;
}
th{
  background:#1e293b;
  color:#93c5fd;
}

/* ---------- INPUTS ---------- */
input[type="text"],
input[type="number"]{
width: 50%;
min-width: 60;
padding: 15px 18px;
  font-size:15px;
  border-radius:999px;           /* full pill shape */
  border:1px solid #e5e7eb;
  background:#ffffff;            /* white like Save/Delete */
  color:#020617;
  text-align:center;
  outline:none;
  transition:all 0.2s ease;
  box-shadow:0 2px 6px rgba(0,0,0,0.15);
}

input[type="text"]:hover,
input[type="number"]:hover{
  box-shadow:0 4px 10px rgba(0,0,0,0.25);
}

input[type="text"]:focus,
input[type="number"]:focus{
  border-color:#3b82f6;
  box-shadow:
    0 0 0 3px rgba(59,130,246,0.25),
    0 6px 14px rgba(0,0,0,0.3);
  transform:translateY(-1px);
}

/* Text column wide */
#textTable td:nth-child(2) input{
  min-width:220px;
  text-align:left;
}


/* ---------- CHECKBOX ---------- */
input[type="checkbox"]{
  transform:scale(1);
  accent-color:#22c55e;
  cursor:pointer;
}

/* ---------- ROW HOVER ---------- */
tbody tr:hover{
  background:rgba(148,163,184,0.08);
}

/* ---------- BRIGHTNESS ---------- */
.brightness-box{
  background:rgba(255,255,255,0.05);
  border:1px solid rgba(255,255,255,0.08);
  border-radius:22px;
  padding: 11px 19px;
  margin-top:30px;
}
.brightness-box h2{
  margin:0 0 16px;
  color:#60a5fa;
  text-align:center;
}
.brightness-row{
  display:flex;
  gap:16px;
  justify-content:center;
  align-items:center;
}

/* ---------- BUTTONS ---------- */
button{
  padding:12px 18px;
  border:none;
  border-radius:14px;
  cursor:pointer;
  font-weight:600;
  transition:0.25s ease;
}
button:hover{
  transform:translateY(-1px);
  filter:brightness(1.1);
}

.btn-add{background:#22c55e;color:black}
.btn-save{background:#3b82f6;color:white}
.btn-del{background:#ef4444;color:white}
.btn-undo{background:#facc15;color:black}
.btn-clear{background:#dc2626;color:white}

/* ---------- ACTION BAR ---------- */
.actions{
  display:flex;
  gap:16px;
  justify-content:center;
  margin-top:22px;
}

/* ---------- BACK ---------- */
.back{
  background:#22c55e;
  margin-top:30px;
}
.back:hover{
  background:#16a34a;
}
</style>


</head>

<body>

<div class="header">
  <h1>DMD Designer Tool</h1>
  <p>LED Layout Editor</p>
</div>

<button class="btn-save" onclick="apiOn()">API ON (This Page)</button>




<div id="pageBar">
  <button class="pageBtn active">Text Manager</button>
  <button class="pageBtn">Line Manager</button>
</div>

<div class="section">
  <h2 style="text-align:center">Page Auto Loop</h2>

  <div class="actions">
    <input type="number" id="loopTime" min="1" max="3600" value="5">
    <button class="btn-save" onclick="loopOn()">LOOP ON</button>
    <button class="btn-clear" onclick="loopOff()">LOOP OFF</button>
  </div>
</div>


<div class="section">
  <div class="section-header">
    <h2>Texts</h2>
    <button class="btn-add" onclick="addText()">+ Add Text</button>
  </div>

  <table id="textTable">
    <thead>
      <tr>
        <th>ID</th><th>Text</th><th>X</th><th>Y</th><th>Marquee</th><th>Action</th>
      </tr>
    </thead>
    <tbody></tbody>
  </table>
</div>

<div class="section">
  <div class="section-header">
    <h2>Lines</h2>
    <button class="btn-add" onclick="addLine()">+ Add Line</button>
  </div>

  <table id="lineTable">
    <thead>
      <tr>
        <th>ID</th><th>X1</th><th>Y1</th><th>X2</th><th>Y2</th><th>Action</th>
      </tr>
    </thead>
    <tbody></tbody>
  </table>
</div>

<div class="brightness-box">
  <h2>Display Brightness</h2>
  <div class="brightness-row">
    <input type="number" min="1" max="255" value="40" id="brightnessInput">
    <button class="btn-save" onclick="updateBrightnessValue()">APPLY</button>
  </div>
</div>
<br>
</br>

<div class="section">
  <h2 style="text-align:center;margin-bottom:14px;">Actions</h2>
  <div class="actions">
    <button class="btn-undo" onclick="undo()">UNDO</button>
    <button class="btn-clear" onclick="clearAll()">CLEAR</button>
    <button class="btn-save" onclick="exportData()">EXPORT</button>
    <button class="btn-save" onclick="document.getElementById('importFile').click()">IMPORT</button>
  </div>
  <input type="file" id="importFile" accept=".json" style="display:none" onchange="importData(this)">
</div>

<div style="text-align:center;">
  <button class="back" onclick="window.location.href='/dashboard'">
    Open Dashboard
  </button>
</div>

<script>
let currentPage = 0;
const MAX_PAGES = 4;

/* ---------- PAGE BUTTONS ---------- */
function createPages(){
  let bar = document.getElementById("pageBar");
  bar.innerHTML = "";

  for(let i=0;i<MAX_PAGES;i++){
    bar.innerHTML +=
      `<button class="pageBtn" id="p${i}"
        onclick="setPage(${i})">
        Page ${i+1}
      </button>`;
  }
  highlightPage();
}

function setPage(p){
  let f=new FormData();
  f.append("p",p);

  fetch("/page",{method:"POST",body:f})
  .then(()=>{
    currentPage = p;
    highlightPage();
    loadTable();
  });
}

function highlightPage(){
  for(let i=0;i<MAX_PAGES;i++){
    let btn=document.getElementById("p"+i);
    if(btn) btn.classList.remove("active");
  }
  let active=document.getElementById("p"+currentPage);
  if(active) active.classList.add("active");
}

/* ---------- API CONTROL ---------- */
function apiOn(){
  let f = new FormData();
  f.append("api","on");

  fetch("/set", { method: "POST", body: f });
}

function apiOff(){
  let f = new FormData();
  f.append("api","off");

  fetch("/set", { method: "POST", body: f });
}

/* ---------- PAGE LOOP ON ---------- */
function loopOn(){
  let f = new FormData();
  f.append("loop","on");

  let t = document.getElementById("loopTime").value;
  f.append("looptime", t);

  fetch("/set", { method: "POST", body: f });
}

/* ---------- PAGE LOOP OFF ---------- */
function loopOff(){
  let f = new FormData();
  f.append("loop","off");

  fetch("/set", { method: "POST", body: f });
}

/* ---------- CLEAR ---------- */
function clearAll(){
  let f=new FormData();
  f.append("action","clearAll");
  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- UNDO ---------- */
function undo(){
  let f=new FormData();
  f.append("action","undo");
  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- EXPORT ---------- */
function exportData(){
  window.location.href="/export";
}

/* ---------- IMPORT ---------- */
function importData(input){
  let f=new FormData();
  f.append("file",input.files[0]);
  fetch("/import",{method:"POST",body:f})
  .then(()=>{ loadTable(); });
}

/* ---------- ADD TEXT ---------- */
function addText(){
  fetch("/data")
  .then(r=>r.json())
  .then(d=>{
    let id = -1;
    for(let i=0;i<50;i++){
      if(!d.texts.find(t => t.id === i)){
        id=i; break;
      }
    }
    if(id === -1){
      alert("No empty Text ID available");
      return;
    }

    let f = new FormData();
    f.append("tid", id);
    f.append("text", "NEW TEXT");
    f.append("tx", 0);
    f.append("ty", 0);

    fetch("/set",{method:"POST",body:f}).then(loadTable);
  });
}

/* ---------- ADD LINE ---------- */
function addLine(){
  fetch("/data")
  .then(r=>r.json())
  .then(d=>{
    let id = -1;
    for(let i=0;i<50;i++){
      if(!d.lines.find(l => l.id === i)){
        id=i; break;
      }
    }
    if(id === -1){
      alert("No empty Line ID available");
      return;
    }

    let f = new FormData();
    f.append("lid", id);
    f.append("x1", 0);
    f.append("y1", 0);
    f.append("x2", 10);
    f.append("y2", 0);

    fetch("/set",{method:"POST",body:f}).then(loadTable);
  });
}

/* ---------- LOAD TABLE ---------- */
function loadTable(){
  fetch("/data")
  .then(r=>r.json())
  .then(d=>{

    let tb=document.querySelector("#textTable tbody");
    tb.innerHTML="";
    d.texts.forEach(t=>{
      tb.innerHTML+=`
      <tr>
        <td>${t.id}</td>
        <td><input value="${t.text}" id="t${t.id}"></td>
        <td><input value="${t.x}" id="tx${t.id}"></td>
        <td><input value="${t.y}" id="ty${t.id}"></td>
        <td>
          <input type="checkbox" id="m${t.id}" ${t.marquee ? "checked" : ""}>
        </td>
        <td>
          <button onclick="updateText(${t.id})">Save</button>
          <button onclick="delText(${t.id})">Delete</button>
        </td>
      </tr>`;
    });

    let lb=document.querySelector("#lineTable tbody");
    lb.innerHTML="";
    d.lines.forEach(l=>{
      lb.innerHTML+=`
      <tr>
        <td>${l.id}</td>
        <td><input value="${l.x1}" id="x1${l.id}"></td>
        <td><input value="${l.y1}" id="y1${l.id}"></td>
        <td><input value="${l.x2}" id="x2${l.id}"></td>
        <td><input value="${l.y2}" id="y2${l.id}"></td>
        <td>
          <button onclick="updateLine(${l.id})">Save</button>
          <button onclick="delLine(${l.id})">Delete</button>
        </td>
      </tr>`;
    });
  });
}

/* ---------- UPDATE TEXT ---------- */
function updateText(id){
  let f=new FormData();
  f.append("tid",id);
  f.append("text",document.getElementById("t"+id).value);
  f.append("tx",document.getElementById("tx"+id).value);
  f.append("ty",document.getElementById("ty"+id).value);

  if(document.getElementById("m"+id).checked){
    f.append("marquee","on");
  }

  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- UPDATE LINE ---------- */
function updateLine(id){
  let f=new FormData();
  f.append("lid",id);
  f.append("x1",document.getElementById("x1"+id).value);
  f.append("y1",document.getElementById("y1"+id).value);
  f.append("x2",document.getElementById("x2"+id).value);
  f.append("y2",document.getElementById("y2"+id).value);

  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- DELETE TEXT ---------- */
function delText(id){
  let f=new FormData();
  f.append("action","removeText");
  f.append("tremove",id);
  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- DELETE LINE ---------- */
function delLine(id){
  let f=new FormData();
  f.append("action","removeLine");
  f.append("lremove",id);
  fetch("/set",{method:"POST",body:f}).then(loadTable);
}

/* ---------- PAGE LOAD ---------- */
window.onload = ()=>{
  createPages();
  loadTable();
};
</script>

</body>
</html>
)rawliteral";


// void clearApiDataForPage(int p) {
//   for (int i = 0; i <= 11; i++) {
//     pages[p].texts[i].enable = false;
//     pages[p].texts[i].marquee = false;
//     pages[p].texts[i].offset = 0;
//   }
// }




/******************** HANDLERS ********************/
void handleRoot() {
  server.send(200, "text/html", tablePage);
}


void handleSet() {

  String action = server.hasArg("action") ? server.arg("action") : "save";

  /******** API PAGE TOGGLE ********/
  if (server.hasArg("api")) {

    if (server.arg("api") == "on") {
      apiEnabledForPage[currentPage] = true;
      fetchEnggenvData();
    } 
    else {
      apiEnabledForPage[currentPage] = false;

      for (int i = 0; i < MAX_TEXT; i++) {
        if (pages[currentPage].texts[i].isApi) {
          pages[currentPage].texts[i].enable = false;
        }
      }

      drawDashboard();
    }

    server.send(200, "text/plain", "API TOGGLED");
    return;
  }

  /******** UNDO ********/
  if (action == "undo") {
    undoLast();
    server.send(200, "text/plain", "UNDO");
    return;
  }

  /******** CLEAR ALL ********/
  if (action == "clearAll") {
    clearAllData();
    server.send(200, "text/plain", "CLEARED");
    return;
  }

  /******** REMOVE TEXT ********/
  if (action == "removeText") {
    int id = server.arg("tremove").toInt();
    if (id >= 0 && id < MAX_TEXT) {
      backupState();
      pages[currentPage].texts[id].enable = false;
      pages[currentPage].texts[id].isApi = false;
      saveToFS();
      drawDashboard();
    }
    server.send(200, "text/plain", "TEXT REMOVED");
    return;
  }

  /******** REMOVE LINE ********/
  if (action == "removeLine") {
    int id = server.arg("lremove").toInt();
    if (id >= 0 && id < MAX_LINES) {
      backupState();
      pages[currentPage].lines[id].enable = false;
      saveToFS();
      drawDashboard();
    }
    server.send(200, "text/plain", "LINE REMOVED");
    return;
  }

  /******** SAVE / UPDATE ********/
  backupState();

  // ----- BRIGHTNESS -----
  if (server.hasArg("brightness")) {
    brightnessValue = constrain(server.arg("brightness").toInt(), 1, 255);
    dmd.setBrightness(brightnessValue);
  }

  /******** PAGE LOOP CONTROL ********/
  if (server.hasArg("loop")) {

    String loopState = server.arg("loop");

    if (loopState == "on") {
      pageLoopEnabled = true;
    } else {
      pageLoopEnabled = false;
    }

    lastPageSwitch = millis();  // reset timer
  }

  if (server.hasArg("looptime")) {
    int sec = server.arg("looptime").toInt();

    if (sec >= 1 && sec <= 3600) {
      pageInterval = sec * 1000UL;
    }
  }

  // ----- TEXT SAVE -----
  if (server.hasArg("tid")) {
    int id = server.arg("tid").toInt();

    if (id >= 0 && id < MAX_TEXT) {

      server.arg("text").toCharArray(
        pages[currentPage].texts[id].text, 30);

      pages[currentPage].texts[id].x = server.arg("tx").toInt();
      pages[currentPage].texts[id].y = server.arg("ty").toInt();
      pages[currentPage].texts[id].enable = true;
      pages[currentPage].texts[id].isApi = false;

      pages[currentPage].texts[id].marquee =
        server.hasArg("marquee");
    }
  }

  // ----- LINE SAVE -----
  if (server.hasArg("lid")) {
    int id = server.arg("lid").toInt();

    if (id >= 0 && id < MAX_LINES) {

      pages[currentPage].lines[id].x1 = server.arg("x1").toInt();
      pages[currentPage].lines[id].y1 = server.arg("y1").toInt();
      pages[currentPage].lines[id].x2 = server.arg("x2").toInt();
      pages[currentPage].lines[id].y2 = server.arg("y2").toInt();
      pages[currentPage].lines[id].enable = true;
    }
  }

  saveToFS();
  drawDashboard();
  server.send(200, "text/plain", "OK");
}


// handel data new ()
void handleData() {
  StaticJsonDocument<4096> doc;

  doc["brightness"] = brightnessValue;
  doc["page"] = currentPage;  // optional but useful
  // -------- TEXTS --------
  JsonArray t = doc.createNestedArray("texts");
  for (int i = 0; i < MAX_TEXT; i++) {
    if (pages[currentPage].texts[i].enable) {

      JsonObject o = t.createNestedObject();
      o["id"] = i;
      o["text"] = pages[currentPage].texts[i].text;
      o["x"] = pages[currentPage].texts[i].x;
      o["y"] = pages[currentPage].texts[i].y;
      o["enable"] = true;
      o["marquee"] = pages[currentPage].texts[i].marquee;
    }
  }

  // -------- LINES --------

  JsonArray l = doc.createNestedArray("lines");
  for (int i = 0; i < MAX_LINES; i++) {
    if (pages[currentPage].lines[i].enable) {

      JsonObject o = l.createNestedObject();
      o["id"] = i;
      o["x1"] = pages[currentPage].lines[i].x1;
      o["y1"] = pages[currentPage].lines[i].y1;
      o["x2"] = pages[currentPage].lines[i].x2;
      o["y2"] = pages[currentPage].lines[i].y2;
      o["enable"] = true;
    }
  }

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}
 
 //ALWAY CHANGE

void fetchEnggenvData() {

  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, "https://api.enggenv.com/api/fetchdata?id=ENE05666");

  int httpCode = http.GET();
  if (httpCode != 200) {
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<4096> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) return;

  int p = currentPage;
  if (!apiEnabledForPage[p]) return;

  // ---- CLEAR OLD API TEXT ----
  for (int i = 0; i < MAX_TEXT; i++) {
    if (pages[p].texts[i].isApi) {
      pages[p].texts[i].enable = false;
      pages[p].texts[i].isApi = false;
    }
  }

  int textIndex = 0;
  int yPos = 0;

  for (JsonPair kv : doc.as<JsonObject>()) {

    const char* key = kv.key().c_str();

    // ❌ SKIP THESE PARAMETERS
    if (strcmp(key, "lg") == 0 ||
        strcmp(key, "lt") == 0 ||
        strcmp(key, "bv") == 0 ||
        strcmp(key, "ts_client") == 0 ||
        strcmp(key, "ts_server") == 0) {
      continue;
    }

    if (textIndex + 1 >= MAX_TEXT) break;

    // 🔥 CONVERT KEY TO CAPITAL
    String keyStr = String(key);
    keyStr.toUpperCase();

    String valueStr = kv.value().as<String>();

    // KEY (Capital)
    strncpy(pages[p].texts[textIndex].text, keyStr.c_str(), 30);
    pages[p].texts[textIndex].text[29] = '\0';
    pages[p].texts[textIndex].x = 0;
    pages[p].texts[textIndex].y = yPos;
    pages[p].texts[textIndex].enable = true;
    pages[p].texts[textIndex].isApi = true;
    textIndex++;

    // VALUE
    strncpy(pages[p].texts[textIndex].text, valueStr.c_str(), 30);
    pages[p].texts[textIndex].text[29] = '\0';
    pages[p].texts[textIndex].x = 70;
    pages[p].texts[textIndex].y = yPos;
    pages[p].texts[textIndex].enable = true;
    pages[p].texts[textIndex].isApi = true;
    textIndex++;

    yPos += 8;
  }

  drawDashboard();
}
/******************** SETUP ********************/


void setup() {

  Serial.begin(115200);

  pageLoopEnabled = false;
  currentPage = 0;

  LITTLEFS.begin(true);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  ArduinoOTA.begin();

  /******** INIT ALL PAGES ********/
  for (int p = 0; p < MAX_PAGES; p++) {

    for (int i = 0; i < MAX_TEXT; i++) {
      pages[p].texts[i].enable = false;
      pages[p].texts[i].marquee = false;
      pages[p].texts[i].offset = 0;
      pages[p].texts[i].isApi = false;
    }

    for (int i = 0; i < MAX_LINES; i++) {
      pages[p].lines[i].enable = false;
    }
  }

  loadFromFS();
  dmd.setBrightness(brightnessValue);

  lastPageSwitch = millis();   // Proper timer init

  /******** ROUTES ********/

  server.on("/", handleRoot);
  server.on("/set", HTTP_POST, handleSet);
  server.on("/export", HTTP_GET, handleExport);
  server.on("/data", HTTP_GET, handleData);

  server.on("/dashboard", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/import", HTTP_POST,
    []() {
      server.send(200);
    },
    handleImport
  );

  /******** PAGE SWITCH ROUTE ********/
  server.on("/page", HTTP_POST, []() {

    if (server.hasArg("p")) {

      int p = server.arg("p").toInt();

      if (p >= 0 && p < MAX_PAGES) {

        currentPage = p;
        lastPageSwitch = millis();   // reset timer
        drawDashboard();
      }
    }

    server.send(200, "text/plain", "OK");
  });

  /******** MANUAL API SYNC ROUTE ********/

  server.on("/sync", HTTP_GET, []() {
    fetchEnggenvData();
    server.send(200, "text/plain", "API SYNCED");
  });

  server.begin();

  fetchEnggenvData();   // First boot API fetch
  drawDashboard();
}


/******************** LOOP ********************/
unsigned long marqueeTimer = 0;
const unsigned long MARQUEE_DELAY = 40;  // 40 smooth speed

unsigned long lastAPICall = 0;
const unsigned long API_INTERVAL = 30000; // 60k second  fetch every api in one hour


void loop() {

  dmd.scanDisplayBySPI();
  ArduinoOTA.handle();
  server.handleClient();

  unsigned long now = millis();

  /******** MARQUEE TEXT LOOP ********/

  if (now - marqueeTimer >= MARQUEE_DELAY) {

    marqueeTimer = now;
    bool redraw = false;

    for (int i = 0; i < MAX_TEXT; i++) {

      if (pages[currentPage].texts[i].enable &&
          pages[currentPage].texts[i].marquee) {

        int textWidth = strlen(pages[currentPage].texts[i].text) * 6;

        pages[currentPage].texts[i].x--;

        if (pages[currentPage].texts[i].x <= -textWidth) {
          pages[currentPage].texts[i].x = SCREEN_W;
        }

        redraw = true;
      }
    }

    if (redraw) {
      drawDashboard();
    }
  }

  /******** PAGE AUTO LOOP (UPDATED) ********/

  if (pageLoopEnabled && (now - lastPageSwitch >= pageInterval)) {

    lastPageSwitch = now;

    // Cleaner wrap-around logic

    currentPage = (currentPage + 1) % MAX_PAGES;

    drawDashboard();
  }

  /******** API FETCH LOOP ********/

  if (now - lastAPICall >= API_INTERVAL) {   // 30 sec
    lastAPICall = now;
    fetchEnggenvData();   // Server se data aayega
  }
}
