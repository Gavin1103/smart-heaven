#include <Arduino.h>

const int trigPin = 5;
const int echoPin = 4;
const int redpin = 38; 
const int greenpin = 36; 
const int sensitivity = 5;

int val;

float duration, distance;


void setup() {
	pinMode(redpin, OUTPUT);
	pinMode(greenpin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600); // no magic numbers todo
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2); // no magic numbers todo
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // no magic numbers todo
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // no magic numbers todo

  Serial.print("Distance: ");
  Serial.println(distance);

  delay(500); // no magic numbers todo

  if (distance < sensitivity) {
		analogWrite(redpin, 255 - val); 
    analogWrite(greenpin, val); 
    Serial.print("Vehicle in parkingspot");
  }
  else {
    analogWrite(redpin, val); 
		analogWrite(greenpin, 255 - val); 
    Serial.print("Parkingspot empty");
  }
  delay(500); 
}

