/*stepperFunctions.h

    Safety functions for servo motor

    Anisha Burgul
    27/04/25
*/
#ifndef stepperFunctions_h
#define stepperFunctions_h

#include "globals.h"
#include "supervisor_pinMap.h"
#include <autoDelay.h>

autoDelay stepPulse;
bool stepPulseState;

volatile bool stepperFaultDetected = false;
volatile bool limitSwitchTriggered = false;

// --- ISR Handlers ---
void stepperFaultISR() {
  if (!digitalRead(STEPP_FLT)) {  // LOW = fault
    stepperFaultDetected = true;
  }
}

void limitSwitchISR() {
  if (!digitalRead(LIMIT_1)) {    // LOW = limit switch pressed
    limitSwitchTriggered = true;
  }
}

// --- Stepper Setup ---
void stepperBegin() {
  pinMode(STEPP_STEP, OUTPUT);
  pinMode(STEPP_FLT, INPUT_PULLUP);
  pinMode(LIMIT_1, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(STEPP_FLT), stepperFaultISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(LIMIT_1), limitSwitchISR, FALLING);
}

// --- Stepper Sleep Mode Control ---
void stepperSleep(bool enable) {
  shiftReg.shiftWrite(STEPP_SLP, enable ? HIGH : LOW);
}

// --- Set Stepper Direction ---
void stepperDirection(bool clockwise = false) {
  shiftReg.shiftWrite(STEPP_DIR, clockwise);
}

// --- Reset Stepper Controller ---
void stepperReset(bool reset = false) {
  shiftReg.shiftWrite(STEPP_RST, reset);
}

// --- Pulse Stepper Motor (Supervisor Control for nudging of limit switch) ---
void stepperPulse(uint16_t pulseCount = 100, uint16_t pulseDelay_us = 500) {
  for (int i = 0; i < pulseCount; i++) {
    digitalWrite(STEPP_STEP, HIGH);
    delayMicroseconds(pulseDelay_us);
    digitalWrite(STEPP_STEP, LOW);
    delayMicroseconds(pulseDelay_us);
  }
}

void handleStepperFault() {
  if (!stepperDisabledDueToFault) {
    stepperDisabledDueToFault = true;
    shiftReg.shiftWrite(STUDENT_STEP_EN, LOW);
    stepperSleep(false);
    Serial.println("Stepper fault detected. Student control disabled.");
  }
}

void handleLimitSwitch() {
  shiftReg.shiftWrite(STUDENT_STEP_EN, LOW);  // Block student control
  stepperSleep(true);                         // Ensure driver active
  stepperDirection(true);                     // Force UP
  Serial.println("Limit switch hit. Nudging stepper off...");
  stepperPulse();                             // Supervisor nudges up
  shiftReg.shiftWrite(STUDENT_STEP_EN, HIGH); // Control back
  Serial.println("Control returned to student.");
}
#endif