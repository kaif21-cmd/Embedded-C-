#include <math.h> // sin , accos , ^ calculation 

// ---- Your function ----
float flowArea(float d0, float yd0) {   
  float y = yd0 * d0;
  float theta = 2 * acos(1 - (2 * y / d0));
  float a = (d0 * d0 / 8.0) * (theta - sin(theta));
  return a;
}

float d0, yd0, area;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20000);   
  Serial.println("FLOW AREA CALCULATOR");
}

void loop() {

  // -------- Pipe Diameter --------
  Serial.println("\nEnter Pipe Diameter d0 (m):");
  while (!Serial.available());
  d0 = Serial.parseFloat();
  Serial.readStringUntil('\n');   // 

  Serial.print("d0 = ");
  Serial.println(d0);

  // -------- Relative Depth --------
  Serial.println("Enter Relative Flow Depth y/d0:");
  while (!Serial.available());
  yd0 = Serial.parseFloat();
  Serial.readStringUntil('\n');   // 

  Serial.print("y/d0 = ");
  Serial.println(yd0);

  // -------- Validation --------
  if (yd0 < 0 || yd0 > 1) {
    Serial.println("ERROR: y/d0 must be between 0 and 1");
    return;
  }

  // -------- Calculation --------
  area = flowArea(d0, yd0);

  Serial.println("----------------------");
  Serial.print("FLOW AREA (a) = ");
  Serial.print(area, 4);
  Serial.println(" m^2");
  Serial.println("----------------------");

  delay(3000);
}
.
