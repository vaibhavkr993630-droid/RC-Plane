#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>

const char* ssid = "RC_Controller";
const char* password = "12345678";

WiFiUDP udp;
const int udpPort = 2222;

const int RIGHT_WING_PIN = D4;
const int LEFT_WING_PIN = D3;
const int ELEVATOR_PIN = D1;
const int RUDDER_PIN = D0;
const int BLDC_PIN = D5;

Servo rightWing;
Servo leftWing;
Servo elevator;
Servo rudder;
Servo bldc;

const int ELEVATOR_MEAN = 107;
const int RUDDER_MEAN = 105;
const int LEFT_WING_MEAN = 70;
const int RIGHT_WING_MEAN = 68;
const int BLDC_MIN = 1000;

unsigned long lastPacketTime = 0;
const unsigned long FAILSAFE_TIMEOUT = 1000;
unsigned long lastBLDCUpdate = 0;
bool inCalibration = false;

void setup() {
    Serial.begin(115200);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    udp.begin(udpPort);
    
    rightWing.attach(RIGHT_WING_PIN);
    leftWing.attach(LEFT_WING_PIN);
    elevator.attach(ELEVATOR_PIN);
    rudder.attach(RUDDER_PIN);
    bldc.attach(BLDC_PIN);
    
    setFailsafePositions();
}

void setFailsafePositions() {
    elevator.write(ELEVATOR_MEAN);
    rudder.write(RUDDER_MEAN);
    leftWing.write(LEFT_WING_MEAN);
    rightWing.write(RIGHT_WING_MEAN);
    
    if(!inCalibration) {
        int currentSpeed = bldc.readMicroseconds();
        while(currentSpeed > BLDC_MIN) {
            currentSpeed = max(BLDC_MIN, currentSpeed - 50);
            bldc.writeMicroseconds(currentSpeed);
            delay(50);
        }
    }
}

void processPacket() {
    char packet[32];
    int packetSize = udp.parsePacket();
    
    if(packetSize) {
        int len = udp.read(packet, sizeof(packet)-1);
        packet[len] = '\0';
        
        int elevatorVal, rudderVal, leftWingVal, rightWingVal, bldcVal;
        bool calibrationMode;
        sscanf(packet, "%d,%d,%d,%d,%d,%d", 
               &elevatorVal, &rudderVal, 
               &leftWingVal, &rightWingVal, 
               &bldcVal, &calibrationMode);
        
        if(calibrationMode) {
            inCalibration = true;
            bldc.writeMicroseconds(bldcVal);
            return;
        }
        
        inCalibration = false;
        elevator.write(elevatorVal);
        rudder.write(rudderVal);
        leftWing.write(leftWingVal);
        rightWing.write(rightWingVal);
        
        int currentBLDC = bldc.readMicroseconds();
        int targetBLDC = bldcVal;
        
        if(millis() - lastBLDCUpdate >= 50) {
            if(abs(targetBLDC - currentBLDC) > 50) {
                currentBLDC += (targetBLDC > currentBLDC) ? 50 : -50;
            } else {
                currentBLDC = targetBLDC;
            }
            bldc.writeMicroseconds(currentBLDC);
            lastBLDCUpdate = millis();
        }
        
        lastPacketTime = millis();
    }
}

void checkFailsafe() {
    if(millis() - lastPacketTime > FAILSAFE_TIMEOUT) {
        setFailsafePositions();
    }
}

void loop() {
    processPacket();
    checkFailsafe();
}
