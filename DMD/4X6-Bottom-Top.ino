#include <DMD32.h>
#include "fonts/SystemFont5x7.h"

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

  uint8_t cpuClock = ESP.getCpuFreqMHz();
  timer = timerBegin(0, cpuClock, true);
  timerAttachInterrupt(timer, &scanDMD, true);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);

  dmd.clearScreen(true);
  dmd.setBrightness(600);
  dmd.selectFont(SystemFont5x7);
}

void loop() {

  // =================================================
  // =============== MARQUEE 1 (TOP) =================
  // =================================================
  dmd.clearScreen(true);

  dmd.drawMarquee(
    "   TOP MARQUEE TEXT RUNNING   ",
    28,
    (32 * DISPLAYS_ACROSS) - 1,
    2          // 🔼 TOP
  );

  unsigned long t = millis();
  while (true) {
    if (millis() - t > 160) {
      t = millis();
      if (dmd.stepMarquee(-1, 0)) break;
    }
  }

  delay(1000);   // gap after top marquee

  // =================================================
  // ============== MARQUEE 2 (BOTTOM) ===============
  // =================================================
  dmd.clearScreen(true);

  dmd.drawMarquee(
    "   BOTTOM MARQUEE TEXT RUNNING   ",
    31,
    (32 * DISPLAYS_ACROSS) - 1,
    86         // 🔽 BOTTOM
  );

  t = millis();
  while (true) {
    if (millis() - t > 160) {
      t = millis();
      if (dmd.stepMarquee(-1, 0)) break;
    }
  }

  delay(3000);   // full cycle pause
}
