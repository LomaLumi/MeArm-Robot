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

// ====== Shared variables ======
volatile float posX = 0, posY = 50, posZ = 100;
volatile bool clawOpen = true;

// debounce
volatile unsigned long lastDebounce = 0;
const unsigned long debounceDelay = 50; // ms

// thresholds
const int thresholdLow  = 500;
const int thresholdHigh = 3500;
const float step = 2.0;

void setup() {
  Serial.begin(115200);
  arm.begin(basePin, shoulderPin, elbowPin, clawPin);
  pinMode(joy2SW, INPUT_PULLUP);
  arm.openClaw();

  analogReadResolution(12); // 12-bit ADC

  // --- Create FreeRTOS tasks ---
  xTaskCreatePinnedToCore(readJoystickTask, "ReadJoystick", 2048, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(moveArmTask, "MoveArm", 2048, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(serialTask, "SerialOut", 2048, NULL, 1, NULL, 1);
}

void loop() {
  // Nothing needed here
}

// ====== Tasks ======
void readJoystickTask(void *pvParameters) {
  int lastSw = HIGH;
  for (;;) {
    int x1 = analogRead(joy1X);
    int z1 = analogRead(joy1Y);
    int y2 = analogRead(joy2X);
    int sw2 = digitalRead(joy2SW);
    
    // Update XYZ
    noInterrupts(); // ป้องกันการแก้ไข posX/Y/Z ขณะ moveArmTask อ่าน
    posX += (x1 < thresholdLow) ? -step : (x1 > thresholdHigh) ? step : 0;
    posZ += (z1 < thresholdLow) ? -step : (z1 > thresholdHigh) ? step : 0;
    posY += (y2 < 200) ? -step : (y2 > 4000) ? step : 0;

    posX = constrain(posX, -70, 90);
    posY = constrain(posY, 20, 100);
    posZ = constrain(posZ, 50, 150);
    interrupts();

    // Claw button with debounce
    unsigned long now = millis();
    if (sw2 == LOW && lastSw == HIGH && (now - lastDebounce) > debounceDelay) {
      clawOpen ? arm.closeClaw() : arm.openClaw();
      clawOpen = !clawOpen;
      lastDebounce = now;
    }
    lastSw = sw2;

    vTaskDelay(pdMS_TO_TICKS(10)); // 100 Hz
  }
}

void moveArmTask(void *pvParameters) {
  for (;;) {
    noInterrupts();
    float x = posX;
    float y = posY;
    float z = posZ;
    interrupts();
    arm.moveToXYZ(x, y, z);
    vTaskDelay(pdMS_TO_TICKS(10)); // 100 Hz
  }
}

void serialTask(void *pvParameters) {
  for (;;) {
    noInterrupts();
    float x = posX;
    float y = posY;
    float z = posZ;
    bool claw = clawOpen;
    interrupts();
    Serial.printf("%.1f,%.1f,%.1f,%s\n", x, y, z, claw ? "Open" : "Closed");
    vTaskDelay(pdMS_TO_TICKS(50)); // 20 Hz
  }
}
