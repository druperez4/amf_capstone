#ifndef I2C_WEBSERVER_H
#define I2C_WEBSERVER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <stdio.h>

#define I2C_SLAVES 16
#define I2C_SDA 27
#define I2C_SCL 26
#define ADDRESS 0

#define LED 33

extern const char* ssid;
extern const char* password;
extern AsyncWebServer server;
extern boolean MASTER;

// // Function Declarations
void reset();
#endif // I2C_WEBSERVER_H
