/*globals.h

  Global variables

  Anisha Burgul
  27/04/25

*/

#ifndef globals_h
#define globals_h

#include "shiftRegister.h"

// --- Global Shift Register Object ---
extern shiftRegister shiftReg;

// --- Shared Flags for DC Motor ---
extern bool motorDisabledDueToRuntime;
extern bool motorDisabledDueToCurrent;
extern bool motorDisabledDueToStall;

// --- Shared Flags for Stepper Motor ---
extern bool stepperDisabledDueToFault;
extern bool stepperDisabledDueToLimit;

#endif

