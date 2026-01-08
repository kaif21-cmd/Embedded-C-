#include <SPI.h>
#include <DMD32.h>

#include "fonts/SystemFont5x7.h"
#include "fonts/Droid_Sans_12.h"
#include "fonts/Droid_Sans_16.h"

// ---------------- DISPLAY SIZE ----------------
#define WIDTH  1
#define HEIGHT 1

DMD dmd(WIDTH, HEIGHT);

// ---------------- GLOBAL ----------------
unsigned long lastPageChange = 0;
int page = 0;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  dmd.begin();
  dmd.clearScreen(true);
  dmd.selectFont(SystemFont5x7);
}

// ---------------- CENTER TEXT FUNCTION ----------------
void drawCenterText(const char* text, int textWidth, int fontHeight) {
  int x = (32 - textWidth) / 2;
  int y = (16 - fontHeight) / 2;
  dmd.drawString(x, y, text, strlen(text), GRAPHICS_NORMAL);
}

// ---------------- PAGE 1 : BASIC TEXT ----------------
void pageText() {
  dmd.clearScreen(true);
  dmd.selectFont(SystemFont5x7);
  dmd.drawString(0, 0, "HELLO", 5, GRAPHICS_NORMAL);
}

// ---------------- PAGE 2 : CENTER TEXT ----------------
void pageCenter() {
  dmd.clearScreen(true);
  dmd.selectFont(SystemFont5x7);
  drawCenterText("OK", 10, 7);
}

// ---------------- PAGE 3 : BIG FONT ----------------
void pageBigFont() {
  dmd.clearScreen(true);
  dmd.selectFont(Droid_Sans_16);
  dmd.drawString(0, 0, "12", 2, GRAPHICS_NORMAL);
}

// ---------------- PAGE 4 : SHAPES ----------------
void pageShapes() {
  dmd.clearScreen(true);
  dmd.drawBox(0, 0, 31, 15, GRAPHICS_NORMAL);
  dmd.drawLine(0, 15, 31, 0, GRAPHICS_NORMAL);
  dmd.drawCircle(16, 8, 4, GRAPHICS_NORMAL);
}

// ---------------- PAGE 5 : PIXEL DEMO ----------------
void pagePixels() {
  dmd.clearScreen(true);
  for (int x = 0; x < 32; x++) {
    dmd.drawPixel(x, 8, GRAPHICS_NORMAL);
    delay(20);
  }
}

// ---------------- PAGE 6 : ANIMATION ----------------
void pageAnimation() {
  for (int x = 0; x < 32; x++) {
    dmd.clearScreen(true);
    dmd.drawCircle(x, 8, 2, GRAPHICS_NORMAL);
    delay(40);
  }
}

// ---------------- PAGE 7 : MARQUEE ----------------
void pageMarquee() {
  dmd.clearScreen(true);
  dmd.selectFont(SystemFont5x7);

  char msg[] = "WELCOME TO DMD ";
  dmd.drawMarquee(msg, strlen(msg), 32, 0);

  unsigned long start = millis();
  while (millis() - start < 4000) {
    if (dmd.stepMarquee(-1, 0)) {
      dmd.drawMarquee(msg, strlen(msg), 32, 0);
    }
    delay(40); // speed control
  }
}

// ---------------- LOOP ----------------
void loop() {
  if (millis() - lastPageChange > 3000) {
    lastPageChange = millis();
    page++;
    if (page > 6) page = 0;
  }

  switch (page) {
    case 0: pageText(); break;
    case 1: pageCenter(); break;
    case 2: pageBigFont(); break;
    case 3: pageShapes(); break;
    case 4: pagePixels(); break;
    case 5: pageAnimation(); break;
    case 6: pageMarquee(); break;
  }
}
