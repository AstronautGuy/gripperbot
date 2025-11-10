/*
 * ESP32 Wi-Fi Controlled 4-Wheel Robot Car with Arm and Gripper
 * Optimized for Non-Blocking Control + Debounced Commands + Safety Timeout
 */

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>

// ------------------- Wi-Fi Configuration -------------------
struct WiFiNetwork {
  const char *ssid;
  const char *password;
};

WiFiNetwork knownNetworks[] = {
  {"Hotspot", "12345678"},
  {"STUDENT", ""},
  {"WorkshopNet", "Password12345"}
};

const char *ap_ssid = "RobotCar_AP";
const char *ap_password = "password";

// ------------------- Motor Pins -------------------
const int RIGHT_MOTOR_FORWARD = 22;
const int RIGHT_MOTOR_REVERSE = 23;
const int LEFT_MOTOR_FORWARD = 25;
const int LEFT_MOTOR_REVERSE = 26;

// ------------------- Servo Pins -------------------
const int ARM_SERVO_LEFT_PIN = 12;
const int ARM_SERVO_RIGHT_PIN = 14;
const int GRIPPER_SERVO_PIN = 13;

// ------------------- Servo Objects -------------------
Servo armServoLeft;
Servo armServoRight;
Servo gripperServo;

int armPos = 90;
int gripperPos = 90;
bool armMirrored = true;

// ------------------- State Variables -------------------
bool armUpActive = false;
bool armDownActive = false;
bool gripperOpenActive = false;
bool gripperCloseActive = false;

unsigned long lastServoUpdate = 0;
unsigned long lastCmdTime = 0;
unsigned long lastActionTime = 0;
String lastCmd = "";

const int servoInterval = 15;
const unsigned long cmdDebounce = 200;
const unsigned long safetyTimeout = 3000;

// ------------------- Server -------------------
AsyncWebServer server(80);

// ------------------- Helper: Debounce Commands -------------------
bool shouldProcess(const String &cmd) {
  unsigned long now = millis();
  if (cmd == lastCmd && now - lastCmdTime < cmdDebounce) return false;
  lastCmd = cmd;
  lastCmdTime = now;
  lastActionTime = now;
  return true;
}

// ------------------- Drive Functions -------------------
void stopCar() {
  digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
  digitalWrite(RIGHT_MOTOR_REVERSE, LOW);
  digitalWrite(LEFT_MOTOR_FORWARD, LOW);
  digitalWrite(LEFT_MOTOR_REVERSE, LOW);
  Serial.println("🛑 Stop Car");
}

void moveForward() {
  if (!shouldProcess("forward")) return;
  digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_REVERSE, LOW);
  digitalWrite(LEFT_MOTOR_FORWARD, LOW);
  digitalWrite(LEFT_MOTOR_REVERSE, HIGH);
  Serial.println("⬆️ Forward");
}

void moveReverse() {
  if (!shouldProcess("reverse")) return;
  digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
  digitalWrite(RIGHT_MOTOR_REVERSE, HIGH);
  digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
  digitalWrite(LEFT_MOTOR_REVERSE, LOW);
  Serial.println("⬇️ Reverse");
}

void turnLeft() {
  if (!shouldProcess("left")) return;
  digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
  digitalWrite(RIGHT_MOTOR_REVERSE, HIGH);
  digitalWrite(LEFT_MOTOR_FORWARD, LOW);
  digitalWrite(LEFT_MOTOR_REVERSE, HIGH);
  Serial.println("↩️ Left");
}

void turnRight() {
  if (!shouldProcess("right")) return;
  digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_REVERSE, LOW);
  digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
  digitalWrite(LEFT_MOTOR_REVERSE, LOW);
  Serial.println("↪️ Right");
}

// ------------------- Arm & Gripper -------------------
void updateArmServos() {
  int rightAngle = armMirrored ? (180 - armPos) : armPos;
  armServoLeft.write(armPos);
  armServoRight.write(rightAngle);
}

void updateGripper() {
  gripperServo.write(gripperPos);
}

// ------------------- Wi-Fi -------------------
void connectToWiFi() {
  Serial.println("🔍 Connecting to available Wi-Fi...");

  int numNetworks = sizeof(knownNetworks) / sizeof(knownNetworks[0]);
  for (int i = 0; i < numNetworks; i++) {
    Serial.printf("\nTrying: %s\n", knownNetworks[i].ssid);
    WiFi.begin(knownNetworks[i].ssid, knownNetworks[i].password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✅ Connected!");
      Serial.printf("Network: %s\n", knownNetworks[i].ssid);
      Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
      return;
    }
  }

  Serial.println("\n⚠️ No known Wi-Fi found. Starting AP...");
  WiFi.softAP(ap_ssid, ap_password);
  Serial.printf("AP SSID: %s | IP: %s\n", ap_ssid, WiFi.softAPIP().toString().c_str());
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);

  pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
  pinMode(RIGHT_MOTOR_REVERSE, OUTPUT);
  pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
  pinMode(LEFT_MOTOR_REVERSE, OUTPUT);

  armServoLeft.attach(ARM_SERVO_LEFT_PIN);
  armServoRight.attach(ARM_SERVO_RIGHT_PIN);
  gripperServo.attach(GRIPPER_SERVO_PIN);

  updateArmServos();
  updateGripper();

  connectToWiFi();

  // Drive routes
  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *r){ moveForward(); r->send(200, "text/plain", "OK"); });
  server.on("/reverse", HTTP_GET, [](AsyncWebServerRequest *r){ moveReverse(); r->send(200, "text/plain", "OK"); });
  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *r){ turnLeft(); r->send(200, "text/plain", "OK"); });
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *r){ turnRight(); r->send(200, "text/plain", "OK"); });
  server.on("/stop-car", HTTP_GET, [](AsyncWebServerRequest *r){ stopCar(); r->send(200, "text/plain", "OK"); });

  // Arm control
  server.on("/arm-up", HTTP_GET, [](AsyncWebServerRequest *r){ armUpActive = true; armDownActive = false; r->send(200, "text/plain", "Hold-Up"); });
  server.on("/arm-down", HTTP_GET, [](AsyncWebServerRequest *r){ armDownActive = true; armUpActive = false; r->send(200, "text/plain", "Hold-Down"); });
  server.on("/stop-arm", HTTP_GET, [](AsyncWebServerRequest *r){ armUpActive = armDownActive = false; r->send(200, "text/plain", "Arm-Stopped"); });

  // Gripper control
  server.on("/gripper-open", HTTP_GET, [](AsyncWebServerRequest *r){ gripperOpenActive = true; gripperCloseActive = false; r->send(200, "text/plain", "Hold-Open"); });
  server.on("/gripper-close", HTTP_GET, [](AsyncWebServerRequest *r){ gripperCloseActive = true; gripperOpenActive = false; r->send(200, "text/plain", "Hold-Close"); });
  server.on("/stop-gripper", HTTP_GET, [](AsyncWebServerRequest *r){ gripperOpenActive = gripperCloseActive = false; r->send(200, "text/plain", "Gripper-Stopped"); });

  server.begin();
  Serial.println("🚀 HTTP server started");
}

// ------------------- Main Loop -------------------
void loop() {
  unsigned long now = millis();

  // Servo updates (non-blocking)
  if (now - lastServoUpdate >= servoInterval) {
    lastServoUpdate = now;

    if (armUpActive && armPos < 180) armPos++;
    else if (armDownActive && armPos > 0) armPos--;

    if (gripperOpenActive && gripperPos < 180) gripperPos++;
    else if (gripperCloseActive && gripperPos > 0) gripperPos--;

    updateArmServos();
    updateGripper();
  }

  // Safety stop
  if (now - lastActionTime > safetyTimeout) {
    stopCar();
  }
}