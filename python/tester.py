import sys
import os
import importlib.util
from typing import List, Tuple

# --- Configuration ---
TESTS_DIR = 'tests/'
FULL_LOGGING = False
COLORS = {
    "green": "\033[92m",
    "red": "\033[91m",
    "bold": "\033[1m",
    "end": "\033[0m"
}

def print_colored(text: str, color: str):
    """Prints text in a given color."""
    print(f"{COLORS.get(color, '')}{text}{COLORS['end']}")

def load_module_from_file(filepath: str):
    """Dynamically loads a Python module from a given file path."""
    try:
        # Create a unique module name from the file path
        module_name = os.path.splitext(os.path.basename(filepath))[0]
        spec = importlib.util.spec_from_file_location(module_name, filepath)
        if spec is None or spec.loader is None:
            print_colored(f"❌ Error: Could not create module spec from {filepath}.", "red")
            return None
        
        user_module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(user_module)
        return user_module
    except FileNotFoundError:
        print_colored(f"❌ Error: The file '{filepath}' was not found.", "red")
        return None
    except Exception as e:
        print_colored(f"❌ Error loading module: {e}", "red")
        return None

def find_test_cases(directory: str) -> List[str]:
    """Finds and returns a sorted list of test case basenames (e.g., '1', '2')."""
    if not os.path.isdir(directory):
        print_colored(f"❌ Error: Test directory '{directory}' not found.", "red")
        return []
    
    # Get all .in files and extract their base names
    in_files = [f for f in os.listdir(directory) if f.endswith('.in')]
    basenames = sorted([int(f.removesuffix('.in')) for f in in_files])
    return [str(name) for name in basenames]

def run_test(module, case_name: str) -> bool:
    in_file = os.path.join(TESTS_DIR, f"{case_name}.in")
    out_file = os.path.join(TESTS_DIR, f"{case_name}.out")

    if FULL_LOGGING:
        print(f"Running test '{case_name}'...", end=' ', flush=True)

    try:
        # 1. Read input data
        with open(in_file, 'r') as f:
            input_str = f.readline()
            if input_str[-1] == '\n':
                input_str = input_str[:-1]
            input_int = int(f.readline().strip())

        # 2. Read expected output
        if not os.path.exists(out_file):
            print_colored(f"⚠️ Output file '{out_file}' not found. Skipping.", "bold")
            # actual_output = module.solution(input_str, input_int)
            # with open(out_file, 'w') as f:
            #    f.write(str(actual_output))
            return False
        
        with open(out_file, 'r') as f:
            expected_output = f.read().strip()

        # 3. Call the user's main function
        actual_output = module.solution(input_str, input_int)
        
        # 4. Compare results
        actual_output_str = str(actual_output).strip()

        if actual_output_str == expected_output:
            if FULL_LOGGING:
                print_colored("✅ PASSED ", "green")
            return True
        else:
            if FULL_LOGGING:
                print_colored("❌ FAILED ", "red")
                print(f'   - Input          : text="{input_str}", width={input_int}')
                print(f"   - Expected output: {expected_output}")
                print(f"   - Actual output  : {actual_output_str}")
            return False

    except Exception as e:
        print_colored(f"❌ FAILED", "red")
        print(f"   - Error: {e}")
        return False

def main():
    # 1. Check for command line argument
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <your_python_file.py>")
        sys.exit(1)

    target_file = sys.argv[1]
    if FULL_LOGGING:
        print_colored(f"--- Starting Tester for '{target_file}' ---", "bold")

    # 2. Load the user's Python code as a module
    user_module = load_module_from_file(target_file)
    if user_module is None:
        sys.exit(1)

    if not hasattr(user_module, 'solution'):
        print_colored("❌ Error: The provided file must contain a 'solution(string, int)' function.", "red")
        sys.exit(1)
        
    # 3. Find all available test cases
    test_cases = find_test_cases(TESTS_DIR)
    if not test_cases:
        print(f"No test cases found in the '{TESTS_DIR}' directory.")
        sys.exit(0)

    # 4. Run tests and track results
    passed_count = 0
    total_count = len(test_cases)
    
    for case in test_cases:
        if run_test(user_module, case):
            passed_count += 1
    
    # 5. Print summary
    if FULL_LOGGING:
        print_colored("\n--- Test Summary ---", "bold")
    summary_color = "green" if passed_count == total_count else "red"
    print_colored(f"Passed {passed_count} out of {total_count} tests.", summary_color)
    if FULL_LOGGING:
        print_colored("--------------------", "bold")

if __name__ == "__main__":
    main()
