/*servoFunctions.h

    Safety functions for servo motor

    Anisha Burgul
    27/04/25
*/

#ifndef servoFunctions_h
#define servoFunctions_h

#include <Servo.h>

// Pin definitions and constants
#define SERVO_OUT           9     // Output pin to control the servo
#define SERVO_IN            A2    // Input pin for student PPM signal
#define PPM_TIMEOUT_MS      2000  // Timeout duration before defaulting to safe position
#define PPM_MIN_US          1000   // Minimum valid PPM pulse width (in microseconds)
#define PPM_MAX_US          2000  // Maximum valid PPM pulse width (in microseconds)
#define PPM_TO_ANGLE_MIN    1000  // Minimum PPM used for angle mapping
#define PPM_TO_ANGLE_MAX    2000  // Maximum PPM used for angle mapping
#define SERVO_UPDATE_BLOCK_MS 20  // Minimum time between servo updates
#define SERVO_DEFAULT_ANGLE 90    // Default angle to set servo on startup and timeout

Servo servo;  // Servo object for PWM control

// Variables for PPM signal measurement
volatile unsigned long ppmRiseTime = 0;     // Time when rising edge detected
volatile unsigned long ppmPulseWidth = 0;   // Measured pulse width
volatile bool newPulse = false;             // Flag to indicate new pulse is ready

// Tracking timing and constraints
unsigned long lastPulseTime = 0;            // Last time a valid pulse was received
int16_t servoMin = 0;                       // Minimum allowed servo angle
int16_t servoMax = 180;                     // Maximum allowed servo angle
int16_t restingAngle = SERVO_DEFAULT_ANGLE; // Angle to return to on timeout

bool servoBlocked = false;                  // (Unused) Flag if servo is disabled
unsigned long lastUpdateTime = 0;           // Last time servo was updated

// Set servo angle constraints (called externally if needed)
void servoConstrain(int16_t minVal, int16_t maxVal) {
  servoMin = minVal;
  servoMax = maxVal;
}

// Interrupt service routine to capture PPM signal width
void servoPPM_ISR() {
  if (digitalRead(SERVO_IN)) {
    // Rising edge: store timestamp
    ppmRiseTime = micros();
  } else {
    // Falling edge: calculate pulse duration
    unsigned long now = micros();
    unsigned long duration = now - ppmRiseTime;

    // Validate and store if within expected range
    if (duration >= PPM_MIN_US && duration <= PPM_MAX_US) {
      ppmPulseWidth = duration;
      newPulse = true;
      lastPulseTime = millis();  // Update last received pulse time
    }
  }
}

// Safely update the servo angle with rate limiting
void servoUpdate(int angle) {
  if (millis() - lastUpdateTime >= SERVO_UPDATE_BLOCK_MS) {
    angle = constrain(angle, servoMin, servoMax);  // Enforce limits
    servo.write(angle);                            // Send angle to servo
    lastUpdateTime = millis();                     // Record update time
  }
}

// Detect and process incoming PPM signal, handle timeout fallback
void studentServoDetect() {
  if (newPulse) {
    // Safely read pulse data without interruption
    noInterrupts();
    unsigned long pulse = ppmPulseWidth;
    newPulse = false;
    interrupts();

    // Convert pulse width to angle and update servo
    int angle = map(pulse, PPM_TO_ANGLE_MIN, PPM_TO_ANGLE_MAX, servoMin, servoMax);
    servoUpdate(angle);
  }

  // If too long since last valid pulse, reset to safe angle
  if (millis() - lastPulseTime > PPM_TIMEOUT_MS) {
    servoUpdate(restingAngle);
  }
}

// Initialize servo hardware and ISR
void servoBegin() {
  servo.attach(SERVO_OUT);  // Attach servo to output pin
  pinMode(SERVO_IN, INPUT); // Set PPM input pin
  attachInterrupt(digitalPinToInterrupt(SERVO_IN), servoPPM_ISR, CHANGE);  // Attach ISR
  servo.write(SERVO_DEFAULT_ANGLE);  // Set servo to default angle
  lastPulseTime = millis();          // Initialize timing
}

#endif