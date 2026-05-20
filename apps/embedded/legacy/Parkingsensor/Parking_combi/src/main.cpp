#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Config --- 
const int SDA_PIN = 8;
const int SCL_PIN = 9;
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int SCREEN_ADDRESS = 0x3C;

const int trigPin = 5;
const int echoPin = 4;
const int redpin = 38; 
const int greenpin = 36; 
const int sensitivity = 5;

void renderParkingStatus(int count);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int val;

float duration, distance;


void setup() {
	
  pinMode(redpin, OUTPUT);
	pinMode(greenpin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(115200);
  delay(1000); 
  Serial.println("System Initializing...");

  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C Bus Started.");

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { 
    Serial.println(F("SSD1306 allocation failed! Check wiring or address."));
    for(;;); 
  }

  Serial.println("Display initialized successfully!");
  display.clearDisplay();
  display.display(); 
}

void loop() {
  int freeParkingSpots = 12; 
  Serial.print("Updating display with: ");
  Serial.println(freeParkingSpots);

  renderParkingStatus(freeParkingSpots);
  
  delay(1000); 

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

void renderParkingStatus(int count) {
  display.clearDisplay(); 
  
  display.setTextSize(1); // Smaller text for header
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("FREE PARKING:");
  
  display.setTextSize(4);
  display.setCursor(35, 20); // Centering the big number better
  display.print(count);
  
  display.display(); 
}