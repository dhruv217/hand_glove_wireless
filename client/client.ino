/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

#define sensorPin1 D2
#define sensorPin2 D3
#define sensorPin3 D4
#define sensorReadPin A0



int sensor1Value = 0, sensor2Value=0, sensor3Value=0;
int initSensor1Value = 0, initSensor2Value=0, initSensor3Value=0;
int state = 0, oldState = 0;

ESP8266WiFiMulti WiFiMulti;

void readSensors() {

  digitalWrite(sensorPin1, HIGH);
  delay(200);
  sensor1Value = analogRead(sensorReadPin);
  delay(100);
  digitalWrite(sensorPin1, LOW);
  delay(100);
  digitalWrite(sensorPin2, HIGH);
  delay(200);
  sensor2Value = analogRead(sensorReadPin);
  delay(100);
  digitalWrite(sensorPin2, LOW);
  delay(100);
  digitalWrite(sensorPin3, HIGH);
  delay(200);
  sensor3Value = analogRead(sensorReadPin);
  delay(100);
  digitalWrite(sensorPin3, LOW);
}

void debugValues() {
  Serial.print("Values : ");
  Serial.print(sensor1Value);
  Serial.print(", ");
  Serial.print(sensor2Value);
  Serial.print(", ");
  Serial.print(sensor3Value);
  Serial.print(", ");
  Serial.print(state);
  Serial.println(".");
}

void updateState() {
  if(
    (sensor1Value <= (initSensor1Value + 7)) &&
    (sensor2Value <= (initSensor2Value + 7)) &&
    (sensor3Value <= (initSensor3Value + 7))    
    ) state = 0;
  else if(
    (sensor1Value >= (initSensor1Value + 10)) &&
    (sensor2Value <= (initSensor2Value + 7)) &&
    (sensor3Value <= (initSensor3Value + 7))    
    ) state = 1;
  else if(
    (sensor1Value <= (initSensor1Value + 7)) &&
    (sensor2Value >= (initSensor2Value + 10)) &&
    (sensor3Value <= (initSensor3Value + 7))    
    ) state = 2;
  else if(
    (sensor1Value <= (initSensor1Value + 7)) &&
    (sensor2Value <= (initSensor2Value + 7)) &&
    (sensor3Value >= (initSensor3Value + 10))    
    ) state = 3;
  else if(
    (sensor1Value >= (initSensor1Value + 10)) &&
    (sensor2Value >= (initSensor2Value + 10)) &&
    (sensor3Value <= (initSensor3Value + 7))    
    ) state = 4;
  else if(
    (sensor1Value >= (initSensor1Value + 10)) &&
    (sensor2Value <= (initSensor2Value + 7)) &&
    (sensor3Value >= (initSensor3Value + 10))    
    ) state = 5;
  else if(
    (sensor1Value <= (initSensor1Value + 7)) &&
    (sensor2Value >= (initSensor2Value + 10)) &&
    (sensor3Value >= (initSensor3Value + 10))  
    ) state = 6;
  else if(
    (sensor1Value >= (initSensor1Value + 10)) &&
    (sensor2Value >= (initSensor2Value + 10)) &&
    (sensor3Value >= (initSensor3Value + 10))    
    ) state = 7;  
}

void setInitValues(){
  initSensor1Value = sensor1Value;
  initSensor2Value = sensor2Value;
  initSensor3Value = sensor3Value;
}

void sendData(String url) {
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://192.168.4.1/"+url)) {  // HTTP


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect to Server !!");
    }
  } else {
      Serial.printf("[HTTP} Unable to connect to WIFI !!");
  }
}

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  pinMode(sensorPin1, OUTPUT);
  pinMode(sensorPin2, OUTPUT);
  pinMode(sensorPin3, OUTPUT);

  digitalWrite(sensorPin1, LOW);
  digitalWrite(sensorPin2, LOW);
  digitalWrite(sensorPin3, LOW);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 3; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    readSensors();
    setInitValues();
    delay(500);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("STASSID", "11235813");

  readSensors();
  setInitValues();


}

void loop() {
  readSensors();
  updateState();
  debugValues();
  if (state == 0)
      setInitValues();
  else if (oldState != state) {
    oldState = state;
    sendData("state"+String(state));
  }
  
  delay(500);
}
