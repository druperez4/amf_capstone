#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <stdio.h>

#define CONTENT_TYPE "application/json"

// Boolean
#define MASTER 1
// I2C Address of this Arduino
#define ADDRESS 0

const char* ssid = "ESP32-Network";
const char* password = "12345678";

AsyncWebServer server(80);

void receiveEvent(int bytes) {
    String message;
    while (Wire.available()) {
      message += Wire.read();
    }

    Serial.println("Processing message: " + message);

    processI2C(message);
}

void processI2C(String message) {
    // TODO: Decide all different message types
}


void sendI2C(int address, String message){
    Wire.beginTransmission(address);
    Wire.write(message.c_str());
    Wire.endTransmission();
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, CONTENT_TYPE, "{\"error\":\"Not Found\"}");
}

void messageI2C(AsyncWebServerRequest *request) {
    if (request->hasParam("body", true)) {
        String body = request->getParam("body", true)->value();
        Serial.println("Received Message: " + body);

        // Parse JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, body);

        if (error) {
            request->send(400, CONTENT_TYPE, "{\"error\":\"Invalid JSON\"}");
            return;
        }

        int address = doc["address"].as<int>();
        Serial.println("Address: " + address);

        String message = doc["message"].as<String>();
        Serial.println("Message: " + message);

        if (address == ADDRESS) {
            processI2C(message);
        } else {
            sendI2C(address, message);
        }

        JsonDocument res;
        String response;
        res["response"] = "Message forwarded to index " + address;
        serializeJson(res, response);

        request->send(200, CONTENT_TYPE, response);
    } else {
        request->send(400, CONTENT_TYPE, "{\"error\":\"Missing body\"}");
    }
}

void getMap(AsyncWebServerRequest *request) {
    JsonDocument res;
    String response;

    // TODO: Add JSON representation of AMF mapping
    serializeJson(res, response);

    request->send(200, CONTENT_TYPE, response);
}

void setup() {
    Serial.begin(115200);

    if (MASTER) {
        Wire.begin();
    } else {
        Wire.begin(ADDRESS);
        Wire.onReceive(receiveEvent);
    }
    Serial.println("Initialized I2C");

    WiFi.softAP(ssid, password);
    Serial.println("Access Point Started");
    Serial.print("IP Address: " + WiFi.softAPIP());

    server.on("/message", HTTP_POST, messageI2C);
    server.on("/getMap", HTTP_GET, getMap);
    server.onNotFound(notFound);

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    // Nothing needed in loop, async server handles requests
}
