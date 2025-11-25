#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

// Wi-Fi
const char* ssid = "Home";
const char* password = "1amsherl0cked";

WiFiUDP udp;
const int localPort = 4210;

// Motor pins
#define IN1 14
#define IN2 25
#define IN3 5
#define IN4 17

char command[50];

// Servo
#define SERVO_PIN 21
Servo myServo;

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Servo
  myServo.attach(SERVO_PIN);
  myServo.write(90); // neutral

  // Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");

  udp.begin(localPort);
  Serial.println("Vehicle Ready");
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(command, sizeof(command) - 1);
    if (len > 0) command[len] = 0; // null terminate
    Serial.print("Received: "); Serial.println(command);

    // Split command by '|'
    char *cmd = strtok(command, "|");
    char *joyXStr = strtok(NULL, "|");
    char *joyYStr = strtok(NULL, "|");

    int joyXVal = joyXStr ? atoi(joyXStr) : 0;
    int joyYVal = joyYStr ? atoi(joyYStr) : 0;

    // Motor Control based on gesture
    if (strcmp(command, "FORWARD") == 0) {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);

        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
    }
    else if (strcmp(command, "BACKWARD") == 0) {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);

        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
    }
    else if (strcmp(command, "LEFT") == 0) {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);

        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
    }
    else if (strcmp(command, "RIGHT") == 0) {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);

        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
    }
    else { // STOP
      digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    }

    // Servo Control from joystick X (-100 to 100)
    int servoAngle = map(joyXVal, -100, 100, 0, 180);
    myServo.write(servoAngle);
  }
}
