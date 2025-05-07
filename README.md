
# Remote Labs Supervisor PCB – New Development

This repository contains the latest work and updates for the **Remote Labs Supervisor PCB V1.2** project.

The **Remote Labs Supervisor PCB** is designed to allow students to safely upload and execute microcontroller code remotely, while a supervisor system monitors and prevents actions that could damage hardware. This new repo documents the latest progress in firmware, usage tools, and supporting scripts.

---

## Repository Structure

This repository is organized into several key folders:

### `supervisor/`
- Contains all firmware code related to the **Supervisor MCU**.
- The supervisor is responsible for monitoring outputs, blocking unsafe actions, spoofing safe signals, and providing control when needed.
- Includes a `README.md` inside the folder that explains the features and capabilities of the supervisor code.

### `student/`
- Contains code for the **Student MCU** — the microcontroller students interact with directly via remote labs.
- Also includes a `README.md` that provides a usage guide and explains how to upload and work with the code.

#### `student/tester/`
- This subfolder includes code for a third Arduino called the **Tester**.
- The Tester is connected to the outputs of the Student and Supervisor MCUs and is used to record and validate their behavior.
- The Tester is integrated with a Python script for automated result analysis.
- Detailed explanation of how this works is included in the documentation inside the folder.

### `usbip/`
- Contains documentation and setup scripts for using **USB over IP (usbip)**.
- usbip is used to allow remote USB devices (such as Arduinos) to appear locally for programming or serial communication.
- The folder includes a guide on what usbip is, how it is used in this context, and how to set it up.

---

## Summary

This repo is a continuation of the Remote Labs Supervisor PCB project, with added tools, MCU firmware, test frameworks, and documentation to support robust remote experimentation in embedded systems. It enables:

- Safe and constrained student interaction with hardware
- Supervisor-level override and monitoring
- Automated output validation via a third microcontroller
- USB-over-IP functionality for remote device access

---

For more information, please see the individual `README.md` files inside each folder.
