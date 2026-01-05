#define S_RXD 13
#define S_TXD 32

#include <SCServo.h>
#include <EEPROM.h>

// =================================================
// SERVO OBJECTS
// =================================================
SCSCL servoH;   // Servo 1 - Horizontal
SCSCL servoV;   // Servo 2 - Vertical

// =================================================
// BUTTON PINS
// =================================================
#define BTN_A 27
#define BTN_B 26
#define BTN_C 25
#define BTN_D 4

// EEPROM

#define EEPROM_SIZE 64

// =================================================
// // ANGLE STORAGE (RAW VALUES)
// // =================================================
// int posH[4] = {100, 540, 100, 540};     // Default raw
// int posV[4] = {300, 300, 800, 800};

// HOME POSITION
#define HOME_H 100
#define HOME_V 560

// =================================================
// MOTION PARAMS
// =================================================
#define MOVE_SPEED 1000
#define STEP_DELAY 1000
#define HOLD_TIME 2000


// =================================================
// UTILITY FUNCTIONS
// =================================================
int degreeToRaw(int deg) {
  deg = constrain(deg, 0, 180);
  return map(deg, 0, 180, 0, 1000);
}

// =================================================
// EEPROM FUNCTIONS
// =================================================
void saveAnglesToEEPROM() {
  int addr = 0;

  for (int i = 0; i < 4; i++) {
    EEPROM.put(addr, posH[i]); addr += 4;
  }
  for (int i = 0; i < 4; i++) {
    EEPROM.put(addr, posV[i]); addr += 4;
  }


  EEPROM.commit();
}

void loadAnglesFromEEPROM() {
  int addr = 0;

  for (int i = 0; i < 4; i++) {
    EEPROM.get(addr, posH[i]); addr += 4;
  }
  for (int i = 0; i < 4; i++) {
    EEPROM.get(addr, posV[i]); addr += 4;
  }

  // EEPROM empty safety check
  if (posH[0] < 0 || posH[0] > 1000) {
    posH[0] = 100;  posV[0] = 75;
    posH[1] = 540;  posV[1] = 75;
    posH[2] = 100;  posV[2] = 1000;
    posH[3] = 540;  posV[3] = 1000;
    saveAnglesToEEPROM();
  }
}

// =================================================
// SERVO SEQUENCE
// =================================================
void moveSequential(int index) {

  servoH.WritePos(1, posH[index], 0, MOVE_SPEED);
  delay(STEP_DELAY);

  servoV.WritePos(1, posV[index], 0, MOVE_SPEED);
  delay(HOLD_TIME);

  servoV.WritePos(1, HOME_V, 0, MOVE_SPEED);
  delay(STEP_DELAY);

  servoH.WritePos(1, HOME_H, 0, MOVE_SPEED);
}

// =================================================
// SETUP
// =================================================
void setup() {
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  loadAnglesFromEEPROM();

  Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);
  Serial2.begin(1000000, SERIAL_8N1, 14, 33);

  servoH.pSerial = &Serial1;
  servoV.pSerial = &Serial2;

  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_C, INPUT_PULLUP);
  pinMode(BTN_D, INPUT_PULLUP);

  delay(1000);

  servoH.WritePos(1, HOME_H, 0, MOVE_SPEED);
  servoV.WritePos(1, HOME_V, 0, MOVE_SPEED);

  Serial.println("Smart Dustbin Ready");
}

// =================================================
// LOOP
// =================================================
void loop() {

  // ---------- BUTTON CONTROL ----------
  if (digitalRead(BTN_A) == LOW) { moveSequential(0); delay(300); }
  else if (digitalRead(BTN_B) == LOW) { moveSequential(1); delay(300); }
  else if (digitalRead(BTN_C) == LOW) { moveSequential(2); delay(300); }
  else if (digitalRead(BTN_D) == LOW) { moveSequential(3); delay(300); }

  // ---------- SERIAL CALIBRATION ----------
  if (Serial.available()) {

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.length() < 5) return;

    char bin = cmd.charAt(0);   // A/B/C/D
    int colon = cmd.indexOf(':');
    int comma = cmd.indexOf(',');

    if (colon == -1 || comma == -1) return;
    
    int degH = cmd.substring(colon + 1, comma).toInt();
    int degV = cmd.substring(comma + 1).toInt();

    int index = bin - 'A';
    if (index < 0 || index > 3) return;

    posH[index] = degreeToRaw(degH);
    posV[index] = degreeToRaw(degV);

    servoH.WritePos(1, posH[index], 0, MOVE_SPEED);
    servoV.WritePos(1, posV[index], 0, MOVE_SPEED);

    saveAnglesToEEPROM();

    Serial.print("Saved ");
    Serial.print(bin);
    Serial.print(" -> H:");
     Serial.print(degH);
    Serial.print(" V:");
    Serial.println(degV);
  }
}

