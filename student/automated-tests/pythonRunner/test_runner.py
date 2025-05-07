import serial
import time
import csv
import json
import os
import ast
from collections import defaultdict

# --- Config ---
COM_PORT = "COM5" # change to whatever port your Arduino is on
# COM_PORT = "/dev/ttyUSB0"  # For Linux users, change to your Arduino's port
BAUD_RATE = 115200
CSV_FILE = "expected_results.csv"
BADGE_FOLDER = "badges"
OVERALL_BADGE = os.path.join(BADGE_FOLDER, "results-badge.json")
PERCENTAGE_BADGE = os.path.join(BADGE_FOLDER, "results-percentage-badge.json")
FRACTION_BADGE = os.path.join(BADGE_FOLDER, "results-fraction-badge.json")
SAMPLES_TO_IGNORE = 26
MUST_END_IN_ZERO = {4, 8, 10, 11, 13, 14, 15}
BASIC_TESTS = set(range(0, 8))

def read_expected_results(file_path):
    expected = defaultdict(list)
    with open(file_path, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            try:
                test_id = int(row['test_id'])
                enable = row['enable'].strip()
                if not enable:
                    continue

                if enable.startswith("["):
                    parsed = ast.literal_eval(enable)
                    if all(isinstance(val, list) and len(val) == 1 for val in parsed):
                        # Servo test 29: list of valid angles
                        expected[test_id] = [x[0] for x in parsed]
                    else:
                        expected[test_id] = parsed
                elif test_id >= 24 and test_id <= 29:
                    expected[test_id] = int(enable)  # Servo angle
                else:
                    expected[test_id] = [[int(enable), int(row['dir1']), int(row['dir2'])]]
            except Exception as e:
                print(f"Error parsing row: {row} — {e}")
    return expected

def write_badge(passed):
    with open(OVERALL_BADGE, "w") as f:
        json.dump({
            "schemaVersion": 1,
            "label": "Supervisor Tests",
            "message": "pass" if passed else "fail",
            "color": "brightgreen" if passed else "red"
        }, f)

def write_percentage_badge(passed, total):
    percent = int((passed / total) * 100)
    with open(PERCENTAGE_BADGE, "w") as f:
        json.dump({
            "schemaVersion": 1,
            "label": "Passing",
            "message": f"{percent}%",
            "color": "brightgreen" if percent == 100 else "orange" if percent >= 50 else "red"
        }, f)

def write_fraction_badge(passed, total):
    with open(FRACTION_BADGE, "w") as f:
        json.dump({
            "schemaVersion": 1,
            "label": "Passed",
            "message": f"{passed}/{total}",
            "color": "brightgreen" if passed == total else "orange" if passed > 0 else "red"
        }, f)

def write_individual_badges(results_by_test, expected):
    for test_id, allowed in expected.items():
        outputs = results_by_test.get(test_id, [])
        if test_id == 29:
            passed = any([out in allowed for out in outputs if isinstance(out, int)])
        elif 24 <= test_id <= 29:
            if outputs:
                avg = sum(outputs) / len(outputs)
                passed = abs(avg - allowed) <= 5
            else:
                passed = False
        elif test_id in BASIC_TESTS:
            passed = any([out in allowed for out in outputs]) if outputs else False
        elif test_id in MUST_END_IN_ZERO:
            passed = outputs and outputs[-1] == [0, 0, 0]
        else:
            passed = all([out in allowed for out in outputs]) if outputs else False

        badge_path = os.path.join(BADGE_FOLDER, f"test{test_id}-badge.json")
        with open(badge_path, "w") as f:
            json.dump({
                "schemaVersion": 1,
                "label": f"Test {test_id}",
                "message": "pass" if passed else "fail",
                "color": "brightgreen" if passed else "red"
            }, f)

def print_compressed_output(line, last_line, repeat_count):
    if last_line is not None:
        if repeat_count > 1:
            print(f"Arduino: {last_line} *{repeat_count}")
        else:
            print(f"Arduino: {last_line}")
    return line, 1

def main():
    if not os.path.exists(BADGE_FOLDER):
        os.makedirs(BADGE_FOLDER)

    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
    ser.write(b'start\n')
    print("Listening for test results...\n")

    results_by_test = defaultdict(list)
    sample_counts = defaultdict(int)
    last_line = None
    repeat_count = 0

    TIMEOUT_AFTER_LAST_LINE = 30
    last_read_time = time.time()

    while True:
        line = ser.readline().decode("utf-8").strip()
        now = time.time()

        if line:
            last_read_time = now
        else:
            if now - last_read_time > TIMEOUT_AFTER_LAST_LINE:
                print("Timeout reached. Ending test loop.")
                break
            continue

        if line == last_line:
            repeat_count += 1
        else:
            last_line, repeat_count = print_compressed_output(line, last_line, repeat_count)

        if line.startswith("All tests"):
            break

        if "," in line:
            parts = line.split(",")
            prefix = parts[0]

            try:
                test_id = int(parts[1])
            except (IndexError, ValueError):
                continue

            if prefix == "DC":
                try:
                    enable, dir1, dir2 = map(int, parts[2:5])
                    result = [enable, dir1, dir2]
                except ValueError:
                    continue
            elif prefix == "STP":
                try:
                    select, dirOut, stepOut = map(int, parts[2:5])
                    result = [select, dirOut, stepOut]
                except ValueError:
                    continue
            elif prefix == "SRV":
                try:
                    angle = int(parts[2])
                    result = angle
                except ValueError:
                    continue
            else:
                continue

            sample_counts[test_id] += 1
            if sample_counts[test_id] > SAMPLES_TO_IGNORE:
                results_by_test[test_id].append(result)

    if last_line is not None:
        if repeat_count > 1:
            print(f"Arduino: {last_line} *{repeat_count}")
        else:
            print(f"Arduino: {last_line}")

    expected = read_expected_results(CSV_FILE)
    print("\nChecking results...")
    num_passed = 0
    total = len(expected)

    for test_id, allowed in expected.items():
        outputs = results_by_test.get(test_id, [])
        if test_id == 29:
            passed = any([out in allowed for out in outputs if isinstance(out, int)])
        elif 24 <= test_id <= 29:
            angles = [val for val in outputs if isinstance(val, int)]
            if not angles:
                passed = False
            else:
                avg = sum(angles) / len(angles)
                passed = abs(avg - allowed) <= 5
        elif test_id in BASIC_TESTS:
            passed = any([out in allowed for out in outputs]) if outputs else False
        elif test_id in MUST_END_IN_ZERO:
            passed = outputs and outputs[-1] == [0, 0, 0]
        else:
            passed = all([out in allowed for out in outputs]) if outputs else False

        print(f"Test {test_id}: {len(outputs)} samples → {'PASS' if passed else 'FAIL'}")

        if not passed:
            mismatch_counts = defaultdict(int)
            for out in outputs:
                if isinstance(allowed, int):
                    if not isinstance(out, int) or abs(out - allowed) > 5:
                        mismatch_counts[out] += 1
                else:
                    if out not in allowed:
                        mismatch_counts[tuple(out)] += 1

            for mismatch, count in mismatch_counts.items():
                if isinstance(mismatch, tuple):
                    print(f"   Got {list(mismatch)} {f'*{count}' if count > 1 else ''}, expected one of {allowed}")
                else:
                    print(f"   Got {mismatch} {f'*{count}' if count > 1 else ''}, expected {allowed}")

        if passed:
            num_passed += 1

    write_badge(num_passed == total)
    write_percentage_badge(num_passed, total)
    write_fraction_badge(num_passed, total)
    write_individual_badges(results_by_test, expected)

    ser.close()

if __name__ == "__main__":
    main()
