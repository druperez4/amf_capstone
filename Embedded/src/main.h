#ifndef I2C_WEBSERVER_H
#define I2C_WEBSERVER_H

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <stdio.h>

#define CONTENT_TYPE "application/json"
#define ADDRESS 0
#define I2C_SLAVES 16

boolean MASTER = 1;
int i2c_ids[I2C_SLAVES] = {0};
const char* ssid = "ESP32-Network";
const char* password = "12345678";

// Function Declarations
void discoverI2C();
void requestEvent();
void processI2C(String message);
void receiveEvent(int bytes);
void sendI2C(int address, String message);
void notFound(AsyncWebServerRequest *request);
void messageI2C(AsyncWebServerRequest *request);
void getMap(AsyncWebServerRequest *request);
void shutdown();
void determineMaster();
void init();
void setup();
void loop();

#endif // I2C_WEBSERVER_H
