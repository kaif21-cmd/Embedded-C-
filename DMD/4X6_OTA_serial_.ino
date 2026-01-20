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
#define EEPROM_SIZE 512   // thoda bada rakha

// 🔹 TEXT LENGTH INCREASED
#define TEXT_MAX_LEN 80

// 🔹 EEPROM ADDRESS UPDATED (NO OVERLAP)
#define ADDR_HEADER  0          // 0 – 79
#define ADDR_FOOTER  100        // 100 – 179
#define ADDR_DATA    220        // data aage se

/******************** DMD OBJECT ********************/
DMD dmd(
  DISPLAYS_ACROSS, DISPLAYS_DOWN,
  OE_PIN, A_PIN, B_PIN, CLK_PIN, LAT_PIN, R_DATA_PIN);

/******************** PAGE CONTROL ********************/
uint8_t currentPage = 0;
unsigned long lastPageChange = 0;
const unsigned long PAGE_INTERVAL = 5000;

#define TOTAL_PAGES 2

/******************** STATIC TIME (DEMO) ********************/
char timeStr[] = "10:10";
char dateStr[] = "06-01-2026";

/******************** DEFAULT + DYNAMIC TEXT ********************/
char headerText[TEXT_MAX_LEN] = "AAQMS DATA";
char footerText[TEXT_MAX_LEN] = "L&K PVT LTD.";

char pm25Str[20] = "293 ug/m3";
char pm10Str[20] = "551 ug/m3";
char tempStr[20] = "18.9 C";
char humStr[20]  = "67 %";

/******************** FOOTER MARQUEE ********************/
int footerOffset = 0;
unsigned long lastFooterScroll = 0;
const unsigned long FOOTER_SCROLL_INTERVAL = 50;

/******************** EEPROM WRITE ********************/
void writeEEPROMString(int addr, const char* data) {
  for (int i = 0; i < TEXT_MAX_LEN; i++) {
    if (data[i] == '\0') {
      EEPROM.write(addr + i, 0);
      break;
    }
    EEPROM.write(addr + i, data[i]);
  }
  EEPROM.commit();
}

/******************** EEPROM SAFE READ ********************/
void readEEPROMStringSafe(int addr, char* buffer, const char* defaultValue) {
  bool isEmpty = true;

  for (int i = 0; i < TEXT_MAX_LEN; i++) {
    char c = EEPROM.read(addr + i);
    if (c != 0 && c != 255) {
      isEmpty = false;
      break;
    }
  }

  if (isEmpty) {
    strcpy(buffer, defaultValue);
    return;
  }

  for (int i = 0; i < TEXT_MAX_LEN; i++) {
    char c = EEPROM.read(addr + i);
    if (c == 0 || c == 255) {
      buffer[i] = '\0';
      break;
    }
    buffer[i] = c;
  }
}

/******************** TABLE + FULL BORDER ********************/
void drawAAQMSTable() {
  dmd.drawLine(0, 0, 127, 0, GRAPHICS_NORMAL);
  dmd.drawLine(0, 95, 127, 95, GRAPHICS_NORMAL);
  dmd.drawLine(0, 0, 0, 95, GRAPHICS_NORMAL);
  dmd.drawLine(127, 0, 127, 95, GRAPHICS_NORMAL);

  dmd.drawLine(0, 16, 127, 16, GRAPHICS_NORMAL);
  dmd.drawLine(0, 32, 127, 32, GRAPHICS_NORMAL);
  dmd.drawLine(0, 81, 127, 81, GRAPHICS_NORMAL);

  dmd.drawLine(48, 16, 48, 80, GRAPHICS_NORMAL);
}

/******************** CENTERED HEADER ********************/
void drawCenteredHeader(const char* text) {
  dmd.selectFont(Droid_Sans_12);

  int textLen = strlen(text);
  int displayWidth = DISPLAYS_ACROSS * 32;

  int textWidth = 0;
  for (int i = 0; i < textLen; i++) {
    textWidth += dmd.charWidth(text[i]);
  }

  int x = (displayWidth - textWidth) / 2;
  if (x < 0) x = 0;

  dmd.drawString(x, 2, text, textLen, GRAPHICS_NORMAL);
}

/******************** FOOTER DRAW ********************/
void drawFooter() {
  dmd.selectFont(SystemFont5x7);

  int textLen = strlen(footerText);
  int charWidth = 6;
  int displayWidth = DISPLAYS_ACROSS * 32;
  int textWidth = textLen * charWidth;

  int leftLimit  = 2;
  int rightLimit = displayWidth - 2;
  int safeWidth  = rightLimit - leftLimit;

  if (textWidth <= safeWidth) {
    int x = leftLimit + (safeWidth - textWidth) / 2;
    if (x < leftLimit) x = leftLimit;
    dmd.drawString(x, 84, footerText, textLen, GRAPHICS_NORMAL);
  } 
  else {
    int startX = rightLimit + 10;
    int x = startX - footerOffset;

    if (x < leftLimit - textWidth - 5) return;

    dmd.drawString(x, 84, footerText, textLen, GRAPHICS_NORMAL);
  }
}

/******************** ONLY FOOTER REDRAW (BORDER CONTROL) ********************/
void redrawFooterOnly() {

  // Footer inner clear
  dmd.drawFilledBox(1, 82, 126, 94, GRAPHICS_INVERSE);

  int textLen = strlen(footerText);
  int charWidth = 6;
  int displayWidth = DISPLAYS_ACROSS * 32;
  int textWidth = textLen * charWidth;

  bool marqueeActive = (textWidth > (displayWidth - 4));

  if (marqueeActive) {
    // Sirf footer ke left/right border hide
    dmd.drawFilledBox(0, 82, 0, 94, GRAPHICS_INVERSE);
    dmd.drawFilledBox(127, 82, 127, 94, GRAPHICS_INVERSE);
  } 
  else {
    // Footer border wapas
    dmd.drawLine(0, 82, 0, 94, GRAPHICS_NORMAL);
    dmd.drawLine(127, 82, 127, 94, GRAPHICS_NORMAL);
  }

  drawFooter();
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

/******************** AQI LOGIC ********************/
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

/******************** FULL PAGE DRAW ********************/
void drawCurrentPage() {

  dmd.clearScreen(true);
  drawAAQMSTable();

  switch (currentPage) {
    case 0: drawPage1(); break;
    case 1: drawPage2(); break;
    default: drawPage1(); break;
  }

  drawFooter();

  // Initial footer border state
  int textLen = strlen(footerText);
  int charWidth = 6;
  int displayWidth = DISPLAYS_ACROSS * 32;
  int textWidth = textLen * charWidth;

  if (textWidth > (displayWidth - 4)) {
    dmd.drawFilledBox(0, 82, 0, 94, GRAPHICS_INVERSE);
    dmd.drawFilledBox(127, 82, 127, 94, GRAPHICS_INVERSE);
  }
}

/******************** SERIAL HANDLER ********************/
void handleSerial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.startsWith("H:")) {
    cmd.substring(2).toCharArray(headerText, TEXT_MAX_LEN);
    writeEEPROMString(ADDR_HEADER, headerText);
  }
  else if (cmd.startsWith("F:")) {
    cmd.substring(2).toCharArray(footerText, TEXT_MAX_LEN);
    writeEEPROMString(ADDR_FOOTER, footerText);
    footerOffset = 0;
  }
  else if (cmd.startsWith("PM25:")) {
    cmd.substring(5).toCharArray(pm25Str, 20);
    writeEEPROMString(ADDR_DATA + 0, pm25Str);
  }
  else if (cmd.startsWith("PM10:")) {
    cmd.substring(5).toCharArray(pm10Str, 20);
    writeEEPROMString(ADDR_DATA + 30, pm10Str);
  }
  else if (cmd.startsWith("TEMP:")) {
    cmd.substring(5).toCharArray(tempStr, 20);
    writeEEPROMString(ADDR_DATA + 60, tempStr);
  }
  else if (cmd.startsWith("HUM:")) {
    cmd.substring(4).toCharArray(humStr, 20);
    writeEEPROMString(ADDR_DATA + 90, humStr);
  }

  drawCurrentPage();
}

/******************** SETUP ********************/
void setup() {
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);

  readEEPROMStringSafe(ADDR_HEADER, headerText, "AAQMS DATA");
  readEEPROMStringSafe(ADDR_FOOTER, footerText, "L&K PVT LTD.");

  readEEPROMStringSafe(ADDR_DATA + 0,  pm25Str, "293 ug/m3");
  readEEPROMStringSafe(ADDR_DATA + 30, pm10Str, "551 ug/m3");
  readEEPROMStringSafe(ADDR_DATA + 60, tempStr, "18.9 C");
  readEEPROMStringSafe(ADDR_DATA + 90, humStr,  "67 %");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(3000);
    ESP.restart();
  }

  ArduinoOTA.setHostname("AAQMS-DMD");
  ArduinoOTA.begin();

  dmd.setBrightness(20);
  randomSeed(millis());

  drawCurrentPage();
}

/******************** LOOP ********************/
void loop() {

  dmd.scanDisplayBySPI();
  ArduinoOTA.handle();
  handleSerial();

  // Footer smooth scroll
  if (millis() - lastFooterScroll >= FOOTER_SCROLL_INTERVAL) {
    lastFooterScroll = millis();

    int textLen = strlen(footerText);
    int charWidth = 6;
    int displayWidth = DISPLAYS_ACROSS * 32;
    int textWidth = textLen * charWidth;

    if (textWidth > (displayWidth - 4)) {
      footerOffset++;

      if (footerOffset > textWidth + displayWidth + 20) {
        footerOffset = 0;
      }

      redrawFooterOnly();
    } 
    else {
      footerOffset = 0;
    }
  }

  // Page change
  if (millis() - lastPageChange >= PAGE_INTERVAL) {
    lastPageChange = millis();

    currentPage++;
    if (currentPage >= TOTAL_PAGES) {
      currentPage = 0;
    }

    drawCurrentPage();
  }
}
