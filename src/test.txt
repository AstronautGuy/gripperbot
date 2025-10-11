#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// --- Access Point (Hotspot) credentials ---
const char* ap_ssid = "RobotCar_AP";
const char* ap_password = "password123";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
    Serial.begin(115200);

    // --- Start the Access Point ---
    Serial.println("\nStarting Access Point...");
    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("AP SSID: ");
    Serial.println(ap_ssid);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // --- Web Server Request Handlers ---

    // THIS IS THE CRITICAL PART THAT WAS MISSING
    // This tells the server what to do when you visit the main page.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      // For now, we send a simple text message.
      // Later, you can replace this with your HTML control panel.
      request->send(200, "text/html", "<h1>Success!</h1><p>The web server is working.</p><p><a href='/forward'>Test Forward</a></p>");
    });

    // Add a test command to make sure controls are received
    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Received FORWARD command!");
      request->send(200, "text/plain", "Moving forward command sent.");
    });

    // Start server
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    // Nothing needed here for AsyncWebServer
}