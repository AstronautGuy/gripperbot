/*
 * ESP32 Wi-Fi Controlled 4-Wheel Robot Car with Arm and Gripper
 * * MODIFIED VERSION (4-Wheel Drive):
 * - Controls four independent drive motors.
 * - Adds new movement commands for strafing and diagonal motion (for Mecanum wheels).
 * - Attempts to connect to a list of known Wi-Fi networks.
 * - Uses a static IP address for predictable access.
 * - Creates its own Wi-Fi Access Point (AP) as a fallback if no known networks are found.
 *
 * HOW TO USE:
 * 1. Install "ESPAsyncWebServer" and "AsyncTCP" libraries.
 * 2. Update the `knownNetworks` array with your Wi-Fi credentials.
 * 3. Update the `staticIP`, `gateway`, and `subnet` to match YOUR network configuration.
 * 4. (Optional) Change the `ap_ssid` and `ap_password` for the fallback hotspot.
 * 5. Connect your four motor driver inputs to the GPIO pins defined below.
 * 6. Upload the code to your ESP32.
 * 7. Open the Serial Monitor at 115200 baud to see the IP address.
 * 8. Control the robot by sending GET requests to the new endpoints (e.g., http://<IP>/strafe-left).
 */

// ------------------- Libraries -------------------
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// ------------------- Wi-Fi Configuration -------------------

// --- Define a structure to hold network credentials ---
struct WiFiNetwork {
    const char* ssid;
    const char* password;
};

// --- List of known Wi-Fi networks to try ---
// Add your networks here. The ESP32 will try them in order.
WiFiNetwork knownNetworks[] = {
    {"Devansh-jio", "Devansh@#$2007"},
    {"MyHomeWiFi", "MyHomePassword"},
    {"WorkshopNet", "Password12345"}
};

// --- Static IP Configuration ---
// Set this to an IP address that is available on your network.
IPAddress staticIP(192, 168, 29, 184);
// Your router's IP address
IPAddress gateway(192, 168, 29, 1);
// Subnet mask (usually this is correct)
IPAddress subnet(255, 255, 255, 0);

// --- Access Point (Hotspot) credentials ---
// This is used as a fallback if no known networks are found.
const char* ap_ssid = "RobotCar_AP";
const char* ap_password = "password";


// ------------------- Pin Definitions -------------------
// --- MODIFIED: Pin definitions for 4 independent motors ---
// Front Left Motor
const int FRONT_LEFT_MOTOR_FORWARD = 22;
const int FRONT_LEFT_MOTOR_REVERSE = 23;
// Front Right Motor
const int FRONT_RIGHT_MOTOR_FORWARD = 15;
const int FRONT_RIGHT_MOTOR_REVERSE = 2;
// Rear Left Motor
const int REAR_LEFT_MOTOR_FORWARD = 26;
const int REAR_LEFT_MOTOR_REVERSE = 25;
// Rear Right Motor
const int REAR_RIGHT_MOTOR_FORWARD = 14;
const int REAR_RIGHT_MOTOR_REVERSE = 27;

// Arm and Gripper pins remain the same
const int ARM_MOTOR_UP = 12;//red
const int ARM_MOTOR_DOWN = 13;//brown
const int GRIPPER_MOTOR_OPEN = 18;//check
const int GRIPPER_MOTOR_CLOSE = 19;//chechk

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// ------------------- Motor Control Functions -------------------

// --- MODIFIED: Updated car movement functions for 4WD ---

void stopCar() {
    Serial.println("Stopping Car");
    // Stop all 4 drive motors
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, LOW);
}

void moveForward() {
    Serial.println("Moving Forward");
    // Left side forward
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, LOW);
    // Right side forward
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, LOW);
}

void moveReverse() {
    Serial.println("Moving Reverse");
    // Left side reverse
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, HIGH);
    // Right side reverse
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, HIGH);
}

void turnLeft() {
    Serial.println("Turning Left (Pivot)");
    // Left side reverse
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, HIGH);
    // Right side forward
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, LOW);
}

void turnRight() {
    Serial.println("Turning Right (Pivot)");
    // Left side forward
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, LOW);
    // Right side reverse
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, HIGH);
}

// --- NEW: Mecanum Wheel Specific Movements ---
// NOTE: These functions will only produce the desired motion
// if you are using Mecanum wheels on your robot.

void strafeLeft() {
    Serial.println("Strafing Left");
    // Front-left and rear-right motors reverse
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, HIGH);
    // Front-right and rear-left motors forward
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, LOW);
}

void strafeRight() {
    Serial.println("Strafing Right");
    // Front-left and rear-right motors forward
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, LOW);
    // Front-right and rear-left motors reverse
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, HIGH);
}

void diagonalForwardLeft() {
    Serial.println("Diagonal Forward-Left");
    // Front-right and rear-left motors forward, others stop
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, LOW);
}

void diagonalForwardRight() {
    Serial.println("Diagonal Forward-Right");
    // Front-left and rear-right motors forward, others stop
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, LOW);
}


// --- Arm and Gripper Functions (Unchanged) ---
void armUp() { Serial.println("Arm Up"); digitalWrite(ARM_MOTOR_UP, HIGH); digitalWrite(ARM_MOTOR_DOWN, LOW); }
void armDown() { Serial.println("Arm Down"); digitalWrite(ARM_MOTOR_UP, LOW); digitalWrite(ARM_MOTOR_DOWN, HIGH); }
void stopArm() { Serial.println("Stop Arm"); digitalWrite(ARM_MOTOR_UP, LOW); digitalWrite(ARM_MOTOR_DOWN, LOW); }
void gripperOpen() { Serial.println("Gripper Open"); digitalWrite(GRIPPER_MOTOR_OPEN, HIGH); digitalWrite(GRIPPER_MOTOR_CLOSE, LOW); }
void gripperClose() { Serial.println("Gripper Close"); digitalWrite(GRIPPER_MOTOR_OPEN, LOW); digitalWrite(GRIPPER_MOTOR_CLOSE, HIGH); }
void stopGripper() { Serial.println("Stop Gripper"); digitalWrite(GRIPPER_MOTOR_OPEN, LOW); digitalWrite(GRIPPER_MOTOR_CLOSE, LOW); }

void addCorsHeaders(AsyncWebServerResponse *response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ------------------- WiFi Connection Logic (Unchanged) -------------------
void connectToWiFi() {
    Serial.println("Configuring static IP...");
    if (!WiFi.config(staticIP, gateway, subnet)) {
        Serial.println("STA Failed to configure");
    }

    int numNetworks = sizeof(knownNetworks) / sizeof(knownNetworks[0]);
    for (int i = 0; i < numNetworks; i++) {
        Serial.print("\nTrying to connect to: ");
        Serial.println(knownNetworks[i].ssid);
        WiFi.begin(knownNetworks[i].ssid, knownNetworks[i].password);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n\nSUCCESS!");
            Serial.print("Connected to ");
            Serial.println(knownNetworks[i].ssid);
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            return; // Exit the function once connected
        } else {
            Serial.println("\nConnection failed.");
        }
    }

    Serial.println("\nNo known Wi-Fi networks found. Starting Access Point.");
    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("AP SSID: ");
    Serial.println(ap_ssid);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
}


// ------------------- Setup Function -------------------
void setup() {
    Serial.begin(115200);

    // --- MODIFIED: Set all 8 motor control pins to outputs ---
    pinMode(FRONT_LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(FRONT_LEFT_MOTOR_REVERSE, OUTPUT);
    pinMode(FRONT_RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(FRONT_RIGHT_MOTOR_REVERSE, OUTPUT);
    pinMode(REAR_LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(REAR_LEFT_MOTOR_REVERSE, OUTPUT);
    pinMode(REAR_RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(REAR_RIGHT_MOTOR_REVERSE, OUTPUT);

    // Arm and gripper pins
    pinMode(ARM_MOTOR_UP, OUTPUT);
    pinMode(ARM_MOTOR_DOWN, OUTPUT);
    pinMode(GRIPPER_MOTOR_OPEN, OUTPUT);
    pinMode(GRIPPER_MOTOR_CLOSE, OUTPUT);

    // Stop all motors initially
    stopCar();
    stopArm();
    stopGripper();

    // Call the WiFi connection logic
    connectToWiFi();

    // --- MODIFIED: Web Server Request Handlers ---

    // Basic 4WD movement
    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){ moveForward(); request->send(200, "text/plain", "OK"); });
    server.on("/reverse", HTTP_GET, [](AsyncWebServerRequest *request){ moveReverse(); request->send(200, "text/plain", "OK"); });
    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){ turnLeft(); request->send(200, "text/plain", "OK"); });
    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){ turnRight(); request->send(200, "text/plain", "OK"); });
    server.on("/stop-car", HTTP_GET, [](AsyncWebServerRequest *request){ stopCar(); request->send(200, "text/plain", "OK"); });

    // --- NEW: Endpoints for Mecanum/4WD specific movements ---
    server.on("/strafe-left", HTTP_GET, [](AsyncWebServerRequest *request){ strafeLeft(); request->send(200, "text/plain", "OK"); });
    server.on("/strafe-right", HTTP_GET, [](AsyncWebServerRequest *request){ strafeRight(); request->send(200, "text/plain", "OK"); });
    server.on("/diag-fl", HTTP_GET, [](AsyncWebServerRequest *request){ diagonalForwardLeft(); request->send(200, "text/plain", "OK"); });
    server.on("/diag-fr", HTTP_GET, [](AsyncWebServerRequest *request){ diagonalForwardRight(); request->send(200, "text/plain", "OK"); });

    // Arm and Gripper controls
    server.on("/arm-up", HTTP_GET, [](AsyncWebServerRequest *request){ armUp(); request->send(200, "text/plain", "OK"); });
    server.on("/arm-down", HTTP_GET, [](AsyncWebServerRequest *request){ armDown(); request->send(200, "text/plain", "OK"); });
    server.on("/stop-arm", HTTP_GET, [](AsyncWebServerRequest *request){ stopArm(); request->send(200, "text/plain", "OK"); });
    server.on("/gripper-open", HTTP_GET, [](AsyncWebServerRequest *request){ gripperOpen(); request->send(200, "text/plain", "OK"); });
    server.on("/gripper-close", HTTP_GET, [](AsyncWebServerRequest *request){ gripperClose(); request->send(200, "text/plain", "OK"); });
    server.on("/stop-gripper", HTTP_GET, [](AsyncWebServerRequest *request){ stopGripper(); request->send(200, "text/plain", "OK"); });

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
    // No code is needed here.
}