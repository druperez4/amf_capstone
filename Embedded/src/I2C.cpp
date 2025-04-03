#include "I2C.h"
#include "motor.h"

I2CMessage* head = nullptr;
I2CMessage* tail = nullptr;

void setupI2C(){
    new (&Wire) TwoWire(0);           // Reconstruct it (placement new)
    if (MASTER) {
        Wire.setPins(I2C_SDA, I2C_SCL);
        Wire.begin();
        Serial.printf("started I2C as master %d\n", ADDRESS);
    } else {
        Wire.setPins(I2C_SDA, I2C_SCL);
        Wire.begin(ADDRESS);
        Wire.onReceive(receiveEvent);
        Wire.onRequest(requestEvent);
        Serial.print("started I2C as slave ");
        Serial.println(ADDRESS);
    }
}

void enqueueI2C(int module, String message) {
    I2CMessage* newNode = new I2CMessage{module, message, nullptr};

    if (tail == nullptr) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }

    Serial.printf("[Queue] Enqueued to %d: %s\n", module, message.c_str());
}

void processI2CQueue() {
    while (head != nullptr) {
        I2CMessage* current = head;

        deliver_message(current->module, current->message);

        // If we reset, discard the queue
        if (head == nullptr)
            break;

        head = head->next;
        if (head == nullptr) tail = nullptr;

        delete current;
        delay(10); // Let the watchdog breathe
        yield();   // Yield for ESP32
    }
}

void endI2C(){
    if (MASTER) {
        // Delete pending messages
        while (head != nullptr) {
            I2CMessage* current = head;
    
            head = head->next;
            if (head == nullptr) tail = nullptr;
    
            delete current;
            delay(10); // Let the watchdog breathe
            yield();   // Yield for ESP32
        }
        Wire.end();
    }
    Serial.println("Ended I2C");
}

void processI2C(String message) {
    // TODO: Decide all different message types

    // Reset I2C
    if (message.charAt(0) == 'R') {
        // Tell all slaves to reset
        if (MASTER) {
            for (int i = 0; i < I2C_SLAVES; i++){
                if (i != ADDRESS) {
                    sendI2C(i, "R");
                    delay(10);
                    yield();
                }
            }
        }

        delay(100);
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

void deliver_message(int module, String message) {
    if (module == ADDRESS) {
        processI2C(message);
    } else {
        sendI2C(module, message);
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
