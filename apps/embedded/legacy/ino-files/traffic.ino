#include <Arduino.h>
#include <vector>
int latchPin = 7;   // ST_CP
int clockPin = 6;   // SH_CP
int dataPin  = 5;   // DS
int tonePin  = 15;
int red = 40;
int orange = 41;
int green = 42;
int delayGreenToYellow = 3000;
int delayYellowToRed = 1000;
int RedCleantime = 700;

unsigned long previousMillis = 0;
void trafficN() ;
void trafficZ() ;
void trafficW() ;
void playMelody() ;
void pad() ; 

byte led = 0;

int melody[] = {
  659, 494, 523, 587,
  523, 494, 440, 440,
  523, 659, 587, 523,
  494, 523, 587, 659,
  523, 440, 440
};

int duration[] = {
  4, 8, 8, 4,
  8, 8, 4, 8,
  8, 4, 8, 8,
  4, 8, 8, 4,
  4, 4, 4
};

void setup() {

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(tonePin, OUTPUT);
  pinMode(green,OUTPUT);
  pinMode(orange, OUTPUT);
  pinMode(red, OUTPUT);
  Serial.begin(115200);
  m
}

void loop() {
 trafficN();
trafficZ();
trafficW();
 pad();
}


void pad(){
led = (1 << 1) | (1 << 4);

digitalWrite(latchPin, LOW);
shiftOut(dataPin, clockPin, MSBFIRST, led);
digitalWrite(latchPin, HIGH);

playMelody();
}
void trafficN() {

  for (int i = 0; i <= 3; i++) {

   led = (1 << i) | (1 << 4);   // zet steeds volgende LED aan

    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, led);
    digitalWrite(latchPin, HIGH);
    Serial.print("hall0");
    if (i == 2){
      delay(delayGreenToYellow);
    } else if (i == 3){
      delay(delayYellowToRed);
    }else{
      delay(1000);
    }
   
  }
}
void trafficZ() {

  for (int i = 4; i <= 6; i++) {

   led = (1 << i) | (1 << 1);   // zet steeds volgende LED aan

    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, led);
    digitalWrite(latchPin, HIGH);
    if (i == 5){
      delay(delayGreenToYellow);
    } else if (i == 6){
      delay(delayYellowToRed);
    }
  }
}
 void trafficW(){

for (int i = 1; i <= 3; i++ ){
  led = (1<< 4) | (1 << 1) | (1 << 0);   // zet steeds volgende LED aan

    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, led);
    digitalWrite(latchPin, HIGH);
  
  if (i == 1){
    digitalWrite(green, HIGH);
      delay(delayGreenToYellow);
    } else if (i == 2){
      digitalWrite(green, LOW);
      digitalWrite(orange, HIGH);
      delay(delayYellowToRed);
    }else {
      digitalWrite(orange, LOW);
      digitalWrite(red, HIGH);
      delay(RedCleantime);

    }

   
  }
}
void playMelody() {
  int length = sizeof(melody) / sizeof(melody[0]);

  for (int i = 0; i < length; i++) {
    int noteDuration = 1000 / duration[i];

    if (melody[i] == 0) {
      noTone(tonePin);
    } else {
      tone(tonePin, melody[i], noteDuration);
    }

    int pause = noteDuration * 1.3;
    delay(pause);
    noTone(tonePin);
  }
}

void lichts() {
  for (int i = 0; i < 8; i++) {
    led = 1 << i;   // zet steeds volgende LED aan

    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, led);
    digitalWrite(latchPin, HIGH);
  }
}