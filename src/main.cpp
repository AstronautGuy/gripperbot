/*
 * ESP32 Wi-Fi Controlled Robot Car with Arm and Gripper
 * * MODIFIED VERSION:
 * - Attempts to connect to a list of known Wi-Fi networks.
 * - Uses a static IP address for predictable access.
 * - Creates its own Wi-Fi Access Point (AP) as a fallback if no known networks are found.
 * * HARDWARE REQUIRED: (Same as original)
 *
 * HOW TO USE:
 * 1. Install "ESPAsyncWebServer" and "AsyncTCP" libraries.
 * 2. Update the `knownNetworks` array with your Wi-Fi credentials.
 * 3. Update the `staticIP`, `gateway`, and `subnet` to match YOUR network configuration.
 * 4. (Optional) Change the `ap_ssid` and `ap_password` for the fallback hotspot.
 * 5. Upload the code to your ESP32.
 * 6. Open the Serial Monitor at 115200 baud.
 * 7. The ESP32 will print which network it connected to and its static IP address.
 * 8. If it can't connect, it will create a hotspot. Connect to "RobotCar_AP" (or your custom name)
 * with the password and navigate to 192.168.4.1 in your browser.
 */

// ------------------- Libraries -------------------
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// ------------------- Wi-Fi Configuration -------------------

// --- NEW: Define a structure to hold network credentials ---
struct WiFiNetwork {
    const char* ssid;
    const char* password;
};

// --- NEW: List of known Wi-Fi networks to try ---
// Add your networks here. The ESP32 will try them in order.
WiFiNetwork knownNetworks[] = {
    {"Devansh-jio", "Devansh@#$2007"},
    {"MyHomeWiFi", "MyHomePassword"},
    {"WorkshopNet", "Password12345"}
};

// --- NEW: Static IP Configuration ---
// Set this to an IP address that is available on your network.
IPAddress staticIP(192, 168, 1, 184);
// Your router's IP address
IPAddress gateway(192, 168, 1, 1);
// Subnet mask (usually this is correct)
IPAddress subnet(255, 255, 255, 0);

// --- NEW: Access Point (Hotspot) credentials ---
// This is used as a fallback if no known networks are found.
const char* ap_ssid = "RobotCar_AP";
const char* ap_password = "password";


// ------------------- Pin Definitions -------------------
// (No changes here, pins remain the same)
const int LEFT_MOTOR_FORWARD = 26;
const int LEFT_MOTOR_REVERSE = 25;
const int RIGHT_MOTOR_FORWARD = 14;
const int RIGHT_MOTOR_REVERSE = 27;
const int ARM_MOTOR_UP = 12;
const int ARM_MOTOR_DOWN = 13;
const int GRIPPER_MOTOR_OPEN = 15;
const int GRIPPER_MOTOR_CLOSE = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// --- Web Page HTML (Assumed to be defined elsewhere or is not needed for this logic) ---
// const char index_html[] PROGMEM = R"rawliteral( ... your html here ... )rawliteral";


// ------------------- Motor Control Functions -------------------
// (No changes in this section)
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

// ------------------- NEW WiFi Connection Logic -------------------
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

        // Try to connect for 10 seconds
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

    // If we're here, no known network was found. Start AP.
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

    // --- NEW: Call the WiFi connection logic ---
    connectToWiFi();

    // --- Web Server Request Handlers (No changes here) ---

    // IMPORTANT: If you have HTML, uncomment this line
    // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //    request->send_P(200, "text/html", index_html);
    // });

    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){ moveForward(); request->send(200, "text/plain", "OK"); });
    server.on("/reverse", HTTP_GET, [](AsyncWebServerRequest *request){ moveReverse(); request->send(200, "text/plain", "OK"); });
    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){ turnLeft(); request->send(200, "text/plain", "OK"); });
    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){ turnRight(); request->send(200, "text/plain", "OK"); });
    server.on("/stop-car", HTTP_GET, [](AsyncWebServerRequest *request){ stopCar(); request->send(200, "text/plain", "OK"); });
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