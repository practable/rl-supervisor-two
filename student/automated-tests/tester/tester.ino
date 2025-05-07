/*
tester.ino
    takes inputs from the supervisor PCB 
    (DC motor, Stepper Motor, Servo and test number from Student Arduino inputted)

    Anisha Burgul
    28/04/2025
*/


// test counter
#define PIN0 A0  // Also used to send start signal
#define PIN1 A1
#define PIN2 A2

//dc motor 
#define ENABLE_PIN 2   // Reads motor enable (Q0 from shift register)
#define DIR1_PIN  7    // Reads spoofed DIR1 from supervisor (D7)
#define DIR2_PIN  12   // Reads spoofed DIR2 from supervisor (D12)

// stepper
#define STEP_SELECT_PIN A3
#define DIR_OUT_PIN     A4
#define STEP_OUT_PIN    A5

// servo
#define SERVO_MONITOR_PIN A6


const int PINS[3] = {PIN0, PIN1, PIN2};

const int SYNC_DURATION = 1000;   // ms
const int BIT_DURATION = 300;     // ms
const int NUM_TESTS = 30;

const int SAMPLE_INTERVAL = 10;       // Time between readings (ms)
const int SAMPLE_DURATION = 3000; 

void setup() {
  Serial.begin(115200);
  Serial.println("Tester: Initialising... (waiting for Python)");

  // Setup motor direction input pins
  pinMode(ENABLE_PIN, INPUT);
  pinMode(DIR1_PIN, INPUT);
  pinMode(DIR2_PIN, INPUT);

  // setup stepper pins
  pinMode(STEP_SELECT_PIN, INPUT);
  pinMode(DIR_OUT_PIN, INPUT);
  pinMode(STEP_OUT_PIN, INPUT);

  //servo pins
  pinMode(SERVO_MONITOR_PIN, INPUT);


  // Setup communication pins
  for (int i = 0; i < 3; i++) {
    pinMode(PINS[i], INPUT);
  }

  // Do not send signal yet — wait for Python to tell us
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');

    if (command == "start") {
      Serial.println("Received start command from Python.");
      sendStartSignal();

      Serial.println("Starting test sequence");

      for (int i = 0; i < NUM_TESTS; i++) {
        waitForSyncPulse();
        int testId = read6Bits();

        // Read and log pin states based on test type
        unsigned long start = millis();
        while (millis() - start < SAMPLE_DURATION) {
          if (testId >= 26 && testId <= 29) {
            // Servo angle test
            unsigned long pulse = pulseIn(SERVO_MONITOR_PIN, HIGH, 25000);  // up to 25ms
            int angle = map(pulse, 1000, 2000, 0, 180);  // Convert pulse to angle
            Serial.print("SRV,");
            Serial.print(testId); Serial.print(",");
            Serial.println(angle);
          }
          else if (testId < 16) {
            // DC Motor tests
            int enable = digitalRead(ENABLE_PIN);
            int dir1 = digitalRead(DIR1_PIN);
            int dir2 = digitalRead(DIR2_PIN);

            Serial.print("DC,");
            Serial.print(testId); Serial.print(",");
            Serial.print(enable); Serial.print(",");
            Serial.print(dir1);   Serial.print(",");
            Serial.println(dir2);
          }
          else {
            // Stepper tests
            int select = digitalRead(STEP_SELECT_PIN);
            int dirOut = digitalRead(DIR_OUT_PIN);
            int stepOut = digitalRead(STEP_OUT_PIN);

            Serial.print("STP,");
            Serial.print(testId); Serial.print(",");
            Serial.print(select); Serial.print(",");
            Serial.print(dirOut); Serial.print(",");
            Serial.println(stepOut);
          }


          delay(SAMPLE_INTERVAL);
        }

      }

      Serial.println("All tests complete.");
      delay(100);
    }
  }
}

// ---------------------
// Start Signal to Student
// ---------------------
//python starts tester, tester starts student
void sendStartSignal() {
  Serial.println("Sending start signal to student...");

  pinMode(PIN0, OUTPUT);
  digitalWrite(PIN0, HIGH);
  delay(100);  // Pulse HIGH for 100ms
  digitalWrite(PIN0, LOW);
  pinMode(PIN0, INPUT); // Release control

  Serial.println("Start signal sent. Listening for tests...");
}

// ---------------------
// Wait for Student Sync
// ---------------------
void waitForSyncPulse() {
  while (true) {
    if (allPinsHigh()) {
      unsigned long start = millis();
      while (allPinsHigh()) {
        if (millis() - start >= SYNC_DURATION) {
          Serial.println("Sync pulse detected.");
          return;
        }
      }
    }
  }
}

// ---------------------
// Read 6-bit Test ID
// ---------------------
int read6Bits() {
  int result = 0;

  delay(BIT_DURATION);  // Wait for first bit set

  for (int round = 0; round < 2; round++) {
    for (int pin = 0; pin < 3; pin++) {
      int bit = digitalRead(PINS[pin]);
      result |= (bit << (round * 3 + pin));
    }
    delay(BIT_DURATION);
  }

  return result;
}

bool allPinsHigh() {
  for (int i = 0; i < 3; i++) {
    if (digitalRead(PINS[i]) == LOW) return false;
  }
  return true;
}
  