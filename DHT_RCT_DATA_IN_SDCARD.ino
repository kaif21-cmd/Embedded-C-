#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <DHT.h>

#define SD_CS 53       // CORRECT CS PIN FOR ARDUINO MEGA
#define DHTPIN 2
#define DHTTYPE DHT11

RTC_DS3231 rtc;
DHT dht(DHTPIN, DHTTYPE);
File dataFile;

bool sdAvailable = false;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("=== LOGGER START ===");

  // ---------- VERY IMPORTANT FOR MEGA ----------
  pinMode(53, OUTPUT);  // Explicitly set SS pin as OUTPUT (required on Mega)

  // ---------- INIT SD CARD ----------
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("SD INIT FAIL ❌");
    sdAvailable = false;
  } else {
    Serial.println("SD INIT OK ✅");
    sdAvailable = true;

    // Create CSV file with header if it doesn't exist
    if (!SD.exists("log.csv")) {
      dataFile = SD.open("log.csv", FILE_WRITE);
      if (dataFile) {
        dataFile.println("Date,Time,Temperature(C),Humidity(%)");
        dataFile.close();
        Serial.println("Header created in log.csv ✅");
      } else {
        Serial.println("Failed to create header ❌");
      }
    } else {
      Serial.println("log.csv already exists - header skipped");
    }
  }

  // ---------- INIT RTC ----------
  if (!rtc.begin()) {
    Serial.println("RTC NOT FOUND ❌");
    Serial.flush();
    while (1);  // Halt if RTC missing
  } else {
    Serial.println("RTC OK ✅");
    // Uncomment the line below to set RTC to compile time (only once!)
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // ---------- INIT DHT ----------
  dht.begin();
  Serial.println("DHT11 OK ✅");
  //Serial.println("Logging started - Data will be saved every second\n");
}

void loop() {
  DateTime now = rtc.now();

  // Read temperature and humidity
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Check if readings are valid
  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT READ ERROR ❌");
    delay(2000);
    return;
  }

  // Format date and time as DD/MM/YYYY and HH:MM:SS
  String dateStr = "";
  if (now.day() < 10) dateStr += "0";
  dateStr += String(now.day()) + "/";
  if (now.month() < 10) dateStr += "0";
  dateStr += String(now.month()) + "/" + String(now.year());

  String timeStr = "";
  if (now.hour() < 10) timeStr += "0";
  timeStr += String(now.hour()) + ":";
  if (now.minute() < 10) timeStr += "0";
  timeStr += String(now.minute()) + ":";
  if (now.second() < 10) timeStr += "0";
  timeStr += String(now.second());

  // Create data string for CSV
  String dataString = dateStr + "," + timeStr + "," + String(temp, 2) + "," + String(hum, 2);

  // Print to Serial Monitor
  Serial.print("Data: ");
  Serial.println(dataString);

  // Write to SD card if available
  if (sdAvailable) {
    dataFile = SD.open("log.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      Serial.println("DATA SAVED TO SD ✅");
    } else {
      //Serial.println("");
      sdAvailable = false;  // Disable further attempts
    }
  } else {
    //Serial.println("SD not available - Data only shown on Serial");
  }

  delay(2000);  // Log every 1 second (adjust as needed)
}
