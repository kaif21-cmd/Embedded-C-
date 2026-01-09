#include <Arduino.h>
#include "LittleFS.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("===== LittleFS Demo =====");

  // 1️⃣ Mount LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("❌ LittleFS Mount Failed");
    return;
  }
  Serial.println("✅ LittleFS Mounted");

  // 2️⃣ File Write
  File file = LittleFS.open("/test.txt", "w");
  if (!file) {
    Serial.println("❌ File open failed for write");
    return;
  }

  file.println("Hello from ESP32 LittleFS");
  file.println("Battery = 78%");
  file.close();
  Serial.println("✅ File written");

  // 3️⃣ File Read
  file = LittleFS.open("/test.txt", "r");
  if (!file) {
    Serial.println("❌ File open failed for read");
    return;
  }

  Serial.println("📄 File Content:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void loop() {
}



