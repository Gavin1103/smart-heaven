#include <ESP32Servo.h>

/*
  Railway Barrier System - Smart City Prototype

  This system simulates a railway crossing using:
  - Ultrasonic sensor (automatic train detection)
  - Push button (manual override)
  - Servo motor (barrier movement)
  - LED lights (visual warning)
  - Buzzer (audible warning)

  The system is controlled using a Finite State Machine (FSM).
*/

Servo barrierServo;

// -------------------- FSM States --------------------
enum BarrierState {
  IDLE,         // Barrier open, no train detected
  APPROACHING,  // Train detected or manual close requested
  CLOSING,      // Barrier is closing
  CLOSED,       // Barrier fully closed
  OPENING,      // Barrier is opening
  ERROR_STATE   // System error
};

// -------------------- Pin Configuration --------------------
const int manualSwitchPin = 38;

const int buzzerPin = 4;

const int trigPin = 5;
const int echoPin = 6;

const int redLightPin = 2;
const int whiteLightPin = 1;
const int barrierServoPin = 18;

// -------------------- Barrier Settings --------------------
const int barrierOpenAngle = 0;
const int barrierClosedAngle = 90;

const int barrierMoveDelay = 60;
const int blinkInterval = 500;

// -------------------- Detection Settings --------------------
const float detectionThresholdCm = 10.0;
const unsigned long requiredDetectTimeMs = 3000;

// -------------------- System State --------------------
BarrierState currentState = IDLE;

unsigned long detectStartTime = 0;
bool detectTimerRunning = false;

bool lastButtonState = HIGH;

// -------------------- Helper Functions --------------------

// Prints current FSM state
void printState(BarrierState state) {
  switch (state) {
    case IDLE: Serial.println("State: IDLE"); break;
    case APPROACHING: Serial.println("State: APPROACHING"); break;
    case CLOSING: Serial.println("State: CLOSING"); break;
    case CLOSED: Serial.println("State: CLOSED"); break;
    case OPENING: Serial.println("State: OPENING"); break;
    case ERROR_STATE: Serial.println("State: ERROR"); break;
  }
}

// Turns both lights OFF
void turnLightsOff() {
  digitalWrite(redLightPin, LOW);
  digitalWrite(whiteLightPin, LOW);
}

// Turns both lights ON
void turnLightsOn() {
  digitalWrite(redLightPin, HIGH);
  digitalWrite(whiteLightPin, HIGH);
}

// Turns buzzer ON
void buzzerOn() {
  digitalWrite(buzzerPin, HIGH);
}

// Turns buzzer OFF
void buzzerOff() {
  digitalWrite(buzzerPin, LOW);
}

// Reads distance from ultrasonic sensor
float getDistanceCm() {
  long duration;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return 999.0;
  }

  return duration * 0.034 / 2.0;
}

// Returns true if object is within detection range
bool isTrainDetected() {
  float distance = getDistanceCm();
  return (distance > 0 && distance < detectionThresholdCm);
}

// Returns true if detection is stable for a defined time
bool isStableDetection(bool detected) {
  if (detected) {
    if (!detectTimerRunning) {
      detectStartTime = millis();
      detectTimerRunning = true;
    }

    if (millis() - detectStartTime >= requiredDetectTimeMs) {
      detectTimerRunning = false;
      return true;
    }
  } else {
    detectTimerRunning = false;
  }

  return false;
}

// Returns true only once when the button is pressed
bool isManualOverridePressed() {
  bool currentButtonState = digitalRead(manualSwitchPin);
  bool pressed = (lastButtonState == HIGH && currentButtonState == LOW);
  lastButtonState = currentButtonState;
  return pressed;
}

// -------------------- Setup --------------------
void setup() {
  pinMode(manualSwitchPin, INPUT_PULLUP);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(redLightPin, OUTPUT);
  pinMode(whiteLightPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  barrierServo.attach(barrierServoPin);

  Serial.begin(115200);

  turnLightsOff();
  buzzerOff();
  barrierServo.write(barrierOpenAngle);

  printState(currentState);
}

// -------------------- Main Loop --------------------
void loop() {
  bool trainDetected = isTrainDetected();
  bool manualOverridePressed = isManualOverridePressed();

  switch (currentState) {

    case IDLE:
      // System idle: barrier open, no warnings active
      turnLightsOff();
      buzzerOff();
      barrierServo.write(barrierOpenAngle);

      // Automatic trigger by sensor
      if (isStableDetection(trainDetected)) {
        Serial.println("Automatic trigger: train detected");
        currentState = APPROACHING;
        printState(currentState);
      }

      // Manual trigger by push button
      if (manualOverridePressed) {
        Serial.println("Manual override: closing requested");
        currentState = APPROACHING;
        printState(currentState);
      }
      break;

    case APPROACHING:
      // Warning phase before barrier closes
      for (int i = 0; i < 5; i++) {
        turnLightsOn();
        buzzerOn();
        delay(blinkInterval);

        turnLightsOff();
        buzzerOff();
        delay(blinkInterval);
      }

      currentState = CLOSING;
      printState(currentState);
      break;

    case CLOSING: {
      int blinkCounter = 0;
      bool whiteState = false;
      bool buzzerState = false;

      for (int pos = barrierOpenAngle; pos <= barrierClosedAngle; pos++) {
        barrierServo.write(pos);
        delay(barrierMoveDelay);

        blinkCounter += barrierMoveDelay;

        if (blinkCounter >= blinkInterval) {
          whiteState = !whiteState;
          buzzerState = !buzzerState;
          blinkCounter = 0;
        }

        digitalWrite(redLightPin, HIGH);
        digitalWrite(whiteLightPin, whiteState);
        digitalWrite(buzzerPin, buzzerState);
      }

      currentState = CLOSED;
      printState(currentState);
      break;
    }

    case CLOSED:
      // Barrier closed: both lights on, buzzer off
      turnLightsOn();
      buzzerOff();
      barrierServo.write(barrierClosedAngle);

      // Automatic trigger by sensor
      if (isStableDetection(trainDetected)) {
        Serial.println("Automatic trigger: opening requested");
        currentState = OPENING;
        printState(currentState);
      }

      // Manual trigger by push button
      if (manualOverridePressed) {
        Serial.println("Manual override: opening requested");
        currentState = OPENING;
        printState(currentState);
      }
      break;

    case OPENING: {
      int blinkCounter = 0;
      bool redState = false;

      for (int pos = barrierClosedAngle; pos >= barrierOpenAngle; pos--) {
        barrierServo.write(pos);
        delay(barrierMoveDelay);

        blinkCounter += barrierMoveDelay;

        if (blinkCounter >= blinkInterval) {
          redState = !redState;
          blinkCounter = 0;
        }

        digitalWrite(redLightPin, redState);
        digitalWrite(whiteLightPin, HIGH);
      }

      currentState = IDLE;
      printState(currentState);
      break;
    }

    case ERROR_STATE:
      buzzerOff();
      digitalWrite(redLightPin, HIGH);
      digitalWrite(whiteLightPin, LOW);
      delay(200);

      buzzerOn();
      digitalWrite(redLightPin, LOW);
      digitalWrite(whiteLightPin, HIGH);
      delay(200);
      break;
  }

  delay(100);
}