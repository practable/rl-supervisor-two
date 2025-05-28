// --- Pin Assignments ---
#define ENABLE_PIN       2    // DC motor enable (Q0)
#define DIR1_PIN         7    // DC DIR1 from supervisor
#define DIR2_PIN         12   // DC DIR2 from supervisor

#define STEP_SELECT_PIN  A3   // Stepper STEP_EN
#define STEP_DIR_PIN     A4   // Stepper DIR
#define STEP_PULSE_PIN   A5   // Stepper STEP

#define SERVO_MONITOR_PIN A6  // Reads PWM signal to servo

// --- Config ---
const unsigned long SAMPLE_DURATION_MS = 5000;
const unsigned long SAMPLE_INTERVAL_MS = 1;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(ENABLE_PIN, INPUT);
  pinMode(DIR1_PIN, INPUT);
  pinMode(DIR2_PIN, INPUT);

  pinMode(STEP_SELECT_PIN, INPUT);
  pinMode(STEP_DIR_PIN, INPUT);
  pinMode(STEP_PULSE_PIN, INPUT);

  pinMode(SERVO_MONITOR_PIN, INPUT);

  Serial.println("Tester ready. Waiting for test number...");
}

void loop() {
  if (Serial.available()) {
    int testNum = Serial.parseInt();
    Serial.print("Running test "); Serial.println(testNum);

    unsigned long start = millis();

    while (millis() - start < SAMPLE_DURATION_MS) {
      unsigned long t = millis() - start;

      if (testNum >= 24 && testNum <= 29) {
        // Servo: measure PWM pulse width
        unsigned long pulse = pulseIn(SERVO_MONITOR_PIN, HIGH, 25000);  // microseconds
        int angle = map(pulse, 1000, 2000, 0, 180);  // Rough angle estimate
        Serial.print(t / 1000.0, 3); Serial.print(",");
        Serial.println(angle);

      } else if (testNum < 16) {
        // DC motor: read enable and direction pins
        int enable = digitalRead(ENABLE_PIN);
        int dir1 = digitalRead(DIR1_PIN);
        int dir2 = digitalRead(DIR2_PIN);
        Serial.print(t / 1000.0, 3); Serial.print(",");
        Serial.print(enable); Serial.print(",");
        Serial.print(dir1); Serial.print(",");
        Serial.println(dir2);

      } else {
        // Stepper: STEP_EN, DIR, STEP
        int select = digitalRead(STEP_SELECT_PIN);
        int dir = digitalRead(STEP_DIR_PIN);
        int pulse = digitalRead(STEP_PULSE_PIN);
        Serial.print(t / 1000.0, 3); Serial.print(",");
        Serial.print(select); Serial.print(",");
        Serial.print(dir); Serial.print(",");
        Serial.println(pulse);
      }

      delay(SAMPLE_INTERVAL_MS);
    }

    Serial.println("Done recording.");
  }
}
