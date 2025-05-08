# USB/IP Integration for Remote Arduino Programming

This directory documents the integration of USB/IP into the Remote Labs project, enabling remote Arduino programming over a network using native USB tools like the Arduino IDE.

USB/IP (USB over IP) allows a remote USB device (e.g. Arduino) to appear as if it were plugged directly into a student's local computer. This enables seamless uploads and serial communication without requiring custom drivers or cloud toolchains.

## Summary of Research

This project evaluated the use of USB/IP for remote sketch uploads to Arduino hardware. It was found that:

- **Not all Arduinos work reliably over USB/IP** due to bootloader and USB enumeration behaviour.
- Boards like the **Arduino Uno R3** (Optiboot bootloader) work over USB/IP.
- Boards like the **Nano 33 IoT** (BOSSA/UF2 bootloader) fail due to timing issues and port re-enumeration.
- Uploads via USB/IP work reliably up to **243 ms** of round-trip latency for boards using Optiboot.
- Above this latency, bootloader timeouts—not USB/IP—cause uploads to fail.

The recommended board for USB/IP use is the **Arduino Uno** or **classic Arduino Nano**, both using the **Optiboot** bootloader and ATmega328P.

## Installation

### On Linux (e.g., Raspberry Pi or client machine)

Most USB/IP tools are already included in recent Linux distributions. If needed:

```bash
sudo apt update
sudo apt install linux-tools-$(uname -r) linux-cloud-tools-$(uname -r)
```
Then load the required kernel modules:

```bash
sudo modprobe usbip_core
sudo modprobe usbip_host         # On host
sudo modprobe vhci-hcd           # On client
```
On Windows (client only)
There is no official USB/IP support on Windows, but it can work using a community-maintained tool:

GitHub repo: https://github.com/cezuni/usbip-win

You can use the GUI-based USB/IP Manager or command line tools.

Follow the usage guide in the GitHub repo for full instructions.

Setup Instructions (Linux-to-Linux)
On the Host (e.g., Raspberry Pi or Odroid with Arduino connected)
Connect your Arduino (Uno or Nano) to the USB port.

Load kernel modules:

```bash
sudo modprobe usbip_core
sudo modprobe usbip_host
```
Start the USB/IP daemon:

```bash
sudo usbipd -D
```
List USB devices and find your Arduino:

```bash
usbip list -l
```
Example output:

```diff
- busid 1-1.3 (2341:0043)
  Arduino SA : Uno R3 (CDC ACM)
```
Bind the Arduino for export:

```bash
sudo usbip bind -b 1-1.3
```
On the Client (Linux machine)
Load required modules:

```bash
sudo modprobe usbip_core
sudo modprobe vhci-hcd
```
List remote devices:

```bash
usbip list -r <RPI_IP_ADDRESS>
```
Attach the remote device:

```bash
sudo usbip attach -r <RPI_IP_ADDRESS> -b 1-1.3
```
Confirm it's available:

```bash
lsusb
```
You should see:

```diff
Bus 001 Device 003: ID 2341:0043 Arduino SA Uno R3 (CDC ACM)
```
You can now open the Arduino IDE and upload sketches as if the board was plugged in locally.

Example Terminal Output (Linux) can be found at the end of this doc

# Latency Limit Summary

- Uploads succeed reliably up to **243 ms round-trip latency** using the **Uno R3**.
- Beyond this, **Optiboot exits before upload begins**.
- Failures are due to **bootloader timeout**, not USB/IP itself.

---

## Alternative Strategies for High Latency

For students with poor internet or latency >243 ms, USB/IP is not reliable. Consider these fallback strategies:

### 1. Caching Proxy (Recommended)

Upload is intercepted locally (e.g., by a proxy on the student’s machine):

- Sketch is sent to the server (e.g., Raspberry Pi) as a binary.
- A local script or daemon then flashes it directly to the board.
- Eliminates tight timing issues and bootloader failures.
- Maintains Arduino IDE compatibility with minimal changes.

### 2. Cloud-Based IDE + Server Upload

- Students use a browser IDE (e.g., Codespaces, Eclipse Theia, etc.).
- Code is compiled and uploaded server-side.
- Guarantees upload timing, avoids USB/IP completely.
- More complex, but scalable and reliable.

### 3. Bootloader Timeout Extension

- Modify the **Optiboot bootloader** to extend the timeout window.
- Works with USB/IP at higher latency.
- Requires custom bootloader flashing and careful documentation.

---

## Conclusion

- USB/IP works very well with the **Arduino Uno** and **classic Nano**.
- Uploads are reliable up to **243 ms latency**.
- **Nano 33 IoT** and other native USB boards **do not work**.
- For rural/slow networks, **caching proxy upload** is the most robust alternative.

---

## For Your Lab Setup, Use:

- **Arduino Uno R3** or **Nano (ATmega328P)**
- **Optiboot bootloader**
- **USB/IP** + `arduino-cli` or Arduino IDE

![USBIP Server](../docs/images/usbip_host_terminal.png)  
![USBIP Client](../docs/images/usbip_client_terminal.png)

