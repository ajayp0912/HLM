#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"
#include "SparkFun_MMA8452Q.h"
#include <HttpClient.h>

#define BUTTON D2
#define HEARTLED D3
#define SITTINGLED D4
#define BUFFERLENGTH 25
#define SITTING_TIME 1800000

HttpClient http;
MMA8452Q accel;
MAX30105 sensor;
unsigned int stepsTaken = 0;
unsigned int sittingTimer = 0;
int buttonState = 0;
int tracking = 0;
uint32_t irBuffer[BUFFERLENGTH];
uint32_t redBuffer[BUFFERLENGTH];
int32_t spo2;
int8_t spo2Valid;
int32_t heartRate;
int8_t heartRateValid;

http_header_t headers[] = {
    { "Accept" , "*/*"},
    { NULL, NULL }
};

http_request_t request;
http_response_t response;

void setup() {
    Wire.begin();
    Serial.begin(9600);
    pinMode(BUTTON, INPUT);
    pinMode(HEARTLED, OUTPUT);
    pinMode(SITTINGLED, OUTPUT);
    sittingTimer = sittingTimer + SITTING_TIME;
    
    if (!sensor.begin())
    {
        Serial.println("MAX30105 was not found.");
        while (1);
    }
    sensor.setup();
    
    if (accel.begin() == false) {
        Serial.println("MMA8452Q Not Connected.");
        while (1);
    }
}

void loop() {
    float x,y;
    float msq;
    buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH) {
        tracking = 1;
        digitalWrite(HEARTLED, HIGH);
    }
    
    if (tracking == 1) {
        for (int i = 0 ; i < BUFFERLENGTH ; i++)
        {
            while (sensor.available() == false) {
                sensor.check();
            }
            redBuffer[i] = sensor.getRed();
            irBuffer[i] = sensor.getIR();
            sensor.nextSample();
        }
        
        maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFERLENGTH, redBuffer, &spo2, &spo2Valid, &heartRate, &heartRateValid);
        if (spo2Valid && heartRateValid) {
            char buf[50];
            sprintf(buf, "/?hr=%d&o2=%d&sc=%d",heartRate,spo2,stepsTaken);
            request.hostname = "18.218.92.211"; // Public IP Address of Cloud Instance
            request.port = 5000; // Port that the Cloud Server is running on
            request.path = buf;
            http.get(request, response, headers);
            Serial.print("Application>\tResponse status: ");
            Serial.println(response.status);
            Serial.print("Application>\tHTTP Response Body: ");
            Serial.println(response.body);
            tracking = 0;
            digitalWrite(HEARTLED, LOW);
        }
    }
    
    if (accel.available()) {
        x = accel.getCalculatedX();
        y = accel.getCalculatedY();
        msq = sqrt(pow(x,2)+pow(y,2));
        if (msq > 0.8) {
            stepsTaken++;
            Serial.println(stepsTaken);
            sittingTimer = millis() + SITTING_TIME;
        }
    }
    
    if (millis() > sittingTimer) {
        digitalWrite(SITTINGLED, HIGH);
    } else {
        digitalWrite(SITTINGLED, LOW);
    }
    
    delay(100);
}