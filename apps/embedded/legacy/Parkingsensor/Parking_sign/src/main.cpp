#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void renderParkingStatus(int count);

#define SDA_PIN 8
#define SCL_PIN 9
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define SCREEN_ADDRESS 0x3C 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {

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
  
  delay(2000); 
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