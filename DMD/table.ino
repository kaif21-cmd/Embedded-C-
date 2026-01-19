/******************** LIBRARIES ********************/
#include <DMD32Plus.h>
// #include "fonts/Arial_Black_16.h"
// #include "fonts/SystemFont8x13.h"
#include "fonts/SystemFont5x7.h"
#include "fonts/Droid_Sans_12.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// #include <Wire.h>        // ❌ RTC DISABLED
// #include <RTClib.h>      // ❌ RTC DISABLED

/******************** DISPLAY CONFIG ********************/
#define DISPLAYS_ACROSS 4
#define DISPLAYS_DOWN 6

/******************** PIN CONFIG ********************/
#define OE_PIN 22
#define A_PIN 19
#define B_PIN 21
#define CLK_PIN 18
#define LAT_PIN 4
#define R_DATA_PIN 23

/******************** WIFI ********************/
const char* ssid = "DMD";
const char* password = "12345678";

/******************** OBJECTS ********************/
DMD dmd(
  DISPLAYS_ACROSS, DISPLAYS_DOWN,
  OE_PIN, A_PIN, B_PIN, CLK_PIN, LAT_PIN, R_DATA_PIN);

// RTC_DS3231 rtc;   // ❌ RTC DISABLED

/******************** TIMER ********************/
hw_timer_t* timer = NULL;

/******************** PAGE CONTROL ********************/
uint8_t currentPage = 0;
uint8_t lastPage = 255;
unsigned long lastPageChange = 0;
const unsigned long PAGE_INTERVAL = 5000;

/******************** TIME BUFFERS ********************/
// RTC nahi hai to fixed demo time/date
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

/******************** FOOTER ********************/
void drawFooter() {
 dmd.drawString(27, 84, "L&K PVT LTD.", 13, GRAPHICS_NORMAL);
}

void drawPage1() {

  // ---- Header Big Font ----
dmd.selectFont(Droid_Sans_12);
dmd.drawString(24, 2, "AAQMS DATA", 10, GRAPHICS_NORMAL);

  // ---- Back to Normal Font ----
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

/******************** PAGE 2 (AQI) ********************/
void drawPage2() {

  int aqiValue = random(0, 1001);
  char aqiStr[6];
  sprintf(aqiStr, "%d", aqiValue);

  const char* aqiStatus = getAQIStatus(aqiValue);

  // ---- Header Big & Center ----
dmd.selectFont(Droid_Sans_12);
dmd.drawString(24, 2, "AAQMS DATA", 10, GRAPHICS_NORMAL);


  // ---- Back to Normal Font ----
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
  dmd.selectFont(SystemFont5x7);

  if (currentPage == 0) drawPage1();
  else drawPage2();

  drawFooter();
}

/******************** SETUP ********************/
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(3000);
    ESP.restart();
  }

  ArduinoOTA.begin();

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
  ArduinoOTA.handle();

  if (millis() - lastPageChange >= PAGE_INTERVAL) {
    lastPageChange = millis();
    currentPage = !currentPage;
    drawCurrentPage();
  }
}
