#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

const char* ssid = "ESP32-Network";
const char* password = "12345678";

AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "application/json", "{\"error\":\"Not Found\"}");
}

void setup() {
    Serial.begin(115200);

    WiFi.softAP(ssid, password);
    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/send", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("body", true)) {
            String body = request->getParam("body", true)->value();
            Serial.println("Received Message: " + body);

            // Parse JSON
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, body);

            if (error) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }

            String message = doc["message"].as<String>();
            Serial.println("Message: " + message);

            request->send(200, "application/json", "{\"response\":\"Message received\"}");
        } else {
            request->send(400, "application/json", "{\"error\":\"Missing body\"}");
        }
    });

    server.onNotFound(notFound);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    // Nothing needed in loop, async server handles requests
}
