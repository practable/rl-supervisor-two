# Supervisor MCU Firmware

This directory contains the firmware for the **Supervisor MCU** used in the Remote Labs Supervisor PCB. The Supervisor is responsible for enforcing hardware safety, monitoring signals, and selectively enabling or blocking student control to prevent hardware damage.

The firmware is modular and clearly commented, allowing for configuration of thresholds, timeouts, and logic to suit different experimental setups.

## DC Motor Safety Mechanisms

The Supervisor enforces a range of protective measures when controlling or monitoring the DC motor:

| **Safety Mechanism**        | **Description**                                                                 |
|----------------------------|---------------------------------------------------------------------------------|
| Overcurrent Protection     | Monitors analog current input; disables motor if measured current exceeds threshold |
| Stall Detection            | Detects zero RPM while current is flowing; indicates blocked motor condition   |
| Excessive Runtime Shutdown | Disables motor after a predefined period of continuous operation                |
| Illegal Direction Detection| Detects invalid combinations of DIR1 and DIR2 (e.g., both HIGH); disables output |
| Rapid Toggling Detection   | Blocks motor if enable or direction lines toggle too rapidly                    |

The motor is disabled by setting the `DC_MOTOR_EN` shift register output (Q0) to LOW. Once safe signal states are restored, control is automatically re-enabled.

## Stepper Motor Safety Mechanisms

The Supervisor also monitors the stepper motor control lines to prevent dangerous conditions:

| **Safety Mechanism**           | **Description**                                                                     |
|-------------------------------|-------------------------------------------------------------------------------------|
| Limit Switch Detection         | Reads physical limit switch on A4; prevents further motion at mechanical boundary |
| Fault Line Detection (DRV8825)| Reads fault line on A5 from DRV8825; halts stepper on fault                        |

If a fault or limit condition is detected, the Supervisor disables student access by driving `STEP_EN` (Q2) LOW. Normal operation resumes once safe conditions are restored.

## Servo Motor Safety Mechanisms

Servo motor control is filtered, clamped, and rate-limited to prevent unintended or unsafe behavior:

| **Safety Mechanism**        | **Description**                                                                            |
|----------------------------|--------------------------------------------------------------------------------------------|
| Angle Clamping             | Limits servo angle within `servoMin` and `servoMax` bounds                                |
| Update Rate Limiting       | Restricts how often new servo positions can be applied                                    |
| PPM Signal Validation      | Ignores invalid PPM widths outside the 0.75–2.25 ms range                                  |
| PPM Signal Loss Timeout    | If PPM input is lost, servo resets to a known safe angle                                  |
| Safe Startup Position      | Default angle is set at Supervisor boot/reset to avoid unintended movement                |

The servo output pin (`D9`) is only driven with valid, safe signals derived from either supervisor logic or spoofed student input.

## Configuration & Customization

- **All thresholds, timeouts, and clamping values** (e.g., max current, max runtime, servo limits) are declared as constants near the top of each relevant code section and can be easily modified to fit different experimental use cases.
- **Commented logic blocks** clearly indicate the safety mechanisms and allow developers to adjust behavior without needing to understand the full firmware structure.
- **Use Case Variability**: 
  - Depending on which hardware configuration is mounted (e.g., Spinner, Governor, Digger), minor changes to pin assignments or limits may be necessary.
  - The code is written to be flexible and reusable across these use cases, but assumptions (e.g., motor shield used, presence of encoder) should be validated for each new setup.

## Contents

| File                          | Description                                                                 |
|-------------------------------|-----------------------------------------------------------------------------|
| `supervisorCode.ino`          | **Main firmware file**. Initialises all systems and runs the supervisor control loop. |
| `supervisor_pinMap.h`         | Defines all pin mappings for the Supervisor MCU. |
| `globals.h`                   | Declares global constants, thresholds, and shared flags used across modules. |
| `dcMotorFunctions.h`          | Implements DC motor safety mechanisms including overcurrent, stall, and direction logic. |
| `stepperFunctions.h`          | Implements protection for stepper motors, including limit switch and driver fault handling. |
| `servoFunctions.h`            | Contains servo signal processing: clamping, filtering, and signal timeout behavior. |
| `encoderFunctions.h`          | Shared utility functions for reading encoder state across experiments. |
| `opticalEncoder.cpp/.h`       | Encoder signal analysis for RPM calculation and direction detection. |
| `shiftRegister.cpp/.h`        | Manages the shift register interface that routes control between Student and Supervisor MCUs. |


---

For any issues, adaptations, or experimental extensions, refer to the source code comments for guidance on how to safely modify behavior.
