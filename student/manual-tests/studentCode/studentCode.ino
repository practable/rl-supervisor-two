#include "shiftRegister.h"

// --------------------
// 🔧 Pin Definitions
// --------------------
#define Q0 0b00000001  // DC_MOTOR_EN
#define Q2 0b00000100  // STEP_EN
#define Q4 0b00010000  // DIR
#define Q5 0b00100000  // SLP
#define Q6 0b01000000  // RST

#define DC_MOTOR_DIR1 12
#define DC_MOTOR_DIR2 7
#define STEPP_STEP    6
#define STEPP_FLT     A5
#define LIMIT_1       A4
#define STUDENT_SERVO_OUT 9
#define PWM_THING     10

shiftRegister shiftReg;
const int pulseDelay = 500;

void setup() {
  Serial.begin(115200);
  shiftReg.begin();
  shiftReg.allOff();

  pinMode(DC_MOTOR_DIR1, OUTPUT);
  pinMode(DC_MOTOR_DIR2, OUTPUT);
  pinMode(STEPP_STEP, OUTPUT);
  pinMode(STUDENT_SERVO_OUT, OUTPUT);
  pinMode(STEPP_FLT, INPUT);
  pinMode(LIMIT_1, INPUT);

  digitalWrite(DC_MOTOR_DIR1, LOW);
  digitalWrite(DC_MOTOR_DIR2, LOW);
  digitalWrite(STEPP_STEP, LOW);
  digitalWrite(STUDENT_SERVO_OUT, LOW);

  Serial.println("Enter a test number (0-29) in the Serial Monitor to run it.");
}

void loop() {
  if (Serial.available()) {
    int test = Serial.parseInt();
    if (test >= 0 && test <= 29) {
      clearAll();
      Serial.print("Starting Test ");
      Serial.println(test);

      switch (test) {
        case 0: case0(); break;
        case 1: case1(); break;
        case 2: case2(); break;
        case 3: case3(); break;
        case 4: case4(); break;
        case 5: case5(); break;
        case 6: case6(); break;
        case 7: case7(); break;
        case 8: case8(); break;
        case 9: case9(); break;
        case 10: case10(); break;
        case 11: case11(); break;
        case 12: case12(); break;
        case 13: case13(); break;
        case 14: case14(); break;
        case 15: case15(); break;
        case 16: case16(); break;
        case 17: case17(); break;
        case 18: case18(); break;
        case 19: case19(); break;
        case 20: case20(); break;
        case 21: case21(); break;
        case 22: case22(); break;
        case 23: case23(); break;
        case 24: case24(); break;
        case 25: case25(); break;
        case 26: case26(); break;
        case 27: case27(); break;
        case 28: case28(); break;
        case 29: case29(); break;
      }
    } else {
      Serial.println("Invalid test number. Enter 0-29.");
    }

    while (Serial.available()) Serial.read();
  }
}

void clearAll() {
  shiftReg.allOff();
  digitalWrite(DC_MOTOR_DIR1, LOW);
  digitalWrite(DC_MOTOR_DIR2, LOW);
  digitalWrite(STEPP_STEP, LOW);
  digitalWrite(STUDENT_SERVO_OUT, LOW);
}

void stepPulseSlow(int steps) {
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEPP_STEP, HIGH);
    delayMicroseconds(pulseDelay);
    digitalWrite(STEPP_STEP, LOW);
    delayMicroseconds(pulseDelay);
  }
}



void clearMotor() {
  shiftReg.shiftWrite(Q0, LOW);
  digitalWrite(DC_MOTOR_DIR1, LOW);
  digitalWrite(DC_MOTOR_DIR2, LOW);
}

// ---------------------
//  Test Cases
// ---------------------
void case0() {
  clearMotor();
  Serial.println("[Test 0] Motor OFF");
}

void case1() {
  clearMotor();
  shiftReg.shiftWrite(Q0, HIGH);
  Serial.println("[Test 1] Motor ENABLED, no direction");
}

void case2() {
  clearMotor();
  digitalWrite(DC_MOTOR_DIR1, HIGH);
  digitalWrite(DC_MOTOR_DIR2, LOW);
  shiftReg.shiftWrite(Q0, HIGH);
  Serial.println("[Test 2] Clockwise (DIR1 HIGH, DIR2 LOW)");
}

void case3() {
  shiftReg.shiftWrite(Q0, HIGH);
  digitalWrite(DC_MOTOR_DIR1, LOW);
  digitalWrite(DC_MOTOR_DIR2, HIGH);
  Serial.println("[Test 3] Counter-Clockwise (DIR1 LOW, DIR2 HIGH)");
}

void case4() {
  shiftReg.shiftWrite(Q0, HIGH);
  digitalWrite(DC_MOTOR_DIR1, HIGH);
  digitalWrite(DC_MOTOR_DIR2, HIGH);
  Serial.println("[Test 4] ILLEGAL: Both DIR1 & DIR2 HIGH");
}

void case5() {
  shiftReg.shiftWrite(Q0, LOW);
  digitalWrite(DC_MOTOR_DIR1, HIGH);
  digitalWrite(DC_MOTOR_DIR2, LOW);
  Serial.println("[Test 5] Motor OFF, DIR set to CW");
}

void case6() {
  shiftReg.shiftWrite(Q0, LOW);
  digitalWrite(DC_MOTOR_DIR1, LOW);
  digitalWrite(DC_MOTOR_DIR2, HIGH);
  Serial.println("[Test 6] Motor OFF, DIR set to CCW");
}

void case7() {
  shiftReg.shiftWrite(Q0, LOW);
  digitalWrite(DC_MOTOR_DIR1, HIGH);
  digitalWrite(DC_MOTOR_DIR2, HIGH);
  Serial.println("[Test 7] ILLEGAL: Motor OFF, DIR1 & DIR2 HIGH");
}

void case8() {
  shiftReg.shiftWrite(Q0, HIGH);
  for (int i = 0; i < 5; i++) {
    digitalWrite(DC_MOTOR_DIR1, HIGH);
    digitalWrite(DC_MOTOR_DIR2, LOW);
    delay(100);
    digitalWrite(DC_MOTOR_DIR1, LOW);
    digitalWrite(DC_MOTOR_DIR2, HIGH);
    delay(100);
  }
  Serial.println("[Test 8] Rapid toggle between CW and CCW");
}

void case9() {
  digitalWrite(DC_MOTOR_DIR1, HIGH);
  for (int i = 0; i < 5; i++) {
    shiftReg.shiftWrite(Q0, HIGH);
    delay(100);
    shiftReg.shiftWrite(Q0, LOW);
    delay(100);
  }
  Serial.println("[Test 9] Rapid motor enable ON/OFF");
}

void case10() {
  for (int i = 0; i < 5; i++) {
    shiftReg.shiftWrite(Q0, HIGH);
    digitalWrite(DC_MOTOR_DIR1, HIGH);
    digitalWrite(DC_MOTOR_DIR2, LOW);
    delay(75);
    shiftReg.shiftWrite(Q0, LOW);
    digitalWrite(DC_MOTOR_DIR1, LOW);
    digitalWrite(DC_MOTOR_DIR2, HIGH);
    delay(75);
  }
  Serial.println("[Test 10] Rapid toggling all motor states");
}

void case11() {
  shiftReg.shiftWrite(Q0, HIGH);
  digitalWrite(DC_MOTOR_DIR1, HIGH);
  digitalWrite(DC_MOTOR_DIR2, LOW);
  delay(4000);
  Serial.println("[Test 11] CW direction - excessive runtime");
}

void case12() {
  shiftReg.shiftWrite(Q0, HIGH);
  digitalWrite(DC_MOTOR_DIR1, HIGH);
  digitalWrite(DC_MOTOR_DIR2, LOW);
  delay(2000);
  shiftReg.shiftWrite(Q0, LOW);
  digitalWrite(DC_MOTOR_DIR1, LOW);
  Serial.println("[Test 12] CW direction - safe runtime");
}

void case13() {
  shiftReg.shiftWrite(Q0, HIGH);
  digitalWrite(DC_MOTOR_DIR1, LOW);
  digitalWrite(DC_MOTOR_DIR2, HIGH);
  delay(4000);
  clearMotor();
  delay(1500);
  shiftReg.shiftWrite(Q0, HIGH);
  digitalWrite(DC_MOTOR_DIR1, HIGH);
  Serial.println("[Test 13] Recover after excessive runtime");
}

void case14() {
  shiftReg.shiftWrite(Q0, HIGH);
  digitalWrite(DC_MOTOR_DIR1, HIGH);
  digitalWrite(DC_MOTOR_DIR2, LOW);
  Serial.println("[Test 14] Stall motor: Please HOLD the motor shaft to prevent spinning");
  delay(3000);
  clearMotor();
}

void case15() {
  shiftReg.shiftWrite(Q0, HIGH);
  digitalWrite(DC_MOTOR_DIR1, HIGH);
  digitalWrite(DC_MOTOR_DIR2, LOW);
  Serial.println("[Test 15] Overcurrent: Please simulate high current draw");
  delay(3000);
  clearMotor();
}

void case16() {
  shiftReg.shiftWrite(Q4, HIGH);
  shiftReg.shiftWrite(Q5, HIGH);
  shiftReg.shiftWrite(Q6, HIGH);
  Serial.println("[Test 16] Stepper CW - DIR=HIGH");
  for (int i = 0; i < 100; i++) {
    digitalWrite(6, HIGH);
    delayMicroseconds(500);
    digitalWrite(6, LOW);
    delayMicroseconds(500);
  }
}

void case17() {
  shiftReg.shiftWrite(Q4, LOW);
  shiftReg.shiftWrite(Q5, HIGH);
  shiftReg.shiftWrite(Q6, HIGH);
  Serial.println("[Test 17] Stepper CCW - DIR=LOW");
  for (int i = 0; i < 100; i++) {
    digitalWrite(6, HIGH);
    delayMicroseconds(500);
    digitalWrite(6, LOW);
    delayMicroseconds(500);
  }
}

void case18() {
  shiftReg.shiftWrite(Q4, HIGH);
  shiftReg.shiftWrite(Q5, HIGH);
  shiftReg.shiftWrite(Q6, HIGH);
  Serial.println("[Test 18] Stepper fault BEFORE motion - please pull A5 to GND");
  delay(3000);
}

void case19() {
  shiftReg.shiftWrite(Q4, LOW);
  shiftReg.shiftWrite(Q5, HIGH);
  shiftReg.shiftWrite(Q6, HIGH);
  Serial.println("[Test 19] Limit switch pressed BEFORE motion");
  delay(3000);
}

void case20() {
  shiftReg.shiftWrite(Q4, HIGH);
  shiftReg.shiftWrite(Q5, HIGH);
  shiftReg.shiftWrite(Q6, HIGH);
  Serial.println("[Test 20] Fault DURING motion");
  for (int i = 0; i < 100; i++) {
    digitalWrite(6, HIGH);
    delayMicroseconds(500);
    digitalWrite(6, LOW);
    delayMicroseconds(500);
  }
}

void case21() {
  shiftReg.shiftWrite(Q4, LOW);
  shiftReg.shiftWrite(Q5, HIGH);
  shiftReg.shiftWrite(Q6, HIGH);
  Serial.println("[Test 21] Limit switch DURING motion");
  for (int i = 0; i < 100; i++) {
    digitalWrite(6, HIGH);
    delayMicroseconds(500);
    digitalWrite(6, LOW);
    delayMicroseconds(500);
  }
}

void case22() {
  Serial.println("[Test 22] Recovery from fault");
  delay(2000);
  shiftReg.shiftWrite(Q4, HIGH);
  shiftReg.shiftWrite(Q5, HIGH);
  shiftReg.shiftWrite(Q6, HIGH);
  for (int i = 0; i < 100; i++) {
    digitalWrite(6, HIGH);
    delayMicroseconds(500);
    digitalWrite(6, LOW);
    delayMicroseconds(500);
  }
}

void case23() {
  Serial.println("[Test 23] Recovery from limit");
  delay(2000);
  shiftReg.shiftWrite(Q4, LOW);
  shiftReg.shiftWrite(Q5, HIGH);
  shiftReg.shiftWrite(Q6, HIGH);
  for (int i = 0; i < 100; i++) {
    digitalWrite(6, HIGH);
    delayMicroseconds(500);
    digitalWrite(6, LOW);
    delayMicroseconds(500);
  }
}

void sendServoPulse(int highTime_us, int duration_ms = 2000) {
  pinMode(STUDENT_SERVO_OUT, OUTPUT);
  unsigned long startTime = millis();
  while (millis() - startTime < duration_ms) {
    digitalWrite(STUDENT_SERVO_OUT, HIGH);
    delayMicroseconds(highTime_us);
    digitalWrite(STUDENT_SERVO_OUT, LOW);
    delayMicroseconds(20000 - highTime_us);
  }
}

void case24() {
  Serial.println("[Test 24] Servo to MIN position");
  sendServoPulse(1000);
}

void case25() {
  Serial.println("[Test 25] Servo to MID position");
  sendServoPulse(1500);
}

void case26() {
  Serial.println("[Test 26] Servo to MAX position");
  sendServoPulse(2000);
}

void case27() {
  Serial.println("[Test 27] Invalid PPM: too short");
  sendServoPulse(500);
}

void case28() {
  Serial.println("[Test 28] Invalid PPM: too long");
  sendServoPulse(2500);
}

void case29() {
  Serial.println("[Test 29] Rapid valid PPM updates");
  for (int i = 0; i < 10; i++) {
    sendServoPulse(1000 + (i % 2) * 1000, 100);
    delay(5);
  }
}