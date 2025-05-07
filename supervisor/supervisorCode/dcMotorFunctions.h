/*dcMotorFunctions.h

    Safety functions for DC motor

    Anisha Burgul
    27/04/25
*/
#ifndef dcMotorFunctions_h
#define dcMotorFunctions_h

// Include necessary headers
#include "globals.h"
#include "supervisor_pinMap.h"
#include "encoderFunctions.h"

// Constants for motor safety and monitoring
#define EXCESSIVE_RUNTIME_MS 300000             // Max allowed continuous runtime (5 min)
#define OVERCURRENT_THRESHOLD_MA 2000           // Overcurrent detection threshold
#define CURRENT_CHECK_INTERVAL_MS 100           // Interval for checking motor current
#define STALL_CURRENT_THRESHOLD_MA 100          // Min current to consider motor stalled
#define STALL_RPM_THRESHOLD 5                   // RPM below which the motor may be stalled
#define STALL_DETECTION_MS 500                  // Time window to confirm a stall
#define TOGGLE_WINDOW_MS 1000                   // Time window to monitor toggle frequency
#define MAX_TOGGLES_ALLOWED 4                   // Max allowed toggles in above window
#define IS_OFFSET_uA 170                        // Offset in current sensor in microamps
#define R_IS 470.0                              // Sense resistor value in ohms

// Variables to track motor operation
unsigned long motorStartTime = 0;
unsigned long lastCurrentCheck = 0;
unsigned long stallStartTime = 0;

// Flag set by direction change interrupt
volatile bool dirChanged = false;

// Store last direction state
int lastDir1 = LOW, lastDir2 = LOW;

// Interrupt service routines to detect direction changes
void IRAM_ATTR dir1ISR() {
  dirChanged = true;
}

void IRAM_ATTR dir2ISR() {
  dirChanged = true;
}

// Initialize motor direction pins and attach interrupts
void dcMotorBegin() {
  pinMode(STDNT_DC_M_DIR_1, INPUT_PULLUP);
  pinMode(STDNT_DC_M_DIR_2, INPUT_PULLUP);
  pinMode(MOTOR_DIR_1, OUTPUT);
  pinMode(MOTOR_DIR_2, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(STDNT_DC_M_DIR_1), dir1ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(STDNT_DC_M_DIR_2), dir2ISR, CHANGE);
}

// Emergency stop: disable motor and print message
void stopMotor() {
  Serial.println("Emergency Stop triggered!");
  shiftReg.shiftWrite(STUDENT_M_EN, LOW);
  shiftReg.shiftWrite(DC_MOTOR_EN, LOW);
  digitalWrite(MOTOR_DIR_1, LOW);
  digitalWrite(MOTOR_DIR_2, LOW);
}

// Convert raw analog value to voltage (3.3V scale)
float analogToVoltage(int val) {
  return val * (3.3 / 1024.0);
}

// Convert voltage reading to current in milliamps
float voltageToCurrent_mA(float voltage) {
  float senseCurrent_mA = (voltage / R_IS) * 1000.0;
  float offset = IS_OFFSET_uA / 1000.0;
  return max(senseCurrent_mA - offset, 0.0);
}

// Get the higher current reading from two sense lines
float getMotorCurrent_mA() {
  float v1 = analogToVoltage(analogRead(DC_MOTOR_IS1));
  float v2 = analogToVoltage(analogRead(DC_MOTOR_IS2));
  float current1 = voltageToCurrent_mA(v1);
  float current2 = voltageToCurrent_mA(v2);
  return max(current1, current2);
}

// Check if the motor has been running too long in one direction
void checkExcessiveRuntime(bool directionActive) {
  unsigned long now = millis();
  if (directionActive) {
    if (motorStartTime == 0) motorStartTime = now;
    else if (!motorDisabledDueToRuntime && (now - motorStartTime > EXCESSIVE_RUNTIME_MS)) {
      motorDisabledDueToRuntime = true;
      stopMotor();
      Serial.println("Runtime limit exceeded. Motor disabled.");
    }
  } else {
    motorStartTime = 0;
    if (motorDisabledDueToRuntime) {
      motorDisabledDueToRuntime = false;
      shiftReg.shiftWrite(STUDENT_M_EN, HIGH);
      shiftReg.shiftWrite(DC_MOTOR_EN, HIGH);
      Serial.println("Runtime recovered. Student control re-enabled.");
    }
  }
}

// Monitor motor current for overcurrent condition
void checkOvercurrent() {
  if (millis() - lastCurrentCheck < CURRENT_CHECK_INTERVAL_MS) return;
  lastCurrentCheck = millis();

  float current = getMotorCurrent_mA();
  if (!motorDisabledDueToCurrent && current > OVERCURRENT_THRESHOLD_MA) {
    motorDisabledDueToCurrent = true;
    stopMotor();
    Serial.println("Overcurrent detected!");
  } else if (motorDisabledDueToCurrent && current < 500) {
    motorDisabledDueToCurrent = false;
    shiftReg.shiftWrite(STUDENT_M_EN, HIGH);
    shiftReg.shiftWrite(DC_MOTOR_EN, HIGH);
    Serial.println("Current normalized. Student control re-enabled.");
  }
}

// Detect stall: high current but very low RPM
void checkMotorStall() {
  float current_mA = getMotorCurrent_mA();
  int rpm = OE.getRPM();

  bool isMoving = rpm > STALL_RPM_THRESHOLD;
  bool isDrawingCurrent = current_mA > STALL_CURRENT_THRESHOLD_MA;

  if (isDrawingCurrent && !isMoving) {
    if (stallStartTime == 0) stallStartTime = millis();
    else if (!motorDisabledDueToStall && millis() - stallStartTime > STALL_DETECTION_MS) {
      motorDisabledDueToStall = true;
      stopMotor();
      Serial.println("Stall detected: Current but no movement.");
    }
  } else {
    stallStartTime = 0;
    if (motorDisabledDueToStall && !isDrawingCurrent) {
      motorDisabledDueToStall = false;
      shiftReg.shiftWrite(STUDENT_M_EN, HIGH);
      shiftReg.shiftWrite(DC_MOTOR_EN, HIGH);
      Serial.println("Stall cleared. Student control re-enabled.");
    }
  }
}

// Monitor and mirror student direction commands, with safety checks
void checkAndHandleDirection() {
  int dc_motor_cw = digitalRead(STDNT_DC_M_DIR_1);
  int dc_motor_acw = digitalRead(STDNT_DC_M_DIR_2);

  // Both directions active is invalid
  if (dc_motor_cw && dc_motor_acw) {
    stopMotor();
    Serial.println("Invalid direction (both HIGH) detected. Student blocked.");
    return;
  }

  bool changed = (dc_motor_cw != lastDir1) || (dc_motor_acw != lastDir2);
  bool directionActive = dc_motor_cw || dc_motor_acw;
  checkExcessiveRuntime(directionActive);

  // Check for rapid direction toggling
  if (changed) {
    unsigned long now = millis();
    static unsigned long toggleTimes[10];
    static int toggleIndex = 0;

    toggleTimes[toggleIndex] = now;
    toggleIndex = (toggleIndex + 1) % 10;

    int togglesInWindow = 0;
    for (int i = 0; i < 10; i++) {
      if (now - toggleTimes[i] <= TOGGLE_WINDOW_MS) togglesInWindow++;
    }

    if (togglesInWindow > MAX_TOGGLES_ALLOWED) {
      Serial.println("Too many direction changes detected!");
      stopMotor();
      return;
    }

    lastDir1 = dc_motor_cw;
    lastDir2 = dc_motor_acw;
  }

  // No direction inputs: motor idle
  if (!dc_motor_cw && !dc_motor_acw) {
    shiftReg.shiftWrite(STUDENT_M_EN, HIGH);
    digitalWrite(MOTOR_DIR_1, LOW);
    digitalWrite(MOTOR_DIR_2, LOW);
    Serial.println("Student control re-enabled (idle).");
    return;
  }

  // Normal operation: mirror direction to motor
  shiftReg.shiftWrite(STUDENT_M_EN, HIGH);
  digitalWrite(MOTOR_DIR_1, dc_motor_cw);
  digitalWrite(MOTOR_DIR_2, dc_motor_acw);
  Serial.println("Mirroring student direction.");
}

#endif