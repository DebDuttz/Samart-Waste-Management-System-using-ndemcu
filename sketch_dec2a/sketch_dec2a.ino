#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include "ESP8266_ISR_Servo.h"

#define REFERENCE_URL "waste-management-by-sam-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "gfs31NxKyEbwfsubvKuAhuCptJcntGg1B2PfEKJx"
#define WIFI_SSID "I1927"
#define WIFI_PASSWORD "AKASH505"
#define DUSTBIN_LENGTH 22
const int ULTRASONIC_TRIG_PIN = D7;
const int ULTRASONIC_ECHO_PIN = D8;
const int IR_SENSOR_PIN = D3;
const int SERVO_PIN = D4;

double distanceVal = 0;
bool objectDetected = false;
unsigned long motorStartTime = 0;
const unsigned long motorDuration = 10000;
ESP8266_ISR_Servo lidServo;

void setup() {
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
  Serial.println(DUSTBIN_LENGTH);
  Firebase.begin(REFERENCE_URL, FIREBASE_AUTH);

  if (Firebase.failed()) {
    Serial.print("Firebase connection failed: ");
    Serial.println(Firebase.error());
  } else {
    Serial.println("Firebase Connected");
    Firebase.setString("/Waste_Management/distance", "0");
    Firebase.setString("/Waste_Management/Remaining", "0");
  }

  lidServo.setupServo(SERVO_PIN);
  lidServo.setPosition(lidServo.setupServo(SERVO_PIN), 0); 
}

void loop() {
  measureDistance();
  detectObject(); 

  int remainingSpace = (DUSTBIN_LENGTH - distanceVal);

  Serial.print("Distance: ");
  Serial.println(distanceVal);

  Serial.print("Garbage Level: ");
  Serial.println(remainingSpace);

  if (remainingSpace <= 8) {
    Serial.println("Empty");
  } else if (remainingSpace <= 16) {
    Serial.println("Half Full");
  } else if (remainingSpace >= 17) {
    Serial.println("Full");
  }

  Firebase.setInt("Remaining", remainingSpace);
  Firebase.setFloat("distance", distanceVal);
  delay(1000);
}

void measureDistance() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  distanceVal = duration * 0.034 / 2;
  
  Firebase.setFloat("distance",distanceVal);
}

void detectObject() {
  if (digitalRead(IR_SENSOR_PIN) == HIGH) {
    objectDetected = true;
    openLid();
    motorStartTime = millis();
  } else {
    objectDetected = false;
  }

  if (objectDetected && millis() - motorStartTime >= motorDuration) {
    closeLid();
    objectDetected = false;
  }
}

void openLid() {
  lidServo.setPosition(lidServo.setupServo(SERVO_PIN), 90);
  delay(1000);
}

void closeLid() {
  lidServo.setPosition(lidServo.setupServo(SERVO_PIN), 0);
  delay(1000);
}