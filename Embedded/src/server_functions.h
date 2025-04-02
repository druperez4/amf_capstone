#include "main.h"

#define CONTENT_TYPE "application/json"

void setup_server();
void getMap(AsyncWebServerRequest *request);
void messageI2C(AsyncWebServerRequest *request);
void notFound(AsyncWebServerRequest *request);
