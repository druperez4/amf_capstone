#include "hotspot.h"

void setupWiFi(){
    if (MASTER) {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid, password);
        Serial.println("Access Point Started");
        Serial.print("IP Address: ");
        Serial.println(WiFi.softAPIP());
    }
}

void endWiFi() {
    Serial.println("Ending WiFi!");
    if (MASTER) {
        WiFi.softAPdisconnect();
    }
}