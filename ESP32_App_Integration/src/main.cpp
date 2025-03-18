#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// Replace with your Wi-Fi credentials
//const char* ssid     = "boyzlife";
//const char* password = "swath9-nit-tip";

const char* ssid     = "Felix’s iPhone";
const char* password = "bgjbkfg58knmx";

// Create an HTTP server on port 80
AsyncWebServer server(80);

// Variable to store module height (for demonstration, one value is used)
int moduleHeight = 0;

void setup() {
  Serial.begin(115200);
  Serial.print("Before Connecting to wifi\n");
  WiFi.begin(ssid, password);

  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
  }

  Serial.print("Connecting to WiFi\n");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  // Define the HTTP endpoint to set the module height.
  server.on("/setHeight", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("height") && request->hasParam("module")) {
      int height = request->getParam("height")->value().toInt();
      int module = request->getParam("module")->value().toInt();
      moduleHeight = height;  // Update module height (for demonstration)
      Serial.printf("Module %d height set to %d inches\n", module, height);
      
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Height updated");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    } else {
      AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", "Missing parameters");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    }
  });  

  server.begin();
}

void loop() {
  // The asynchronous server handles requests in the background.
}
