/*   opticalEncoder Example

      Test Program for Broadcom Incremental Encoder 500 ppr, AEDB-9140-A13 optical encoder

      Optical Encoder:
      https://uk.rs-online.com/web/p/motion-control-sensors/7967806

      Datasheet:
      https://docs.rs-online.com/34ce/0900766b812cdcb0.pdf



     Test software aims to count "Ticks" and Direction to prove hardware is functional.

     Can be expanded to return RPM of motor, and possibly absolute position in (degrees) given a calibration for starting position.

      Imogen Heard 25/04/2022

      Test for Supervisor PCB for practicable.io remote Lab Experiments

      #define ENCODER_B           2
      #define ENCODER_A           3
      #define ENCODER_INDEX       4


*/

/*  Encoder Specifics:

     1000 ppt encoder

     Therefore rotaryCount will be init at 1000, and its range constrained to 1000 +- 500
     to track absolute position



*/


#ifndef encoderFunctions_h
#define encoderFunctions_h

//#include <autoDelay.h>

#include "opticalEncoder.h"

#define ENCODER_B           3
#define ENCODER_A           2
#define ENCODER_INDEX       4

#define PULSES_PER_REVOLUTION 1000
opticalEncoder OE(ENCODER_INDEX, ENCODER_A, ENCODER_B);


// isr just calls the isr from inside the library
void isr() {
  OE.ISR();
}


void encoderSetup() {
  //  Serial.begin(115200);
  OE.encoderBegin(PULSES_PER_REVOLUTION);
  attachInterrupt (digitalPinToInterrupt (ENCODER_A), isr , CHANGE);   // NOTE DO NOT CALL pinMode() AFTER ATTACHING INTERRUPT> IT STOPS IT WORKING interrupt 0 is pin 2
  OE.encoderCalibrate();
  OE.plotHeader();
}


void encoderLoop(bool active = false) {                     // Order of operations is important here
  if (active) {
    OE.encoderDirection();                  // Handles direction and rotaryCount update after trigger by ISR
    OE.calcHeading();             // calculates heading angle in milliDegrees
    OE.calcRPM();                             // Uses milliDegrees traveled and period to calculate the RPM
    OE.plotEncoder();                         // Plot all the values to the serial monitor  #TODO: Update to cover new maths outputs and include direction bool clockwise. In begin we can have user setting to change polarity of bool clockwise incase up is reversed
    OE.resetFlag();                          // reset fired now that all maths have been finished #TODO: change bool from fired to ensure ticks are not missed due to maths taking to long. now encode direction sets a different bool flag
    //  OE.testInputs();
  }
}

#endif

