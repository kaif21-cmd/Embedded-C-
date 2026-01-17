/******************** LIBRARIES ********************/
#include <DMD32.h>
#include "fonts/SystemFont5x7.h"
#include "fonts/Droid_Sans_16.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/******************** CONFIG ********************/
#define DISPLAYS_ACROSS 4
#define DISPLAYS_DOWN   6
#define LED_BUILTIN     2

const char* ssid     = "DMD";
const char* password = "12345678";

/******************** OBJECTS ********************/
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
hw_timer_t *timer = NULL;

/******************** DMD SCAN ISR ********************/
void IRAM_ATTR scanDMD() {
  dmd.scanDisplayBySPI();
}

/******************** SETUP ********************/
void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(3000);
    ESP.restart();
  }

  ArduinoOTA.setHostname("DMD-ESP32");

  ArduinoOTA.onStart([]() {
    timerAlarmDisable(timer);
    dmd.clearScreen(true);
  });

  ArduinoOTA.onEnd([]() {
    timerAlarmEnable(timer);
  });

  ArduinoOTA.begin();

  uint8_t cpuClock = ESP.getCpuFreqMHz();
  timer = timerBegin(0, cpuClock, true);
  timerAttachInterrupt(timer, &scanDMD, true);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);

  dmd.clearScreen(true);
  dmd.setBrightness(600);
}

/******************** 🐕 DOG DRAW ********************/
void drawDog(int x, int y, bool legUp) {
  dmd.drawBox(x + 5, y + 4, x + 20, y + 8, GRAPHICS_NORMAL);   // body
  dmd.drawBox(x, y + 3, x + 5, y + 7, GRAPHICS_NORMAL);       // head
  dmd.drawBox(x + 1, y + 2, x + 1, y + 2, GRAPHICS_NORMAL);   // ear
  dmd.drawLine(x + 20, y + 6, x + 24, y + 4, GRAPHICS_NORMAL);// tail

  if (legUp) {
    dmd.drawLine(x + 7,  y + 9, x + 7,  y + 13, GRAPHICS_NORMAL);
    dmd.drawLine(x + 15, y + 9, x + 15, y + 11, GRAPHICS_NORMAL);
  } else {
    dmd.drawLine(x + 7,  y + 9, x + 7,  y + 11, GRAPHICS_NORMAL);
    dmd.drawLine(x + 15, y + 9, x + 15, y + 13, GRAPHICS_NORMAL);
  }
}

/******************** ANIMATIONS ********************/
void animScrollRL(const char* txt) {
  static int x = 128;
  static unsigned long t = 0;

  if (millis() - t > 40) {
    t = millis();
    dmd.clearScreen(true);
    dmd.selectFont(SystemFont5x7);
    dmd.drawString(x, 44, txt, strlen(txt), GRAPHICS_NORMAL);
    x -= 2;
    if (x < -128) x = 128;
  }
}

void animDrop(const char* txt) {
  static int y = -20;
  static unsigned long t = 0;

  if (millis() - t > 60) {
    t = millis();
    dmd.clearScreen(true);
    dmd.selectFont(Droid_Sans_16);
    dmd.drawString(5, y, txt, strlen(txt), GRAPHICS_NORMAL);
    y += 2;
    if (y > 95) y = -20;
  }
}

void animBlink(const char* txt) {
  static bool on = false;
  static unsigned long t = 0;

  if (millis() - t > 400) {
    t = millis();
    on = !on;
    dmd.clearScreen(true);
    if (on) {
      dmd.selectFont(SystemFont5x7);
      dmd.drawString(25, 44, txt, strlen(txt), GRAPHICS_NORMAL);
    }
  }
}

void animBounceBox() {
  static int x = 0, y = 0, dx = 2, dy = 2;
  static unsigned long t = 0;

  if (millis() - t > 40) {
    t = millis();
    dmd.clearScreen(true);
    dmd.drawBox(x, y, x + 20, y + 10, GRAPHICS_NORMAL);

    x += dx; y += dy;
    if (x <= 0 || x >= 107) dx = -dx;
    if (y <= 0 || y >= 85)  dy = -dy;
  }
}

/************ 🐕 RUNNING DOG ************/
void animRunningDog() {
  static int x = -30;
  static bool leg = false;
  static unsigned long t = 0;

  if (millis() - t > 80) {
    t = millis();
    dmd.clearScreen(true);
    drawDog(x, 55, leg);
    leg = !leg;
    x += 3;
    if (x > 128) x = -30;
  }
}

/************ ⚠️ FUN HACK WARNING ************/
void animHackWarning() {
  static bool blink = false;
  static unsigned long t = 0;

  if (millis() - t > 300) {
    t = millis();
    blink = !blink;

    dmd.clearScreen(true);

    if (blink) {
      dmd.drawBox(0, 0, 127, 95, GRAPHICS_NORMAL);
    }

    dmd.selectFont(SystemFont5x7);
    dmd.drawString(10, 20, "WARNING !!!", 11, GRAPHICS_NORMAL);
    dmd.drawString(5, 40, "YOU ARE NOW", 11, GRAPHICS_NORMAL);
    dmd.drawString(20, 55, "HACKED", 6, GRAPHICS_NORMAL);
  }
}

/******************** LOOP ********************/
void loop() {
  ArduinoOTA.handle();

  static uint8_t mode = 0;
  static unsigned long change = 0;

  if (millis() - change > 8000) {
    change = millis();
    mode++;
    if (mode > 5) mode = 0;
    dmd.clearScreen(true);
  }

  switch (mode) {
    case 0: animScrollRL("LOGAN & KAIF"); break;
    case 1: animDrop("OTA READY");       break;
    case 2: animBlink("ESP32 + DMD");    break;
    case 3: animBounceBox();             break;
    case 4: animRunningDog();            break;  // 🐕
    case 5: animHackWarning();            break;  // ⚠️ FUN
  }
}
