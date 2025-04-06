#include "hotspot.h"

void setupWiFi(){
    if (MASTER) {
        // WiFi.mode(WIFI_AP);
        WiFi.begin(ssid, password);
        Serial.print("Connecting to Wi-Fi");
        
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }
        // Serial.println("Access Point Started");
        // Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
}

void endWiFi() {
    Serial.println("Ending WiFi!");
    if (MASTER) {
        // WiFi.softAPdisconnect();
    }
}