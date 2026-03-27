#include <WiFi.h>
#include <esp_now.h>

typedef struct struct_message {
  int base;
  int shoulder;
  int wrist;
  int gripper;
} struct_message;

struct_message data;

uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0xXX, 0xXX, 0xXX};

esp_now_peer_info_t peerInfo;

// Button variables
int buttonPin = 25;
bool gripperState = false;

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Error");
    return;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);
}

void loop() {

  // Read joystick
  int x = analogRead(34);
  int y = analogRead(35);

  // Read potentiometer
  int pot = analogRead(32);

  // Map to servo angles
  data.base = map(x, 0, 4095, 0, 180);
  data.shoulder = map(y, 0, 4095, 0, 180);
  data.wrist = map(pot, 0, 4095, 0, 180);

  // Button control (toggle gripper)
  if (digitalRead(buttonPin) == LOW) {
    delay(100); // debounce
    gripperState = !gripperState;
  }

  if (gripperState)
    data.gripper = 40;   // CLOSED
  else
    data.gripper = 100;  // OPEN

  // Send data
  esp_now_send(broadcastAddress, (uint8_t *) &data, sizeof(data));

  delay(50);
}
