import subprocess
import os
import re

script_dir = os.path.dirname(os.path.abspath(__file__))

expected_output = """\
Instruction 1,   Hex Instruction: 0x007302b3,   R-type (Register)
Instruction 2,   Hex Instruction: 0x00a28413,   I-type (Immediate)
Instruction 3,   Hex Instruction: 0x00852023,   S-type (Store)
Instruction 4,   Hex Instruction: 0x00828863,   B-type (Branch)
Instruction 5,   Hex Instruction: 0x123454b7,   U-type (Upper Immediate)
Instruction 6,   Hex Instruction: 0x008000ef,   J-type (Jump)
Instruction 7,   Hex Instruction: 0x00000073,   I-type (Immediate)"""

tests = [
    {
        "description": "ASCII binary file",
        "cmd": [os.path.join(script_dir, "../../build/riscv-objdump"), "-types", f"--file={os.path.join(script_dir, "inputs/instruction_types_bin.txt")}", "-filetype=binstr"],
        "should-fail": False
    },
    {
        "description": "ASCII hex file",
        "cmd": [os.path.join(script_dir, "../../build/riscv-objdump"), "-types", f"--file={os.path.join(script_dir, "inputs/instruction_types_hex.txt")}", "-filetype=hexstr"],
        "should-fail": False
    },
    {
        "description": "Raw binary file",
        "cmd": [os.path.join(script_dir, "../../build/riscv-objdump"), "-types", f"--file={os.path.join(script_dir, "inputs/instruction_types_little.bin")}", "-filetype=bin", "--endian=little"],
        "should-fail": False
    },
    {
        "description": "Raw binary file with wrong endian",
        "cmd": [os.path.join(script_dir, "../../build/riscv-objdump"), "-types", f"--file={os.path.join(script_dir, "inputs/instruction_types_little.bin")}", "-filetype=bin", "--endian=big"],
        "should-fail": True
    }
]

def normalize_whitespace(s):
    return re.sub(r'\s+', ' ', s.strip())

def run_tests():
    all_passed = True
    for test in tests:
        print(f"Running test: {test['description']}...")
        result = subprocess.run(test["cmd"], capture_output=True, text=True)
        output = result.stdout
        failed = result.returncode != 0

        if test["should-fail"]:
            if failed:
                print("Correctly failed\n")
            else:
                print("Should have failed but passed!")
                print("Output:")
                print(output)
                all_passed = False
        else:
            expected_lines = [normalize_whitespace(line) for line in expected_output.strip().splitlines()]
            actual_lines = [normalize_whitespace(line) for line in output.strip().splitlines()]

            if expected_lines == actual_lines:
                print("Test passed\n")
            else:
                print("Output mismatch!")
                for i, (exp, act) in enumerate(zip(expected_lines, actual_lines), start=1):
                    if exp != act:
                        print(f"Line {i}:\n  Expected: {repr(exp)}\n  Got:      {repr(act)}")
                if len(expected_lines) != len(actual_lines):
                    print(f"Line count mismatch: expected {len(expected_lines)}, got {len(actual_lines)}")
                all_passed = False
    return all_passed

if __name__ == "__main__":
    success = run_tests()
    if success:
        print("All tests passed.")
    else:
        print("Some tests failed.")

