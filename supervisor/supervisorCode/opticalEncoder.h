/* opticalEncoder.cpp

     Small Library to handle Optical Encoders similar to:


    Broadcom Incremental Encoder 500 ppr, AEDB-9140-A13 optical encoder

    Optical Encoder:
    https://uk.rs-online.com/web/p/motion-control-sensors/7967806

    Datasheet:
    https://docs.rs-online.com/34ce/0900766b812cdcb0.pdf

          Software aims to count individual ticks, absolute postion (degrees),
          and track direction and velocity of the optical encoder wheel.


     Imogen Wren
     01/06/2022

     
#define ENCODER_B           2
#define ENCODER_A           3
#define ENCODER_INDEX       4



*/

/*  Encoder Specifics:

     1000 ppt encoder

     Therefore rotaryCount will be init at 1000, and its range constrained to 1000 +- 500
     to track absolute position



*/



#ifndef opticalEncoder_h
#define opticalEncoder_h


#if (ARDUINO >=100)
#include <Arduino.h>
#else
#include <wProgram.h>
#endif



class opticalEncoder {

  public:


    // constructor
    opticalEncoder(int16_t index_pin = 4, int16_t pin_A = 3, int16_t pin_B = 2);

    void encoderBegin(int16_t pulsePerRevolution = 1000);

    void encoderDirection();     //updates rotaryCount and determines direction of encoder

    void calcHeading();         // Function to return the current position of the encoder in degrees from center position (1000)  (-180 to +180)deg

    void plotHeader();      // Plots a header to serial monitor

    void plotEncoder();     // Plots rotary count, headingString and RPM to the serial monitor

    void testPlot(int16_t A, int16_t B, int16_t C);     // prints absolute values passed as arguments

    void testInputs();     // digitalReads and prints the state of the optical encoder pins for testing

    void calcRPM();          // Calculates RPM by taking the time between 2 samples and extrapolating that to an entire revolution

    void encoderCalibrate();


    void ISR();            // Interrupt service routine triggered by change on optical encoder pin.

    void resetFlag();      // taken reset fired outside of method it was in, so fired can act as trigger for many of the maths functions and then be reset once all new maths is done



    bool direction;
    bool clockwiseRotation;

    int getRPM() { return rpm; }



  private:

    void floatToChar(float f_input);      // returns float input as global char string headingChar
    int32_t heading_milliDeg;
    int32_t last_heading_milliDeg;
    char headingString[32];

    float degPerPulse;                     // save the number of degrees per pulse of the encoder.
    uint32_t milliDegPerPulse;               // Actually going to save it as degree *1000 to avoid float maths


    uint16_t rotaryCount;
    uint16_t prevCount;

    uint16_t ppr;             // pulses per revolution
    uint16_t upperBound;
    uint16_t lowerBound;


    int16_t rpm;

    bool up;
    bool fired = false;

    // Timing movement
    uint32_t sampleMicros;
    uint32_t lastSampleMicros;

    // Hardware Pins
    int16_t  indexPin;
    int16_t  pinA;
    int16_t  pinB;

    bool calcsComplete; //internal flag to trigger fired reset
    bool encoderUpdated;   // internal flag for maths functions to run caculations on new data.


};

































#endif