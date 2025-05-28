import serial
import time
import matplotlib.pyplot as plt

# --- Config ---
STUDENT_PORT = "COM5"  # Adjust to match your setup
TESTER_PORT = "COM6"
BAUD_RATE = 115200
SAMPLE_DURATION = 5.0  # seconds to record

def send_test_number(port, test_num):
    port.write(f"{test_num}\n".encode())

def record_from_tester(ser, duration, test_num):
    print("Recording from tester...")
    start_time = time.time()

    timestamps = []
    data1 = []
    data2 = []
    data3 = []

    while time.time() - start_time < duration:
        try:
            line = ser.readline().decode().strip()
            if line:
                parts = line.split(",")

                if test_num >= 24 and len(parts) == 2:
                    # Servo: timestamp, angle
                    t, angle = float(parts[0]), int(parts[1])
                    timestamps.append(t)
                    data1.append(angle)

                elif test_num < 16 and len(parts) == 4:
                    # DC motor: timestamp, enable, dir1, dir2
                    t, e, d1, d2 = float(parts[0]), int(parts[1]), int(parts[2]), int(parts[3])
                    timestamps.append(t)
                    data1.append(e)
                    data2.append(d1)
                    data3.append(d2)

                elif 16 <= test_num <= 23 and len(parts) == 4:
                    # Stepper: timestamp, STEP_EN, DIR, STEP
                    t, en, d, s = float(parts[0]), int(parts[1]), int(parts[2]), int(parts[3])
                    timestamps.append(t)
                    data1.append(en)
                    data2.append(d)
                    data3.append(s)

        except Exception as e:
            print("Error reading line:", e)

    return timestamps, data1, data2, data3

def plot_waveforms(test_num, timestamps, d1, d2=None, d3=None):
    plt.figure(figsize=(10, 5))

    if test_num >= 24:
        plt.title("Servo Angle Over Time")
        plt.plot(timestamps, d1, label="Angle (°)")
        plt.ylabel("Angle")
    else:
        if test_num < 16:
            plt.title("DC Motor Pin States")
            plt.plot(timestamps, d1, label="Enable")
            plt.plot(timestamps, d2, label="DIR1")
            plt.plot(timestamps, d3, label="DIR2")
        else:
            plt.title("Stepper Pin States")
            plt.plot(timestamps, d1, label="STEP_EN")
            plt.plot(timestamps, d2, label="DIR")
            plt.plot(timestamps, d3, label="STEP")

        plt.ylabel("State (0 or 1)")
        plt.ylim(-0.2, 1.2)

    plt.xlabel("Time (s)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

def main():
    test_num = int(input("Enter test number (0–29): "))

    with serial.Serial(STUDENT_PORT, BAUD_RATE, timeout=1) as student, \
         serial.Serial(TESTER_PORT, BAUD_RATE, timeout=1) as tester:

        time.sleep(2)
        print(f"Sending test number {test_num} to both Arduinos...")
        send_test_number(student, test_num)
        send_test_number(tester, test_num)

        timestamps, d1, d2, d3 = record_from_tester(tester, SAMPLE_DURATION, test_num)
        print(f"Recorded {len(timestamps)} samples.")

        plot_waveforms(test_num, timestamps, d1, d2, d3)

if __name__ == "__main__":
    main()
