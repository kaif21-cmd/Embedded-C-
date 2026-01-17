/******************** LIBRARIES ********************/
#include <DMD32Plus.h>
#include "fonts/SystemFont5x7.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/******************** DISPLAY CONFIG ********************/
#define DISPLAYS_ACROSS 4
#define DISPLAYS_DOWN   6

#define PANEL_W 32
#define PANEL_H 16

#define SCREEN_W (DISPLAYS_ACROSS * PANEL_W)   // 128
#define SCREEN_H (DISPLAYS_DOWN   * PANEL_H)   // 96

/******************** PIN CONFIG ********************/
#define OE_PIN     22
#define A_PIN      19
#define B_PIN      21
#define CLK_PIN    18
#define LAT_PIN    4
#define R_DATA_PIN 23

#define LED_BUILTIN 2

/******************** WIFI ********************/
const char* ssid     = "DMD";
const char* password = "12345678";

/******************** DMD OBJECT ********************/
DMD dmd(
  DISPLAYS_ACROSS,
  DISPLAYS_DOWN,
  OE_PIN,
  A_PIN,
  B_PIN,
  CLK_PIN,
  LAT_PIN,
  R_DATA_PIN
);

/******************** TIMER ********************/
hw_timer_t *timer = NULL;

/******************** SCAN ISR ********************/
void IRAM_ATTR triggerScan() {
  dmd.scanDisplayBySPI();
}

/******************** AAQMS TABLE (2-COLUMN ENHANCED) ********************/
void drawAAQMSTable() {

  // ===== OUTER BORDER =====
  dmd.drawLine(0, 0, 127, 0, GRAPHICS_NORMAL);
  dmd.drawLine(0, 95, 127, 95, GRAPHICS_NORMAL);
  dmd.drawLine(0, 0, 0, 95, GRAPHICS_NORMAL);
  dmd.drawLine(127, 0, 127, 95, GRAPHICS_NORMAL);

  // ===== HORIZONTAL SECTIONS =====
  dmd.drawLine(0, 16, 127, 16, GRAPHICS_NORMAL); // header
  dmd.drawLine(0, 32, 127, 32, GRAPHICS_NORMAL); // time/date
  dmd.drawLine(0, 80, 127, 80, GRAPHICS_NORMAL); // footer

  // ===== SINGLE VERTICAL LINE =====
  dmd.drawLine(48, 32, 48, 80, GRAPHICS_NORMAL); // label | value+unit
}

/******************** SETUP ********************/
void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(3000);
    ESP.restart();
  }

  // OTA
  ArduinoOTA.setHostname("AAQMS-ESP32");
  ArduinoOTA.begin();

  // TIMER
  timer = timerBegin(0, 60, true);
  timerAttachInterrupt(timer, &triggerScan, true);
  timerAlarmWrite(timer, 2500, true);
  timerAlarmEnable(timer);

  // DMD
  dmd.setBrightness(8);
  dmd.clearScreen(true);
}

/******************** LOOP ********************/
void loop() {
  ArduinoOTA.handle();

  dmd.clearScreen(true);
  drawAAQMSTable();
  dmd.selectFont(SystemFont5x7);

  // ===== HEADER =====
  dmd.drawString(36, 4, "AAQMS DATA", 10, GRAPHICS_NORMAL);

  // ===== TIME / DATE =====
  dmd.drawString(6, 20,  "10:10", 5, GRAPHICS_NORMAL);
  dmd.drawString(70, 20, "06-01-2026", 10, GRAPHICS_NORMAL);

  // ===== LABELS =====
  dmd.drawString(6, 36, "PM2.5", 5, GRAPHICS_NORMAL);
  dmd.drawString(6, 46, "PM10", 4, GRAPHICS_NORMAL);
  dmd.drawString(6, 56, "TEMP", 4, GRAPHICS_NORMAL);
  dmd.drawString(6, 66, "HUM", 3, GRAPHICS_NORMAL);

  // ===== VALUE + UNIT (SINGLE COLUMN) =====
  dmd.drawString(54, 36, "293 ug/m3", 9, GRAPHICS_NORMAL);
  dmd.drawString(54, 46, "551 ug/m3", 9, GRAPHICS_NORMAL);
  dmd.drawString(54, 56, "18.9 C",   6, GRAPHICS_NORMAL);
  dmd.drawString(54, 66, "67 %",     4, GRAPHICS_NORMAL);

  // ===== FOOTER =====
  dmd.drawString(22, 84, "", 15, GRAPHICS_NORMAL);

  delay(2000);
}
