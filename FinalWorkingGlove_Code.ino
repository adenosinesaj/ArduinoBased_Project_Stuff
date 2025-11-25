#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

// Replace with your Wi-Fi network
const char* ssid = "Home";
const char* password = "1amsherl0cked";

// Vehicle ESP32 IP and UDP port
const char* vehicleIP = "192.168.0.100";  // Change to your vehicle ESP32 IP
const int vehiclePort = 4210;

WiFiUDP udp;
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Joystick pins (analog)
#define JOY_X 34
#define JOY_Y 35

float tiltThreshold = 6.0;  // gesture sensitivity

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");

  // ADXL345 init
  if (!accel.begin()) {
    Serial.println("ADXL345 NOT FOUND!");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_2_G);
  Serial.println("Glove Ready");

  // Joystick pins
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
}

void loop() {
  // --- ADXL345 Gesture ---
  sensors_event_t event;
  accel.getEvent(&event);
  float xTilt = event.acceleration.x;
  float yTilt = event.acceleration.y;

  String cmdStr = "STOP";

  if (yTilt > tiltThreshold) cmdStr = "FORWARD";
  else if (yTilt < -tiltThreshold) cmdStr = "BACKWARD";
  else if (xTilt > tiltThreshold) cmdStr = "RIGHT";
  else if (xTilt < -tiltThreshold) cmdStr = "LEFT";

  // --- Joystick ---
  int joyX = analogRead(JOY_X); // 0 - 4095
  int joyY = analogRead(JOY_Y);

  // Normalize joystick (-100 to 100)
  int joyXVal = map(joyX, 0, 4095, -100, 100);
  int joyYVal = map(joyY, 0, 4095, -100, 100);

  // Prepare UDP packet as: CMD|joyX|joyY
  char packet[50];
  snprintf(packet, sizeof(packet), "%s|%d|%d", cmdStr.c_str(), joyXVal, joyYVal);

  // Send UDP
  udp.beginPacket(vehicleIP, vehiclePort);
  udp.write((uint8_t *)packet, strlen(packet));
  udp.endPacket();

  Serial.println(packet);
  delay(100);
}
