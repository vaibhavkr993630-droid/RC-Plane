#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "RC_Controller";
const char* password = "12345678";

WiFiUDP udp;
const int udpPort = 2222;
IPAddress receiverIP(192, 168, 4, 2);

const int LEFT_Y_PIN = 34;
const int LEFT_X_PIN = 35;
const int RIGHT_Y_PIN = 32;
const int RIGHT_X_PIN = 33;
const int RIGHT_BTN = 26;
const int LEFT_BTN = 25;

struct Controls {
    int16_t elevator = 107;
    int16_t rudder = 105;
    int16_t leftWing = 70;
    int16_t rightWing = 68;
    uint16_t bldc = 1000;
    bool syncMode = false;
    bool calibrationMode = false;
    uint32_t lastTransmission = 0;
} controls;

const int FILTER_SAMPLES = 5;
int filterIndex = 0;
int filterBuffers[4][FILTER_SAMPLES];
int filteredValues[4] = {0};

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    udp.begin(udpPort);
    
    pinMode(RIGHT_BTN, INPUT_PULLUP);
    pinMode(LEFT_BTN, INPUT_PULLUP);
    
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < FILTER_SAMPLES; j++) {
            filterBuffers[i][j] = 0;
        }
    }
}

int smoothAnalogRead(int pin, int bufferIndex) {
    filterBuffers[bufferIndex][filterIndex] = analogRead(pin);
    int sum = 0;
    for(int i = 0; i < FILTER_SAMPLES; i++) {
        sum += filterBuffers[bufferIndex][filterIndex];
    }
    return sum / FILTER_SAMPLES;
}

int applyDeadZone(int value, int center = 2048, int threshold = 205) {
    if(abs(value - center) < threshold) return center;
    return value;
}

void processInputs() {
    filterIndex = (filterIndex + 1) % FILTER_SAMPLES;
    
    int leftY = applyDeadZone(smoothAnalogRead(LEFT_Y_PIN, 0));
    int leftX = applyDeadZone(smoothAnalogRead(LEFT_X_PIN, 1));
    int rightY = applyDeadZone(smoothAnalogRead(RIGHT_Y_PIN, 2));
    int rightX = applyDeadZone(smoothAnalogRead(RIGHT_X_PIN, 3));
    
    // Check for ESC calibration condition
    if(leftY < 100 && leftX > 4000 && rightY < 100 && rightX > 4000) {
        controls.calibrationMode = true;
        controls.bldc = 2000; // Maximum throttle for calibration
        delay(2000);
        controls.bldc = 1000; // Minimum throttle to complete calibration
        return;
    }
    
    controls.calibrationMode = false;
    
    // Process elevator (reversed)
    controls.elevator = map(leftY, 4095, 0, 80, 150);
    controls.elevator = constrain(controls.elevator, 80, 150);
    
    // Process rudder
    controls.rudder = map(leftX, 0, 4095, 70, 130);
    controls.rudder = constrain(controls.rudder, 70, 130);
    
    // Process BLDC
    static int lastBLDC = controls.bldc;
    int targetBLDC = map(rightY, 2048, 4095, 1000, 2000);
    if(targetBLDC > lastBLDC) {
        controls.bldc = targetBLDC;
    } else if(targetBLDC < lastBLDC) {
        controls.bldc = targetBLDC;
    }
    
    // Process wings
    if(!controls.syncMode) {
        controls.leftWing = map(rightX, 0, 4095, 30, 110);
        controls.rightWing = map(rightX, 0, 4095, 20, 100);
    }
    
    if(digitalRead(RIGHT_BTN) == LOW) {
        controls.syncMode = true;
        int syncValue = map(leftY, 0, 4095, -40, 40);
        controls.elevator = 107 + syncValue;
        controls.leftWing = 70 + syncValue;
        controls.rightWing = 68 - syncValue;
    } else {
        controls.syncMode = false;
    }
    
    if(leftX == 2048 && leftY == 2048) {
        controls.bldc = 1000;
    }
}

void transmitData() {
    if(millis() - controls.lastTransmission >= 20) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%d,%d,%d,%d,%d,%d", 
                controls.elevator, controls.rudder, 
                controls.leftWing, controls.rightWing, 
                controls.bldc, controls.calibrationMode);
        
        udp.beginPacket(receiverIP, udpPort);
        udp.write((uint8_t*)buffer, strlen(buffer));
        udp.endPacket();
        
        controls.lastTransmission = millis();
    }
}

void loop() {
    processInputs();
    transmitData();
}
