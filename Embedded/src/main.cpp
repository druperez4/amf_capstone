#include "main.h"
#include "motor.h"
#include "server_functions.h"
#include "I2C.h"
#include "hotspot.h"

const char* ssid = "ESP32-Network";
const char* password = "12345678";
boolean MASTER = 0;


// void determineMaster() {
//     // Set the master boolean based on algorithm
//     // MASTER = 1;
// }

void start_amf() {
    Serial.println("Starting AMF!");

    // Find which arduino in this group is the master
    // determineMaster();
    setupI2C();
    setupWiFi();
    setup_server();
}

void reset(){
    Serial.println("Resetting!");

    endI2C();
    endWiFi();

    delay(100);
    MASTER = !MASTER;

    start_amf();
}

void setup() {
    Serial.begin(9600);
    setup_motor();
    pinMode(LED, OUTPUT);

    start_amf();
}
  
void loop() {
    // Motor constantly checks it is the correct height
    motor_loop();

    processI2CQueue();
}