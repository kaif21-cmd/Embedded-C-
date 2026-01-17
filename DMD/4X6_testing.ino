#include <DMD32.h>
#include "fonts/SystemFont5x7.h"
#include "fonts/Droid_Sans_16.h"

#define DISPLAYS_ACROSS 4
#define DISPLAYS_DOWN   6

DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

// ================= TIMER =================
hw_timer_t *timer = NULL;

void IRAM_ATTR scanDMD() {
  dmd.scanDisplayBySPI();
}

void setup() {
  Serial.begin(115200);

  // ---------- TIMER SETUP ----------
  uint8_t cpuClock = ESP.getCpuFreqMHz();
  timer = timerBegin(0, cpuClock, true);
  timerAttachInterrupt(timer, &scanDMD, true);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);

  // ---------- DMD INIT ----------
  dmd.clearScreen(true);
  dmd.setBrightness(600);   // 🔅 LOW BRIGHTNESS (safe)
}

void loop() {

  // ================= TEST 1: CENTER SHORT TEXT =================
  dmd.clearScreen(true);
  dmd.selectFont(SystemFont5x7);
  dmd.drawString(48, 44, "4x6 OK", 6, GRAPHICS_NORMAL);
  delay(2000);

  // ================= TEST 2: BIG NAME =================
  dmd.clearScreen(true);
  dmd.selectFont(Droid_Sans_16);
  dmd.drawString(40, 40, "KAIF", 4, GRAPHICS_NORMAL);
  delay(3000);

  // ================= TEST 3: BORDER =================
  dmd.clearScreen(true);
  dmd.drawBox(0, 0, 127, 95, GRAPHICS_NORMAL);
  delay(2000);

  // ================= TEST 4: CROSS =================
  dmd.clearScreen(true);
  dmd.drawLine(0, 0, 127, 95, GRAPHICS_NORMAL);
  dmd.drawLine(0, 95, 127, 0, GRAPHICS_NORMAL);
  delay(2000);

  // ================= TEST 5: BLINK =================
  dmd.clearScreen(true);
  delay(400);
  dmd.clearScreen(false);
  delay(400);
  dmd.clearScreen(true);

  // ================= TEST 6: VERY VERY SLOW MARQUEE =================
  dmd.clearScreen(true);
  dmd.selectFont(SystemFont5x7);

  dmd.drawMarquee(
    "   HELLO FROM KAIF   |   4x6 DMD TESTING   |   ESP32 OK   ",
    64,
    (32 * DISPLAYS_ACROSS) - 1,
    20
  );

  unsigned long t = millis();

  while (true) {
    if (millis() - t > 120) {   
      t = millis();

      if (dmd.stepMarquee(-1, 0)) {
        break;   // marquee complete
      }
    }
  }

  delay(1000);  // pause before repeating loop
}


tested good
