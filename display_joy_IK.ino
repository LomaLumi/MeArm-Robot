#include "MeArm.h"
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ====== Servo Pins ======
int basePin = 13;
int shoulderPin = 14;
int elbowPin = 26;
int clawPin = 27;

// ====== Joystick Pins ======
int joy1X = 32;   // X
int joy1Y = 15;   // Z
int joy2X = 34;   // Y
int joy2SW = 4;   // Claw button

// ====== MeArm ======
MeArm arm(128, 33, -pi / 4, pi / 4,       // Base
          135, 35, pi / 4, 3 * pi / 4, // Shoulder
          135, 45, pi / 4, -pi / 4,    // Elbow
          18, 5, PI/2, 0);            // Claw

// ====== OLED ======
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool clawOpen = true;
float posX = 0, posY = 100, posZ = 100;

// non-blocking timing
unsigned long lastSerial = 0;
const unsigned long serialInterval = 50; // ms
unsigned long lastDebounce = 0;
const unsigned long debounceDelay = 50;  // ms

void setup() {
  Serial.begin(115200);
  arm.begin(basePin, shoulderPin, elbowPin, clawPin);
  pinMode(joy2SW, INPUT_PULLUP);
  arm.openClaw();

  analogReadResolution(12);

  // OLED init
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  int x1 = analogRead(joy1X);
  int z1 = analogRead(joy1Y);
  int y2 = analogRead(joy2X);
  int sw2 = digitalRead(joy2SW);

  const int thresholdLow  = 500;
  const int thresholdHigh = 3500;
  const float step = 2.0;

  posX += (x1 < thresholdLow) ? -step : (x1 > thresholdHigh) ? step : 0;
  posZ += (z1 < thresholdLow) ? -step : (z1 > thresholdHigh) ? step : 0;
  posY += (y2 < thresholdLow) ? -step : (y2 > thresholdHigh) ? step : 0;

  posX = constrain(posX, -70, 120);
  posY = constrain(posY, 40, 120);
  posZ = constrain(posZ, 20, 150);

  arm.moveToXYZ(posX, posY, posZ);

  // Claw button with debounce
  static bool lastSw = HIGH;
  unsigned long now = millis();
  if (sw2 == LOW && lastSw == HIGH && (now - lastDebounce) > debounceDelay) {
    clawOpen ? arm.closeClaw() : arm.openClaw();
    clawOpen = !clawOpen;
    lastDebounce = now;
  }
  lastSw = sw2;

  // Serial print
  if (now - lastSerial >= serialInterval) {
    lastSerial = now;
    Serial.printf("%.1f,%.1f,%.1f,%s\n", posX, posY, posZ, clawOpen ? "Open" : "Closed");
  }

  // OLED display update
  display.clearDisplay();
  display.setCursor(0,0);
  display.printf("X: %.1f", posX);
  display.setCursor(0,10);
  display.printf("Y: %.1f", posY);
  display.setCursor(0,20);
  display.printf("Z: %.1f", posZ);
  display.setCursor(0,30);
  display.printf("Claw: %s", clawOpen ? "Open" : "Closed");
  display.display();
}
