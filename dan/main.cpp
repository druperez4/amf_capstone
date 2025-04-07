#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include <string>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>

// Wifi credentials
// const char* ssid     = "Felix’s iPhone";
// const char* password = "bgjbkfg58knmx";
const char* ssid     = "Juan";
const char* password = "12345678";

// Create an HTTP server on port 80
AsyncWebServer server(80);

#define I2C_DEV_ADDR 0x04

#define NORTH_PIN 16
#define EAST_PIN 4
#define WEST_PIN 2
#define SOUTH_PIN 15

#define IS_MASTER_PIN 34
#define STATUS_LED_PIN 2

#define DIRECTION_PIN 26
#define STEP_PIN 25
#define MOTOR_SPEED 2000

using namespace std;

uint32_t i = 0;
uint32_t r = 0;
bool isMaster = 0;
uint8_t northDetect;
uint8_t eastDetect;
uint8_t southDetect;
uint8_t westDetect;
uint8_t directionDetect = 9;
uint8_t requestedData = 0;

int const DIRECTION_PINS[4] = {NORTH_PIN, EAST_PIN, SOUTH_PIN, WEST_PIN};
int const DIRECTION_DELTAS_X[4] = {0,1,0,-1};
int const DIRECTION_DELTAS_Y[4] = {1,0,-1,0};
char const DIRECTION_CHARS[4] = {'N','E','S','W'};

float currentHeight = 0;

void motor(float new_ht){
    //set spin direction
    if(currentHeight == new_ht){
        return; // if no height change
    }

    float step = 9.69; // 3150(.01) / 3.25 --> 3.25 in. rise
    float dist = abs(currentHeight - new_ht) / .01;
    int spin_steps = int(dist * step);


    
    Serial.println("Spinning...");
    Serial.println(spin_steps);
    Serial.println(dist * step);
    digitalWrite(DIRECTION_PIN, currentHeight > new_ht);

    for(int i = 0; i < spin_steps; i++){
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(MOTOR_SPEED);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(MOTOR_SPEED);
    }
    Serial.println("Done Spinning...");

    currentHeight = new_ht;
  
}

vector<uint8_t> scanI2C(){
    byte error, address;
    int nDevices;

    vector<uint8_t> addresses;

    Serial.println("Scanning...");

    nDevices = 0;
    for(address = 1; address < 127; address++ )
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
          Serial.print("I2C device found at address 0x");
          if (address<16)
            Serial.print("0");
          Serial.print(address,HEX);
          Serial.println("  !");

          addresses.push_back(address);

          nDevices++;
        }
        else if (error==4)
        {
            Serial.print("Unknown error at address 0x");
            if (address<16)
                Serial.print("0");
            Serial.println(address,HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
    
    return addresses;
}


class Module {
    public:
        uint8_t addr;
        int x;
        int y;
        int rotation;
        bool isPlaced;
    
        // Constructor
        Module(uint8_t addr, int x, int y, int rotation, bool isPlaced)
            : addr(addr), x(x), y(y), rotation(rotation), isPlaced(isPlaced) {}
    
        // Default constructor
        Module() : addr(0), x(0), y(0), rotation(0), isPlaced(true) {}

        String toString(){
            String str;
            str += (addr);
            str += x;
            str += y;
            return str;
        }

        void pinModeI2C(int pin, int mode){
            if (addr == I2C_DEV_ADDR){
                pinMode(pin, mode);
            } else {
                // Send digitalWrite command to the slave
                Wire.beginTransmission(addr);
                Wire.write(10); // Request the slave execute digital writes
                Wire.write((uint8_t)pin);
                Wire.write((uint8_t)mode);
                uint8_t error = Wire.endTransmission(true);
            }
        }

        void digitalWriteI2C(int pin, int value){
            //Serial.printf("Device %d setting pin %d to %d mode\n", addr, pin, value);
            if (addr == I2C_DEV_ADDR){
                digitalWrite(pin, value);
            } else {
                // Send digitalWrite command to the slave
                Wire.beginTransmission(addr);
                Wire.write(11); // Request the slave execute digital writes
                Wire.write((uint8_t)pin);
                Wire.write((uint8_t)value);
                uint8_t error = Wire.endTransmission(true);
            }
        }

};

uint8_t getDirectionDetection(uint8_t addr){
    Wire.beginTransmission(addr);
    Wire.write(1); // Request the slave report any detected connections.
    uint8_t error = Wire.endTransmission(true);
    delay(100); // Give slave some time to detect
    
    // Read 1 bytes from the slave
    uint8_t bytesReceived = 0;
    bytesReceived = Wire.requestFrom(addr, 1);
    bytesReceived = Wire.requestFrom(addr, 1);

    //delay(500);

    uint8_t temp = 99;
    while (Wire.available()) {
        temp = Wire.read(); // Read the latest byte
    }
    Serial.printf("Module %X directionDetect: %d\n", addr, temp);
    return temp;
}

vector<Module> buildGrid(){
    vector<uint8_t> unplaced_addr;
    unplaced_addr = scanI2C();

    // Place master module at 0,0
    Module master = Module(I2C_DEV_ADDR, 0, 0, 0, true);
    vector<Module> placed_modules;
    placed_modules.push_back(master);

    // Loop through every placed module to explore the unknown grid
    // This vector grows as new placed modules are found
    for(vector<int>::size_type k = 0; k < placed_modules.size(); k++) {
        // End search if every module is placed
        if(unplaced_addr.size() == 0) break;

        // GET PINS READY TO DRIVE
        placed_modules[k].pinModeI2C(NORTH_PIN, OUTPUT);
        placed_modules[k].pinModeI2C(EAST_PIN, OUTPUT);
        placed_modules[k].pinModeI2C(SOUTH_PIN, OUTPUT);
        placed_modules[k].pinModeI2C(WEST_PIN, OUTPUT);
        placed_modules[k].digitalWriteI2C(NORTH_PIN, LOW);
        placed_modules[k].digitalWriteI2C(EAST_PIN, LOW);
        placed_modules[k].digitalWriteI2C(SOUTH_PIN, LOW);
        placed_modules[k].digitalWriteI2C(WEST_PIN, LOW);
        delay(10);

        // Try each cardinal direction NORTH EAST SOUTH WEST
        for(int d = 0; d < 4; d++){
            uint8_t trueDirection = (4 + d - placed_modules[k].rotation)%4;

            // SET DIRECTION PIN HIGH and poll unplaced addresses for any detections
            Serial.printf("\nModule %X setting %c high\n", placed_modules[k].addr, DIRECTION_CHARS[trueDirection]);
            placed_modules[k].digitalWriteI2C(DIRECTION_PINS[trueDirection], HIGH);
            delay(10); // Give slave some time to detect

            for(vector<int>::size_type i = 0; i < unplaced_addr.size(); i++) {
                
                uint8_t detectedDirection = getDirectionDetection(unplaced_addr[i]);

                // IF HIGH DIRECTIONAL PIN WAS DETECTED
                if(detectedDirection >= 0 && detectedDirection < 4){
                    // Use parent's direction for the direction delta
                    int newX = placed_modules[k].x + DIRECTION_DELTAS_X[d];
                    int newY = placed_modules[k].y + DIRECTION_DELTAS_Y[d];

                    // Use child's direction for detecting its rotation
                    // Difference between expected and detected (opposite is found by adding 2)
                    int expectedRotationIfOrientedNorth = (d+2)%4;
                    int newRotation = (4+expectedRotationIfOrientedNorth-detectedDirection)%4;

                    Module new_module = {unplaced_addr[i], newX, newY, newRotation, true}; 

                    Serial.printf("Module %X placed!\n", unplaced_addr[i]);
                    placed_modules.push_back(new_module);

                    // No longer an unplaced module that needs polling
                    unplaced_addr.erase(unplaced_addr.begin() + i);
                }
            }

            // SET DIRECTION PIN BACK LOW
            placed_modules[k].digitalWriteI2C(DIRECTION_PINS[trueDirection], LOW);
            delay(400);
        }

        // SET PINS BACK TO INPUT
        // TODO: Make this tell the remote slave to change its input pins
        placed_modules[k].pinModeI2C(NORTH_PIN, INPUT_PULLDOWN);
        placed_modules[k].pinModeI2C(EAST_PIN, INPUT_PULLDOWN);
        placed_modules[k].pinModeI2C(SOUTH_PIN, INPUT_PULLDOWN);
        placed_modules[k].pinModeI2C(WEST_PIN, INPUT_PULLDOWN);

    }

    return placed_modules;
}

void onRequest() {
    uint8_t response = 0xFF; // Default invalid response

    if (requestedData == 1) {
        // Update directionDetect
        directionDetect = 9;
        for(int i = 0; i < 4; i++){
            if(digitalRead(DIRECTION_PINS[i])) directionDetect = i;
        }
        response = directionDetect;
    }

    Wire.write(response); // Send back the requested value
    Serial.printf("Reading directionDetect: %d\n", response);
}

void onReceive(int len) {
    uint8_t reading;
    if (Wire.available()) {
        reading = Wire.read(); // Store first byte of master request
    }

    if (reading < 10){
        requestedData = reading;
        Serial.printf("Requested data set to %d\n", reading);
    } else if (reading == 10){ // Pin mode command
        uint8_t pin = Wire.read();
        uint8_t mode = Wire.read();
        pinMode(pin, mode); // Next two bytes are pin and mode
        Serial.printf("Requested to set pin %d to %d mode\n", pin, mode);
    } else if (reading == 11){ // Digital write command
        uint8_t pin = Wire.read();
        uint8_t value = Wire.read();
        digitalWrite(pin, value); // Next two bytes are pin and value
        Serial.printf("Requested to set pin %d to %d\n", pin, value);
    } else if (reading == 20){ // Motor Command
        float newHeight;
        byte * receivedBytes = (byte *) &newHeight;
        for (size_t i = 0; i < sizeof(newHeight); i++) {
            if (Wire.available()) {
                receivedBytes[i] = Wire.read();
            }
        }

        Serial.printf("Requested to set height from %f to %f\n", currentHeight, newHeight);
        motor(newHeight);
    }
}


void requestMotorMovement(float height, int module){
    // Master tells slave to move motor
    if(module != I2C_DEV_ADDR){
        Wire.beginTransmission(module);
        Wire.write(20); // Request the slave be in motor mode
        byte * floatBytes = (byte *) &height;
        for (size_t i = 0; i < sizeof(height); i++) {
            Wire.write(floatBytes[i]);
        }
        uint8_t error = Wire.endTransmission(true);
    }else{
        motor(height);
    }
}

bool moveMotor;
float height;
int moduleCool;

vector<Module> lastPlacedGrid;


void getID(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", String(I2C_DEV_ADDR));
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

bool becomeMasterMode = false;
bool becomeSlaveMode = false;

void masterMode(){
    Wire.end();
    delay(200);

    // MASTER I2C SETUP
    Wire.begin();
    Wire.setClock(10000);
    Serial.println("Became Master");
}

void slaveMode(){
    Wire.end();
    delay(200);

    isMaster = false;

    // SLAVE I2C SETUP
    Wire.begin((uint8_t)I2C_DEV_ADDR);
    Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);
    Serial.println("Became Slave");
}

void becomeMaster(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Becoming Master");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    isMaster = true;
    becomeMasterMode = true;

}

void becomeSlave(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Becoming Slave");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    becomeSlaveMode = true;
}

bool beginGridBuild = false;

void beginGrid(AsyncWebServerRequest *request) {
    Serial.println("Beginning Grid");
    beginGridBuild = true;

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Beginning Grid Building");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void getGrid(AsyncWebServerRequest *request) {
    Serial.println("Placed Modules:");

    DynamicJsonDocument doc(1024); // Adjust size if needed
    JsonArray grid = doc.to<JsonArray>();

    for (auto &module : lastPlacedGrid) {
        JsonObject m = grid.createNestedObject();
        m["addr"] = module.addr;
        m["x"] = module.x;
        m["y"] = module.y;
        m["rotation"] = module.rotation;
        m["isPlaced"] = module.isPlaced;
    }

    String jsonStr;
    serializeJson(doc, jsonStr);

    Serial.println(jsonStr);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonStr);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    
    // String gridString = "";
    // for(vector<int>::size_type i = 0; i != lastPlacedGrid.size(); i++) {
    //     gridString += lastPlacedGrid[i].toString() + "\n";
    // }
    
    // Serial.println(gridString);
    
    // AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", gridString);
    // response->addHeader("Access-Control-Allow-Origin", "*");
    // request->send(response);
}


void setup() {
    // DETERMINE MASTERSHIP FOR TESTING PURPOSES
    pinMode(IS_MASTER_PIN, INPUT_PULLUP);
    delay(1000);
    //isMaster = !digitalRead(IS_MASTER_PIN);
    if(I2C_DEV_ADDR == 0x04){
        isMaster = true;
    }

    // BOTH SETUP
    Serial.begin(9600); 
    Serial.setDebugOutput(true);
    
    if(true){
    
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi\n");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/getID", HTTP_GET, getID);
    server.on("/becomeMaster", HTTP_GET, becomeMaster);
    server.on("/becomeSlave", HTTP_GET, becomeSlave);
    server.on("/beginGrid", HTTP_GET, beginGrid);
    server.on("/getGrid", HTTP_GET, getGrid);


    // Define the HTTP endpoint to set the module height.
    server.on("/setHeight", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("height") && request->hasParam("module")) {
        height = request->getParam("height")->value().toFloat();
        moduleCool = request->getParam("module")->value().toInt();

        Serial.printf("Module %d height set to %f inches\n", moduleCool, height);
        
        if(moveMotor){
            Serial.printf("Error: Still moving motor...");
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "New height ignored, still moving motor");
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        }else{
            moveMotor = true;
        }
        
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

    // DIRECTIONAL PINS
    pinMode(NORTH_PIN, INPUT_PULLDOWN);
    pinMode(EAST_PIN, INPUT_PULLDOWN);
    pinMode(SOUTH_PIN, INPUT_PULLDOWN);
    pinMode(WEST_PIN, INPUT_PULLDOWN);

    // MOTOR DIR AND STEP PINS
    pinMode(DIRECTION_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
}


void loop() {
    unsigned long lastGridUpdate = millis();
    
    if(becomeSlaveMode){
        becomeSlaveMode = false;
        slaveMode();
    }
    
    if(becomeMasterMode){
        becomeMasterMode = false;
        masterMode();
    }
    
    if(!isMaster){ // SLAVE MODE LOOP
        
        // Update directionDetect
        directionDetect = 9;
        for(int i = 0; i < 4; i++){
            if(digitalRead(DIRECTION_PINS[i])) directionDetect = i;
        }
        
    }else{
        if(beginGridBuild){
            beginGridBuild = false;
            lastPlacedGrid = buildGrid();
    
            Serial.println("Placed Modules:");
            for(vector<int>::size_type i = 0; i != lastPlacedGrid.size(); i++) {
                Serial.print(lastPlacedGrid[i].addr,HEX);
                Serial.printf("\tX: %d, Y: %d, rot: %d\n",lastPlacedGrid[i].x,lastPlacedGrid[i].y,lastPlacedGrid[i].rotation);
            }
        }
        
    }
    
    if(moveMotor){
        moveMotor = false;
        requestMotorMovement(height, moduleCool);
    }

}
