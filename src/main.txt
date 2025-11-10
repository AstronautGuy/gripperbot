/*
 * ESP32 Wi-Fi Controlled 4-Wheel Robot Car with Arm and Gripper
 * MODIFIED VERSION (4-Wheel Drive) with random DHCP IP
 */

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// ------------------- Wi-Fi Configuration -------------------
struct WiFiNetwork {
    const char* ssid;
    const char* password;
};

WiFiNetwork knownNetworks[] = {
    {"Hotspot", "12345678"},
    {"STUDENT", ""},
    {"WorkshopNet", "Password12345"}
};

// Fallback Access Point
const char* ap_ssid = "RobotCar_AP";
const char* ap_password = "password";

// ------------------- Pin Definitions -------------------
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

// Arm and Gripper
const int ARM_MOTOR_UP = 12;
const int ARM_MOTOR_DOWN = 13;
const int GRIPPER_MOTOR_OPEN = 18;
const int GRIPPER_MOTOR_CLOSE = 19;

// ------------------- Server -------------------
AsyncWebServer server(80);

// ------------------- Motor Control Functions -------------------
void stopCar() {
    Serial.println("Stopping Car");
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
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, LOW);
}

void moveReverse() {
    Serial.println("Moving Reverse");
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, HIGH);
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, HIGH);
}

void turnLeft() {
    Serial.println("Turning Left (Pivot)");
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, HIGH);
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, LOW);
}

void turnRight() {
    Serial.println("Turning Right (Pivot)");
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, HIGH);
}

// Mecanum wheel movements
void strafeLeft() {
    Serial.println("Strafing Left");
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, HIGH);
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, LOW);
}

void strafeRight() {
    Serial.println("Strafing Right");
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, HIGH);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, HIGH);
}

void diagonalForwardLeft() {
    Serial.println("Diagonal Forward-Left");
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
    digitalWrite(FRONT_LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(FRONT_LEFT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(REAR_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(FRONT_RIGHT_MOTOR_REVERSE, LOW);
    digitalWrite(REAR_LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(REAR_LEFT_MOTOR_REVERSE, LOW);
}

// Arm & Gripper
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

// ------------------- Wi-Fi Logic -------------------
void connectToWiFi() {
    Serial.println("Connecting to available Wi-Fi (DHCP/random IP)...");

    int numNetworks = sizeof(knownNetworks) / sizeof(knownNetworks[0]);
    for (int i = 0; i < numNetworks; i++) {
        Serial.print("\nTrying: ");
        Serial.println(knownNetworks[i].ssid);

        WiFi.begin(knownNetworks[i].ssid, knownNetworks[i].password); // DHCP

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✅ Connected successfully!");
            Serial.print("Network: ");
            Serial.println(knownNetworks[i].ssid);
            Serial.print("Assigned IP: ");
            Serial.println(WiFi.localIP());
            return;
        } else {
            Serial.println("\n❌ Failed to connect.");
        }
    }

    Serial.println("\nNo known Wi-Fi found. Starting AP...");
    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("AP SSID: ");
    Serial.println(ap_ssid);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

// ------------------- Setup -------------------
void setup() {
    Serial.begin(115200);

    // Motor pins
    pinMode(FRONT_LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(FRONT_LEFT_MOTOR_REVERSE, OUTPUT);
    pinMode(FRONT_RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(FRONT_RIGHT_MOTOR_REVERSE, OUTPUT);
    pinMode(REAR_LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(REAR_LEFT_MOTOR_REVERSE, OUTPUT);
    pinMode(REAR_RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(REAR_RIGHT_MOTOR_REVERSE, OUTPUT);

    // Arm & gripper
    pinMode(ARM_MOTOR_UP, OUTPUT);
    pinMode(ARM_MOTOR_DOWN, OUTPUT);
    pinMode(GRIPPER_MOTOR_OPEN, OUTPUT);
    pinMode(GRIPPER_MOTOR_CLOSE, OUTPUT);

    stopCar();
    stopArm();
    stopGripper();

    connectToWiFi();

    // 4WD endpoints
    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *r){ moveForward(); r->send(200,"text/plain","OK"); });
    server.on("/reverse", HTTP_GET, [](AsyncWebServerRequest *r){ moveReverse(); r->send(200,"text/plain","OK"); });
    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *r){ turnLeft(); r->send(200,"text/plain","OK"); });
    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *r){ turnRight(); r->send(200,"text/plain","OK"); });
    server.on("/stop-car", HTTP_GET, [](AsyncWebServerRequest *r){ stopCar(); r->send(200,"text/plain","OK"); });

    // Mecanum
    server.on("/strafe-left", HTTP_GET, [](AsyncWebServerRequest *r){ strafeLeft(); r->send(200,"text/plain","OK"); });
    server.on("/strafe-right", HTTP_GET, [](AsyncWebServerRequest *r){ strafeRight(); r->send(200,"text/plain","OK"); });
    server.on("/diag-fl", HTTP_GET, [](AsyncWebServerRequest *r){ diagonalForwardLeft(); r->send(200,"text/plain","OK"); });
    server.on("/diag-fr", HTTP_GET, [](AsyncWebServerRequest *r){ diagonalForwardRight(); r->send(200,"text/plain","OK"); });

    // Arm & gripper
    server.on("/arm-up", HTTP_GET, [](AsyncWebServerRequest *r){ armUp(); r->send(200,"text/plain","OK"); });
    server.on("/arm-down", HTTP_GET, [](AsyncWebServerRequest *r){ armDown(); r->send(200,"text/plain","OK"); });
    server.on("/stop-arm", HTTP_GET, [](AsyncWebServerRequest *r){ stopArm(); r->send(200,"text/plain","OK"); });
    server.on("/gripper-open", HTTP_GET, [](AsyncWebServerRequest *r){ gripperOpen(); r->send(200,"text/plain","OK"); });
    server.on("/gripper-close", HTTP_GET, [](AsyncWebServerRequest *r){ gripperClose(); r->send(200,"text/plain","OK"); });
    server.on("/stop-gripper", HTTP_GET, [](AsyncWebServerRequest *r){ stopGripper(); r->send(200,"text/plain","OK"); });

    server.onNotFound([](AsyncWebServerRequest *r){
        if(r->method()==HTTP_OPTIONS){
            AsyncWebServerResponse *res = r->beginResponse(204);
            addCorsHeaders(res);
            r->send(res);
        } else {
            r->send(404,"text/plain","Not found");
        }
    });

    server.begin();
    Serial.println("HTTP server started");
}

// ------------------- Loop -------------------
void loop() {
    // Async server handles everything
}