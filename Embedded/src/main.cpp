#include "main.h"

// I2C Address of this Arduino
#define ADDRESS 0


void processI2C(String message) {
    // TODO: Decide all different message types

    // Reset I2C
    if (message.charAt(0) == 'R') {
        // Tell all slaves to reset
        if (MASTER) {
            for (int i = 0; i < I2C_SLAVES; i++){
                sendI2C(i, "R");
            }
        }

        reset();
    }
}

void discoverI2C(){
    for (int i = 0 ; i < I2C_SLAVES; i++){
        Wire.requestFrom(i, 1);
        if (Wire.available()) {
            i2c_ids[i] = i;
        }
    }
}

void receiveEvent(int bytes) {
    String message;
    while (Wire.available()) {
      message += Wire.read();
    }

    Serial.println("Processing message: " + message);

    processI2C(message);
}

// Reply to check event
void requestEvent(){
    Wire.write(1);
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

// Needed to switch master of the I2C
void reset(){
    Wire.end();

    if (MASTER) {
        WiFi.softAPdisconnect();
    }

    init();
}

void determineMaster() {
    // Set the master boolean based on algorithm
    MASTER = 1;
}

void init() {
    // Find which arduino in this group is the master
    determineMaster();

    if (MASTER) {
        Wire.begin();
        WiFi.softAP(ssid, password);
        Serial.println("Access Point Started");
        Serial.print("IP Address: " + WiFi.softAPIP());

        AsyncWebServer server(80);
        server.on("/message", HTTP_POST, messageI2C);
        server.on("/getMap", HTTP_GET, getMap);
        server.onNotFound(notFound);
        server.begin();
        Serial.println("HTTP server started");

    } else {
        Wire.begin(ADDRESS);
        Wire.onReceive(receiveEvent);
        Wire.onRequest(requestEvent);
    }

    Serial.println("Initialized I2C");

}

void setup() {
    Serial.begin(9600);
    init();
}

void loop() {
    // Nothing needed in loop, async server handles requests
}
