/******************** LIBRARIES ********************/
#include <DMD32Plus.h>
#include "fonts/SystemFont5x7.h"
#include "fonts/Droid_Sans_12.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

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

/******************** EEPROM ********************/
#define EEPROM_SIZE 256

#define ADDR_HEADER  0
#define ADDR_FOOTER  50
#define ADDR_DATA    100

/******************** OBJECT ********************/
DMD dmd(
  DISPLAYS_ACROSS, DISPLAYS_DOWN,
  OE_PIN, A_PIN, B_PIN, CLK_PIN, LAT_PIN, R_DATA_PIN);

/******************** PAGE CONTROL ********************/
uint8_t currentPage = 0;
unsigned long lastPageChange = 0;
const unsigned long PAGE_INTERVAL = 5000;

/******************** TIME BUFFERS ********************/
char timeStr[] = "10:10";
char dateStr[] = "06-01-2026";

/******************** DYNAMIC TEXT ********************/
char headerText[30] = "AAQMS DATA";
char footerText[30] = "L&K PVT LTD.";

char pm25Str[20] = "293 ug/m3";
char pm10Str[20] = "551 ug/m3";
char tempStr[20] = "18.9 C";
char humStr[20]  = "67 %";

/******************** EEPROM FUNCTIONS ********************/
void writeEEPROMString(int addr, const char* data) {
  for (int i = 0; i < 30; i++) {
    if (data[i] == '\0') {
      EEPROM.write(addr + i, 0);
      break;
    }
    EEPROM.write(addr + i, data[i]);
  }
  EEPROM.commit();
}

void readEEPROMString(int addr, char* buffer) {
  for (int i = 0; i < 30; i++) {
    char c = EEPROM.read(addr + i);
    if (c == 0 || c == 255) {
      buffer[i] = '\0';
      break;
    }
    buffer[i] = c;
  }
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

/******************** CENTERED HEADER ********************/
void drawCenteredHeader(const char* text) {
  dmd.selectFont(Droid_Sans_12);

  int textLen = strlen(text);
  int charWidth = 8;
  int displayWidth = DISPLAYS_ACROSS * 32;
  int textWidth = textLen * charWidth;

  int x = (displayWidth - textWidth) / 2;
  if (x < 0) x = 0;

  dmd.drawString(x, 2, text, textLen, GRAPHICS_NORMAL);
}

/******************** FOOTER ********************/
void drawFooter() {
  dmd.selectFont(SystemFont5x7);

  int textLen = strlen(footerText);
  int charWidth = 6;
  int displayWidth = DISPLAYS_ACROSS * 32;
  int textWidth = textLen * charWidth;

  int x = (displayWidth - textWidth) / 2;
  if (x < 0) x = 0;

  dmd.drawString(x, 84, footerText, textLen, GRAPHICS_NORMAL);
}

/******************** PAGE 1 ********************/
void drawPage1() {

  drawCenteredHeader(headerText);

  dmd.selectFont(SystemFont5x7);

  dmd.drawString(6, 20, timeStr, strlen(timeStr), GRAPHICS_NORMAL);
  dmd.drawString(55, 20, dateStr, strlen(dateStr), GRAPHICS_NORMAL);

  dmd.drawString(6, 36, "PM2.5", 5, GRAPHICS_NORMAL);
  dmd.drawString(6, 46, "PM10", 4, GRAPHICS_NORMAL);
  dmd.drawString(6, 56, "TEMP", 4, GRAPHICS_NORMAL);
  dmd.drawString(6, 66, "HUM", 3, GRAPHICS_NORMAL);

  dmd.drawString(54, 36, pm25Str, strlen(pm25Str), GRAPHICS_NORMAL);
  dmd.drawString(54, 46, pm10Str, strlen(pm10Str), GRAPHICS_NORMAL);
  dmd.drawString(54, 56, tempStr, strlen(tempStr), GRAPHICS_NORMAL);
  dmd.drawString(54, 66, humStr,  strlen(humStr),  GRAPHICS_NORMAL);
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

  drawCenteredHeader(headerText);

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

/******************** SERIAL HANDLER ********************/
void handleSerial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  Serial.print("Received: ");
  Serial.println(cmd);

  if (cmd.startsWith("H:")) {
    cmd.substring(2).toCharArray(headerText, 30);
    writeEEPROMString(ADDR_HEADER, headerText);
    Serial.print("Header Saved: ");
    Serial.println(headerText);
  }
  else if (cmd.startsWith("F:")) {
    cmd.substring(2).toCharArray(footerText, 30);
    writeEEPROMString(ADDR_FOOTER, footerText);
    Serial.print("Footer Saved: ");
    Serial.println(footerText);
  }
  else if (cmd.startsWith("PM25:")) {
    cmd.substring(5).toCharArray(pm25Str, 20);
    writeEEPROMString(ADDR_DATA + 0, pm25Str);
    Serial.print("PM2.5 Saved: ");
    Serial.println(pm25Str);
  }
  else if (cmd.startsWith("PM10:")) {
    cmd.substring(5).toCharArray(pm10Str, 20);
    writeEEPROMString(ADDR_DATA + 30, pm10Str);
    Serial.print("PM10 Saved: ");
    Serial.println(pm10Str);
  }
  else if (cmd.startsWith("TEMP:")) {
    cmd.substring(5).toCharArray(tempStr, 20);
    writeEEPROMString(ADDR_DATA + 60, tempStr);
    Serial.print("TEMP Saved: ");
    Serial.println(tempStr);
  }
  else if (cmd.startsWith("HUM:")) {
    cmd.substring(4).toCharArray(humStr, 20);
    writeEEPROMString(ADDR_DATA + 90, humStr);
    Serial.print("HUM Saved: ");
    Serial.println(humStr);
  }
  else {
    Serial.println("Unknown Command Format!");
  }

  drawCurrentPage();
}


/******************** SETUP ********************/
void setup() {
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);

  // Read saved values
  readEEPROMString(ADDR_HEADER, headerText);
  readEEPROMString(ADDR_FOOTER, footerText);

  readEEPROMString(ADDR_DATA + 0,  pm25Str);
  readEEPROMString(ADDR_DATA + 30, pm10Str);
  readEEPROMString(ADDR_DATA + 60, tempStr);
  readEEPROMString(ADDR_DATA + 90, humStr);

  // ---- WiFi ----
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(3000);
    ESP.restart();
  }

  // ---- OTA ----
  ArduinoOTA.setHostname("AAQMS-DMD");
  ArduinoOTA.begin();

  dmd.setBrightness(20);
  randomSeed(millis());

  drawCurrentPage();
}

/******************** LOOP ********************/
void loop() {

  // Smooth continuous refresh (NO ISR, NO TIMER)
  dmd.scanDisplayBySPI();

  // OTA safe
  ArduinoOTA.handle();

  // Serial update
  handleSerial();

  // Page switching
  if (millis() - lastPageChange >= PAGE_INTERVAL) {
    lastPageChange = millis();
    currentPage = !currentPage;
    drawCurrentPage();
  }
}
