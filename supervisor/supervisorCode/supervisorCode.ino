/* supervisorCode.ino.h 

    Code for supervisor MCU

    Anisha Burgul
    27/04/25
*/ 

#include "globals.h"
#include "shiftRegister.h"
#include "encoderFunctions.h"
#include "dcMotorFunctions.h"
#include "stepperFunctions.h"
#include "servoFunctions.h"


// --- Global Shift Register Instance ---
shiftRegister shiftReg;

// --- DC Motor Flags ---
bool motorDisabledDueToRuntime = false;
bool motorDisabledDueToCurrent = false;
bool motorDisabledDueToStall = false;

// --- Stepper Motor Flags ---
bool stepperDisabledDueToFault = false;
bool stepperDisabledDueToLimit = false;


void setup() {
  Serial.begin(115200);

  // Shift Register Setup
  shiftReg.begin();
  shiftReg.allOff();

  // Allow student control on startup
  shiftReg.shiftWrite(STUDENT_M_EN, HIGH);
  shiftReg.shiftWrite(STUDENT_STEP_EN, HIGH);

  // Encoder setup
  encoderSetup();

  // DC Motor setup
  dcMotorBegin();

  // Stepper setup
  stepperBegin();
  stepperSleep(false);      // Start asleep
  stepperReset(true);
  delay(10);
  stepperReset(false);

  // Servo setup
  servoBegin();
  servoConstrain(0, 180);  // Optional: Clamp angle range if desired

  Serial.println("Supervisor: Student control enabled.");
}

void loop() {
  // First: Stepper emergency checks
  if (stepperFaultDetected || limitSwitchTriggered) {
    noInterrupts();
    bool fault = stepperFaultDetected;
    bool limit = limitSwitchTriggered;
    stepperFaultDetected = false;
    limitSwitchTriggered = false;
    interrupts();

    if (fault) {
      handleStepperFault();   // new function: stop motor, disable student
    }
    if (limit) {
      handleLimitSwitch();    // new function: nudge stepper off limit
    }
  }

  // Second: DC motor emergency checks
  if (dirChanged) {
    noInterrupts();
    dirChanged = false;
    interrupts();
    checkAndHandleDirection();
  }

  // Third: normal background checks
  checkOvercurrent();
  checkMotorStall();
  encoderLoop(true);
  studentServoDetect();

  delay(1);
}


