/*studentCode.ino

    code for student MCU to test outputs can only be controlled when the code is safe
    Anisha Burgul - 24/04/25
*/
#include "shiftRegister.h"
// Pin definitions
#define PWM_THING       10
#define DC_MOTOR_DIR1   12
#define DC_MOTOR_DIR2   7
#define LATCH           8
#define MOSI            11
#define SCK             13
#define COUNTER_START   A0  // A0–A2 used for 6-bit encoded test number
#define START_SIGNAL    A0  // Reused for input detection before test
#define STUDENT_SERVO_OUT 9  // Student sends PPM/PWM on D9

// Shift register output bitmasks
#define Q0 0b00000001  // DC_MOTOR_EN
shiftRegister shiftReg(MOSI, SCK, LATCH);

// ---------------------
//  Start functions
// ---------------------
void waitForStartSignal() {
  pinMode(START_SIGNAL, INPUT);
  Serial.println("Waiting for tester start signal on A0...");

  while (digitalRead(START_SIGNAL) == LOW);
  Serial.println("Start signal received");
  delay(100);
}

void sendSyncPulse() {
  for (int i = 0; i < 3; i++) {
    pinMode(COUNTER_START + i, OUTPUT);
    digitalWrite(COUNTER_START + i, HIGH);
  }

  delay(1000);

  for (int i = 0; i < 3; i++) {
    digitalWrite(COUNTER_START + i, LOW);
  }

  delay(50);
}

void send6BitTestNumber(uint8_t testNumber) {
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 3; j++) {
      pinMode(COUNTER_START + j, OUTPUT);
      digitalWrite(COUNTER_START + j, (testNumber >> (i * 3 + j)) & 1);
    }
    delay(250);
  }

  for (int j = 0; j < 3; j++) {
    digitalWrite(COUNTER_START + j, LOW);
  }

  for (int j = 0; j < 3; j++) {
    digitalWrite(COUNTER_START + j, LOW);
    pinMode(COUNTER_START + j, INPUT);
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

// ---------------------
// Arduino Setup / Loop
// ---------------------
void setup() {
  Serial.begin(115200);

  pinMode(DC_MOTOR_DIR1, OUTPUT);
  pinMode(DC_MOTOR_DIR2, OUTPUT);
  pinMode(STUDENT_SERVO_OUT, OUTPUT);
  digitalWrite(STUDENT_SERVO_OUT, LOW);

  shiftReg.begin();
  shiftReg.allOff();
}

void loop() {
  waitForStartSignal();
  pinMode(6, OUTPUT);
  digitalWrite(PWM_THING, HIGH);

  const int totalTests = 30;
  const int delayTime = 3000;

  for (int test = 0; test < totalTests; test++) {
    Serial.print("Starting test ");
    Serial.println(test);

    clearMotor();
    shiftReg.allOff();
    digitalWrite(6, LOW);
    sendSyncPulse();
    delay(50);
    send6BitTestNumber(test);
    delay(200);

    switch (test) {
      case 0: case0(); break;
      case 1: case1(); break;
      ... //condensed to save space
      case 29: case29(); break;
    }
    delay(delayTime);
  }
  clearMotor();
  shiftReg.allOff();
  Serial.println("All tests complete. Waiting for new signal.\n");
}