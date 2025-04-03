#include "server_functions.h"
#include "I2C.h"

AsyncWebServer server(80);

void setup_server() {
    server.on("/message", HTTP_POST, messageI2C);
    server.on("/getMap", HTTP_GET, getMap);
    server.on("/setHeight", HTTP_POST, setHeight);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", "<h1>Hello from ESP32 SoftAP!</h1>");
      });        
    server.onNotFound(notFound);
    
    if (MASTER) {
        server.begin();
        Serial.println("HTTP server started");
    }
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, CONTENT_TYPE, "{\"error\":\"Not Found\"}");
}

void setHeight(AsyncWebServerRequest *request) {
    if (request->hasParam("height", true) && request->hasParam("module", true)) {
      float height = request->getParam("height", true)->value().toFloat();
      int module = request->getParam("module", true)->value().toInt();

      String message = "H";
      deliver_message(module, message + height);
      Serial.printf("Module %d height set to %f inches\n", module, height);
      
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Height updated");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    } else {
      AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", "Missing parameters");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    }
}

void messageI2C(AsyncWebServerRequest *request) {
    if (request->hasParam("module", true) && request->hasParam("message", true)) {
        String message = request->getParam("message", true)->value();
        Serial.printf("Received Message: %s\n", message);

        int module = request->getParam("module", true)->value().toInt();
        Serial.printf("Address: %d\n", module);

        JsonDocument res;
        String response;
        String responseMsg;
        res["response"] = String("Forwarding message to module ") + module;
        serializeJson(res, response);
        request->send(200, CONTENT_TYPE, response);

        enqueueI2C(module, message);

    } else {
        request->send(400, CONTENT_TYPE, "{\"error\":\"Missing parameters\"}");
    }
}

void getMap(AsyncWebServerRequest *request) {
    JsonDocument res;
    String response;

    // TODO: Add JSON representation of AMF mapping
    serializeJson(res, response);

    request->send(200, CONTENT_TYPE, response);
}