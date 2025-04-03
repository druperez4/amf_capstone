#include "main.h"

struct I2CMessage {
    int module;
    String message;
    I2CMessage* next;
};

void enqueueI2C(int module, String message);

void setupI2C();

void endI2C();

void processI2C(String message);

void processI2CQueue();

void deliver_message(int module, String message);

void sendI2C(int address, String message);

void receiveEvent(int bytes);

void requestEvent();