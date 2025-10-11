/*
 * ESP32 Wi-Fi Controlled Robot Car with Arm and Gripper
 * * This code turns your ESP32 into a web server that hosts a control panel for your robot.
 * You can access this panel from a phone, tablet, or computer on the same Wi-Fi network.
 * * HARDWARE REQUIRED:
 * 1. ESP32 Development Board
 * 2. Robot Chassis with 2 DC motors (for left and right wheels)
 * 3. Robotic Arm with 1 DC motor (for up/down movement)
 * 4. Gripper with 1 DC motor (for open/close)
 * 5. Motor Driver(s) - Two L298N modules or a single driver that can control 4 DC motors.
 * 6. Power source for motors (e.g., Li-ion batteries)
 * 7. Power source for ESP32 (can be the same, but use a voltage regulator for the ESP32)
 *
 * HOW TO USE:
 * 1. Install the "ESPAsyncWebServer" and "AsyncTCP" libraries in your Arduino IDE.
 * - Go to Sketch > Include Library > Manage Libraries...
 * - Search for "ESPAsyncWebServer" and install it.
 * - Search for "AsyncTCP" and install it.
 * 2. Update the `ssid` and `password` variables below with your Wi-Fi network credentials.
 * 3. Connect the motor driver pins to the ESP32 GPIO pins as defined in the "Pin Definitions" section.
 * 4. Upload the code to your ESP32.
 * 5. Open the Serial Monitor (Tools > Serial Monitor) and set the baud rate to 115200.
 * 6. The ESP32 will attempt to connect to your Wi-Fi and print its IP address.
 * 7. Type that IP address into a web browser on a device connected to the same Wi-Fi network.
 * 8. The control interface should appear, and you can now control your robot!
 */

// ------------------- Libraries -------------------
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// ------------------- Wi-Fi Credentials -------------------
// Replace with your network credentials
const char* ssid = "Devansh-jio";
const char* password = "Devansh@#$2007";

// ------------------- Pin Definitions -------------------
// Connect these ESP32 pins to your motor driver(s).
// This assumes a driver like the L298N.

// Car Movement Motors
const int LEFT_MOTOR_FORWARD = 26;  // IN1 on L298N for left motor
const int LEFT_MOTOR_REVERSE = 25;  // IN2 on L298N for left motor
const int RIGHT_MOTOR_FORWARD = 14; // IN3 on L298N for right motor
const int RIGHT_MOTOR_REVERSE = 27; // IN4 on L298N for right motor

// Arm Lift Motor
const int ARM_MOTOR_UP = 12;   // Another motor driver channel
const int ARM_MOTOR_DOWN = 13; // Another motor driver channel

// Gripper Open/Close Motor
const int GRIPPER_MOTOR_OPEN = 15;  // Another motor driver channel
const int GRIPPER_MOTOR_CLOSE = 2; // Another motor driver channel

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// ------------------- Web Page HTML -------------------
// The complete HTML, CSS, and JavaScript for the control interface.

// ------------------- Motor Control Functions -------------------
void moveForward() {
    Serial.println("Moving Forward");
    digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(RIGHT_MOTOR_REVERSE, LOW);
}

void moveReverse() {
    Serial.println("Moving Reverse");
    digitalWrite(LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(LEFT_MOTOR_REVERSE, HIGH);
    digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_REVERSE, HIGH);
}

void turnLeft() {
    Serial.println("Turning Left");
    digitalWrite(LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(LEFT_MOTOR_REVERSE, HIGH);
    digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(RIGHT_MOTOR_REVERSE, LOW);
}

void turnRight() {
    Serial.println("Turning Right");
    digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_REVERSE, HIGH);
}

void stopCar() {
    Serial.println("Stopping Car");
    digitalWrite(LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_REVERSE, LOW);
}

void armUp() {
    Serial.println("Arm Up");
    digitalWrite(ARM_MOTOR_UP, HIGH);
    digitalWrite(ARM_MOTOR_DOWN, LOW);
}

void armDown() {
    Serial.println("Arm Down");
    digitalWrite(ARM_MOTOR_UP, LOW);
    digitalWrite(ARM_MOTOR_DOWN, HIGH);
}

void stopArm() {
    Serial.println("Stop Arm");
    digitalWrite(ARM_MOTOR_UP, LOW);
    digitalWrite(ARM_MOTOR_DOWN, LOW);
}

void gripperOpen() {
    Serial.println("Gripper Open");
    digitalWrite(GRIPPER_MOTOR_OPEN, HIGH);
    digitalWrite(GRIPPER_MOTOR_CLOSE, LOW);
}

void gripperClose() {
    Serial.println("Gripper Close");
    digitalWrite(GRIPPER_MOTOR_OPEN, LOW);
    digitalWrite(GRIPPER_MOTOR_CLOSE, HIGH);
}

void stopGripper() {
    Serial.println("Stop Gripper");
    digitalWrite(GRIPPER_MOTOR_OPEN, LOW);
    digitalWrite(GRIPPER_MOTOR_CLOSE, LOW);
}

void addCorsHeaders(AsyncWebServerResponse *response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ------------------- Setup Function -------------------
void setup() {
    Serial.begin(115200);

    // Set all motor control pins to outputs
    pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(LEFT_MOTOR_REVERSE, OUTPUT);
    pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_REVERSE, OUTPUT);
    pinMode(ARM_MOTOR_UP, OUTPUT);
    pinMode(ARM_MOTOR_DOWN, OUTPUT);
    pinMode(GRIPPER_MOTOR_OPEN, OUTPUT);
    pinMode(GRIPPER_MOTOR_CLOSE, OUTPUT);

    // Stop all motors initially
    stopCar();
    stopArm();
    stopGripper();

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // --- Web Server Request Handlers ---

    // Serve the main web page
    //server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //    request->send_P(200, "text/html", index_html);
    //});

    // Car Movement handlers
    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){
        moveForward();
        request->send(200, "text/plain", "OK");
    });
    server.on("/reverse", HTTP_GET, [](AsyncWebServerRequest *request){
        moveReverse();
        request->send(200, "text/plain", "OK");
    });
    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
        turnLeft();
        request->send(200, "text/plain", "OK");
    });
    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
        turnRight();
        request->send(200, "text/plain", "OK");
    });
    server.on("/stop-car", HTTP_GET, [](AsyncWebServerRequest *request){
        stopCar();
        request->send(200, "text/plain", "OK");
    });

    // Arm handlers
    server.on("/arm-up", HTTP_GET, [](AsyncWebServerRequest *request){
        armUp();
        request->send(200, "text/plain", "OK");
    });
    server.on("/arm-down", HTTP_GET, [](AsyncWebServerRequest *request){
        armDown();
        request->send(200, "text/plain", "OK");
    });
    server.on("/stop-arm", HTTP_GET, [](AsyncWebServerRequest *request){
        stopArm();
        request->send(200, "text/plain", "OK");
    });

    // Gripper handlers
    server.on("/gripper-open", HTTP_GET, [](AsyncWebServerRequest *request){
        gripperOpen();
        request->send(200, "text/plain", "OK");
    });
    server.on("/gripper-close", HTTP_GET, [](AsyncWebServerRequest *request){
        gripperClose();
        request->send(200, "text/plain", "OK");
    });
    server.on("/stop-gripper", HTTP_GET, [](AsyncWebServerRequest *request){
        stopGripper();
        request->send(200, "text/plain", "OK");
    });

    server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            AsyncWebServerResponse *response = request->beginResponse(204);
            addCorsHeaders(response);
            request->send(response);
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });

    // Start server
    server.begin();
    Serial.println("HTTP server started");
}

// ------------------- Loop Function -------------------
void loop() {
    // The AsyncWebServer handles client requests in the background.
    // No need to add code here for the web server.
}