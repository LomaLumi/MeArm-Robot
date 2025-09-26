#include "MeArm.h"
#include <ESP32Servo.h>

// ====== Servo Pins ======
int basePin = 13;
int shoulderPin = 14;
int elbowPin = 26;
int clawPin = 27;

// ====== Joystick Pins ======
int joy1X = 32;   // X
int joy1Y = 34;   // Z
int joy2X = 15;   // Y
int joy2SW = 4;   // Claw button

// ====== MeArm ======
MeArm arm(33, 128, -PI/4, PI/4,       // Base
          35, 100, PI/4, 3*PI/4,     // Shoulder
          155, 90, PI/4, -PI/4,      // Elbow
          18, 5, PI/2, 0);           // Claw

bool clawOpen = true;
float posX = 0, posY = 50, posZ = 100;

unsigned long lastSerial = 0;
const unsigned long serialInterval = 50; // ms

void setup() {
  Serial.begin(115200);
  arm.begin(basePin, shoulderPin, elbowPin, clawPin);
  pinMode(joy2SW, INPUT_PULLUP);
  arm.openClaw();
}

void loop() {
  // อ่านค่า joystick
  int x1 = analogRead(joy1X);
  int z1 = analogRead(joy1Y);
  int y2 = analogRead(joy2X);
  int sw2 = digitalRead(joy2SW);

  const int thresholdLow  = 500;
  const int thresholdHigh = 3500;
  const float step = 2.0;

  // ปรับ XYZ ตาม joystick
  posX += (x1 < thresholdLow) ? -step : (x1 > thresholdHigh) ? step : 0;
  posZ += (z1 < thresholdLow) ? -step : (z1 > thresholdHigh) ? step : 0;
  posY += (y2 < 200) ? -step : (y2 > 4000) ? step : 0;

  // จำกัดขอบเขต
  posX = constrain(posX, -70, 90);
  posY = constrain(posY, 20, 100);
  posZ = constrain(posZ, 50, 150);

  // ขยับแขน
  arm.moveToXYZ(posX, posY, posZ);

  // ปุ่ม Claw แบบ non-blocking
  static bool lastSw = HIGH;
  if (sw2 == LOW && lastSw == HIGH) {
    if (clawOpen) arm.closeClaw();
    else arm.openClaw();
    clawOpen = !clawOpen;
    delay(50); // debounce
  }
  lastSw = sw2;

  // ส่ง Serial แบบไม่บล็อก
  unsigned long now = millis();
  if (now - lastSerial >= serialInterval) {
    lastSerial = now;
    Serial.printf("%.1f,%.1f,%.1f,%s\n", posX, posY, posZ, clawOpen ? "Open" : "Closed");
  }
}
