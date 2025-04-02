#include "server_functions.h"
#include "I2C.h"

AsyncWebServer server(80);

void setup_server() {
    server.on("/message", HTTP_POST, messageI2C);
    server.on("/getMap", HTTP_GET, getMap);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", "<h1>Hello from ESP32 SoftAP!</h1>");
      });        
    server.onNotFound(notFound);
    
    server.begin();
    Serial.println("HTTP server started");
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, CONTENT_TYPE, "{\"error\":\"Not Found\"}");
}

void messageI2C(AsyncWebServerRequest *request) {
    if (request->hasParam("body", true)) {
        String body = request->getParam("body", true)->value();
        Serial.printf("Received Message: %s\n", body);

        // Parse JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, body);

        if (error) {
            request->send(400, CONTENT_TYPE, "{\"error\":\"Invalid JSON\"}");
            return;
        }

        int address = doc["address"];
        Serial.printf("Address: %d\n", address);

        String message = doc["message"].as<String>();
        Serial.printf("Message: %s\n", message.c_str());

        if (address == ADDRESS) {
            processI2C(message);
        } else {
            sendI2C(address, message);
        }

        JsonDocument res;
        String response;
        String responseMsg;
        res["response"] = String("Message forwarded to index ") + address;
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