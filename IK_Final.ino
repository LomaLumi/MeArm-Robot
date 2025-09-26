#include "MeArm.h"
#include <ESP32Servo.h>

// ====== Servo Pins ======
int basePin = 13;
int shoulderPin = 14;
int elbowPin = 26;
int clawPin = 27;

// ====== Joystick 1 (X,Z) ======
int joy1X = 32;   // X
int joy1Y = 34;   // Z

// ====== Joystick 2 (Y, Claw) ======
int joy2X = 15;   // Y
int joy2SW = 4;   // ปุ่มกด Claw

// ====== MeArm ======
MeArm arm(33, 128, -PI/4, PI/4,       // Base
          35, 100, PI/4, 3*PI/4,     // Shoulder
          155, 90, PI/4, -PI/4,      // Elbow
          18, 5, PI/2, 0);           // Claw

// สถานะ claw
bool clawOpen = true;

// ค่าตำแหน่ง XYZ เริ่มต้น
float posX = 0;
float posY = 50;
float posZ = 100;

void setup() {
  Serial.begin(115200);
  arm.begin(basePin, shoulderPin, elbowPin, clawPin);
  pinMode(joy2SW, INPUT_PULLUP);

  arm.openClaw(); // เริ่มต้นเปิด claw
}

void loop() {
  // อ่านค่า joystick
  int x1 = analogRead(joy1X); 
  int z1 = analogRead(joy1Y);
  int y2 = analogRead(joy2X); 
  int sw2 = digitalRead(joy2SW);
  
  int thresholdLow  = 500;
  int thresholdHigh = 3500;
  float step = 2.0;

  // === X จาก Joystick 1 ===
  if (x1 < thresholdLow) posX -= step;
  else if (x1 > thresholdHigh) posX += step;

  // === Z จาก Joystick 1 ===
  if (z1 < thresholdLow) posZ -= step;
  else if (z1 > thresholdHigh) posZ += step;

  // === Y จาก Joystick 2 ===
  if (y2 < 200) posY -= step;
  else if (y2 > 4000) posY += step;

  // จำกัดช่วง
  posX = constrain(posX, -70, 90);
  posY = constrain(posY, 20, 100);
  posZ = constrain(posZ, 50, 150);

  // ขยับแขน
  arm.moveToXYZ(posX, posY, posZ);

  // === ปุ่ม Claw ===
  static bool lastSw = HIGH;
  if (sw2 == LOW && lastSw == HIGH) {
    if (clawOpen) arm.closeClaw();
    else arm.openClaw();
    clawOpen = !clawOpen;
    delay(50);
  }
  lastSw = sw2;

  // === ส่งข้อมูลออก Serial ===
  String clawState = clawOpen ? "Open" : "Closed";
  Serial.printf("%.1f,%.1f,%.1f,%s\n", posX, posY, posZ, clawState.c_str());

  delay(50); // ส่งทุก 0.5 วินาที
}
