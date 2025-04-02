#include "main.h"
#include "motor.h"
#include "server_functions.h"
#include "I2C.h"

const char* ssid = "ESP32-Network";
const char* password = "12345678";
boolean MASTER = 1;


// void determineMaster() {
//     // Set the master boolean based on algorithm
//     // MASTER = 1;
// }

void start_amf() {
    // Find which arduino in this group is the master
    // determineMaster();

    if (MASTER) {
        Wire.setPins(I2C_SDA, I2C_SCL);
        Wire.begin();
        Serial.printf("started I2C as master %d\n", ADDRESS);

        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid, password);
        Serial.println("Access Point Started");
        Serial.print("IP Address: ");
        Serial.println(WiFi.softAPIP());

        setup_server();
    } else {
        Wire.setPins(I2C_SDA, I2C_SCL);
        Wire.begin(ADDRESS);
        Wire.onReceive(receiveEvent);
        Wire.onRequest(requestEvent);
        Serial.print("started I2C as slave ");
        Serial.println(ADDRESS);
    }
}

void reset(){
    Wire.end();

    if (MASTER) {
        WiFi.softAPdisconnect();
    }

    start_amf();
}

void setup() {
    Serial.begin(9600);
    setup_motor();
    pinMode(LED, OUTPUT);

    start_amf();
}
  
void loop() {
    motor_loop();
}