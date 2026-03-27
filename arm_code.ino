#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

// Create servo objects
Servo baseServo;
Servo shoulderServo;
Servo wristServo;
Servo gripperServo;

// Structure for incoming data
typedef struct struct_message {
  int base;
  int shoulder;
  int wrist;
  int gripper;
} struct_message;

struct_message incomingData;

// Current positions (for smoothing)
int basePos = 90;
int shoulderPos = 90;
int wristPos = 90;
int gripperPos = 90;

// Target positions
int baseTarget = 90;
int shoulderTarget = 90;
int wristTarget = 90;
int gripperTarget = 90;

// Limit function (safety)
int constrainAngle(int val, int minA, int maxA) {
  if (val < minA) return minA;
  if (val > maxA) return maxA;
  return val;
}

// Callback when data received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

  // Apply limits (adjust based on your arm)
  baseTarget     = constrainAngle(incomingData.base, 10, 170);
  shoulderTarget = constrainAngle(incomingData.shoulder, 20, 160);
  wristTarget    = constrainAngle(incomingData.wrist, 0, 180);
  gripperTarget  = constrainAngle(incomingData.gripper, 30, 110);
}

// Smooth movement function
void smoothMove(Servo &servo, int &current, int target) {
  if (current < target) current++;
  else if (current > target) current--;

  servo.write(current);
}

void setup() {
  Serial.begin(115200);

  // Attach servos
  baseServo.attach(18);
  shoulderServo.attach(19);
  wristServo.attach(21);
  gripperServo.attach(22);

  // Initial positions
  baseServo.write(basePos);
  shoulderServo.write(shoulderPos);
  wristServo.write(wristPos);
  gripperServo.write(gripperPos);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

  // Smoothly move each servo
  smoothMove(baseServo, basePos, baseTarget);
  smoothMove(shoulderServo, shoulderPos, shoulderTarget);
  smoothMove(wristServo, wristPos, wristTarget);
  smoothMove(gripperServo, gripperPos, gripperTarget);

  delay(10); // controls speed (lower = faster)
}