#include "I2C.h"
#include "motor.h"

void processI2C(String message) {
    // TODO: Decide all different message types

    // Reset I2C
    if (message.charAt(0) == 'R') {
        // Tell all slaves to reset
        if (MASTER) {
            for (int i = 0; i < I2C_SLAVES; i++){
                sendI2C(i, "R");
            }
        }

        reset();
    }

    if (message.charAt(0) == 'L') {
        // Toggle Light on Message Receive
        digitalWrite(LED, !digitalRead(LED));
    }

    if (message.charAt(0) == 'H') {
        // Change Height on message receive
        update_target(message.substring(1).toFloat());
    }
}

void sendI2C(int address, String message){
    Serial.printf("Sending Message: %s\n To address %d\n", message.c_str(), address);

    Wire.beginTransmission(address);
    Wire.write(message.c_str());
    Serial.printf("I2C send result: %d\n", Wire.endTransmission());
}

void receiveEvent(int bytes) {
    char buffer[33]; // I2C max size is 32 bytes
    int i = 0;

    while (Wire.available() && i < 32) {
        buffer[i++] = Wire.read();
    }
    buffer[i] = '\0'; // Null-terminate the buffer

    String message = String(buffer);
    Serial.printf("Processing message: %s\n", message.c_str());

    processI2C(message);
}

void requestEvent(){
    Wire.write(1);
}
