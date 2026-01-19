/******************** LIBRARIES ********************/
#include <DMD32Plus.h>
#include "fonts/SystemFont5x7.h"
#include "fonts/Droid_Sans_12.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/******************** DISPLAY CONFIG ********************/
#define DISPLAYS_ACROSS 4
#define DISPLAYS_DOWN   6

/******************** PIN CONFIG ********************/
#define OE_PIN     22
#define A_PIN      19
#define B_PIN      21
#define CLK_PIN    18
#define LAT_PIN    4
#define R_DATA_PIN 23

/******************** WIFI ********************/
const char* ssid     = "DMD";
const char* password = "12345678";

/******************** OBJECTS ********************/
DMD dmd(
  DISPLAYS_ACROSS, DISPLAYS_DOWN,
  OE_PIN, A_PIN, B_PIN, CLK_PIN, LAT_PIN, R_DATA_PIN);

/******************** TIMER ********************/
hw_timer_t* timer = NULL;

/******************** PAGE CONTROL ********************/
uint8_t currentPage = 0;
unsigned long lastPageChange = 0;
const unsigned long PAGE_INTERVAL = 5000;

/******************** TIME BUFFERS ********************/
char timeStr[] = "10:10";
char dateStr[] = "06-01-2026";

/******************** SCAN ISR ********************/
void IRAM_ATTR triggerScan() {
  dmd.scanDisplayBySPI();
}

/******************** TABLE ********************/
void drawAAQMSTable() {
  dmd.drawLine(0, 0, 127, 0, GRAPHICS_NORMAL);
  dmd.drawLine(0, 95, 127, 95, GRAPHICS_NORMAL);
  dmd.drawLine(0, 0, 0, 95, GRAPHICS_NORMAL);
  dmd.drawLine(127, 0, 127, 95, GRAPHICS_NORMAL);

  dmd.drawLine(0, 16, 127, 16, GRAPHICS_NORMAL);
  dmd.drawLine(0, 32, 127, 32, GRAPHICS_NORMAL);
  dmd.drawLine(0, 80, 127, 80, GRAPHICS_NORMAL);

  dmd.drawLine(48, 16, 48, 80, GRAPHICS_NORMAL);
}

/******************** FOOTER (CENTERED) ********************/
void drawFooter() {
  int textLen = 13;                   // "L&K PVT LTD."
  int charWidth = 6;                 // 5x7 font approx
  int displayWidth = DISPLAYS_ACROSS * 32;
  int textWidth = textLen * charWidth;
  int x = (displayWidth - textWidth) / 2;

  dmd.drawString(x, 84, "L&K PVT LTD.", textLen, GRAPHICS_NORMAL);
}

/******************** PAGE 1 ********************/
void drawPage1() {

  dmd.selectFont(Droid_Sans_12);
  dmd.drawString(24, 2, "AAQMS DATA", 10, GRAPHICS_NORMAL);

  dmd.selectFont(SystemFont5x7);

  dmd.drawString(6, 20, timeStr, strlen(timeStr), GRAPHICS_NORMAL);
  dmd.drawString(55, 20, dateStr, strlen(dateStr), GRAPHICS_NORMAL);

  dmd.drawString(6, 36, "PM2.5", 5, GRAPHICS_NORMAL);
  dmd.drawString(6, 46, "PM10", 4, GRAPHICS_NORMAL);
  dmd.drawString(6, 56, "TEMP", 4, GRAPHICS_NORMAL);
  dmd.drawString(6, 66, "HUM", 3, GRAPHICS_NORMAL);

  dmd.drawString(54, 36, "293 ug/m3", 9, GRAPHICS_NORMAL);
  dmd.drawString(54, 46, "551 ug/m3", 9, GRAPHICS_NORMAL);
  dmd.drawString(54, 56, "18.9 C", 6, GRAPHICS_NORMAL);
  dmd.drawString(54, 66, "67 %", 4, GRAPHICS_NORMAL);
}

/******************** AQI STATUS ********************/
const char* getAQIStatus(int aqi) {
  if (aqi <= 150) return "GOOD";
  else if (aqi <= 300) return "AVERAGE";
  else if (aqi <= 500) return "MODERATE";
  else if (aqi <= 700) return "UNHEALTHY";
  else return "DANGEROUS !!";
}

/******************** PAGE 2 ********************/
void drawPage2() {

  int aqiValue = random(0, 1001);
  char aqiStr[6];
  sprintf(aqiStr, "%d", aqiValue);

  const char* aqiStatus = getAQIStatus(aqiValue);

  dmd.selectFont(Droid_Sans_12);
  dmd.drawString(24, 2, "AAQMS DATA", 10, GRAPHICS_NORMAL);

  dmd.selectFont(SystemFont5x7);

  dmd.drawString(6, 20, timeStr, strlen(timeStr), GRAPHICS_NORMAL);
  dmd.drawString(55, 20, dateStr, strlen(dateStr), GRAPHICS_NORMAL);

  dmd.drawString(6, 48, "AQI", 3, GRAPHICS_NORMAL);
  dmd.drawString(54, 48, aqiStr, strlen(aqiStr), GRAPHICS_NORMAL);
  dmd.drawString(54, 58, aqiStatus, strlen(aqiStatus), GRAPHICS_NORMAL);
}

/******************** DRAW PAGE ********************/
void drawCurrentPage() {
  dmd.clearScreen(true);
  drawAAQMSTable();

  if (currentPage == 0) drawPage1();
  else drawPage2();

  drawFooter();
}

/******************** SETUP ********************/
void setup() {
  Serial.begin(115200);

  // ---- WiFi ----
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(3000);
    ESP.restart();
  }

  Serial.println("WiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // ---- OTA SETUP ----
  ArduinoOTA.setHostname("AAQMS-DMD");

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress * 100) / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]\n", error);
  });

  ArduinoOTA.begin();

  // ---- DMD TIMER ----
  timer = timerBegin(0, 60, true);
  timerAttachInterrupt(timer, &triggerScan, true);
  timerAlarmWrite(timer, 2500, true);
  timerAlarmEnable(timer);

  dmd.setBrightness(20);
  randomSeed(millis());

  drawCurrentPage();
}

/******************** LOOP ********************/
void loop() {
  // OTA must be handled continuously
  ArduinoOTA.handle();

  if (millis() - lastPageChange >= PAGE_INTERVAL) {
    lastPageChange = millis();
    currentPage = !currentPage;
    drawCurrentPage();
  }
}
