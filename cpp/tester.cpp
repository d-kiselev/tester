#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <map>
#include <stdexcept>

#include "solution.h"

// --- Configuration ---
namespace fs = std::filesystem;
const fs::path TESTS_DIR = "tests/";
const bool FULL_LOGGING = false;

// Define colors for terminal output
const std::map<std::string, std::string> COLORS = {
    {"green", "\033[92m"},
    {"red",   "\033[91m"},
    {"bold",  "\033[1m"},
    {"end",   "\033[0m"}
};

// --- Helper Functions ---

// Prints text in a given color
void print_colored(const std::string& text, const std::string& color_name) {
    auto it = COLORS.find(color_name);
    std::string color_code = (it != COLORS.end()) ? it->second : "";
    std::cout << color_code << text << COLORS.at("end");
}

// Finds and returns a sorted list of test case basenames (e.g., "1", "2")
std::vector<std::string> find_test_cases(const fs::path& directory) {
    std::vector<std::string> basenames;
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        print_colored("❌ Error: Test directory '" + directory.string() + "' not found.", "red");
        std::cout << std::endl;
        return basenames;
    }

    std::vector<int> case_numbers;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".in") {
            try {
                int num = std::stoi(entry.path().stem().string());
                case_numbers.push_back(num);
            } catch (const std::exception&) { /* Ignore non-numeric filenames */ }
        }
    }
    
    std::sort(case_numbers.begin(), case_numbers.end());
    for(int num : case_numbers) {
        basenames.push_back(std::to_string(num));
    }
    
    return basenames;
}

// Reads the entire content of a file into a string
std::string read_file_content(const fs::path& filepath) {
    std::ifstream file(filepath);
    if (!file) throw std::runtime_error("Could not open file: " + filepath.string());
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Removes trailing whitespace from a string (like Python's .strip())
void trim_trailing(std::string& s) {
    s.erase(s.find_last_not_of(" \n\r\t") + 1);
}

// --- Test Execution ---

// Runs a single test case
bool run_test(const std::string& case_name) {
    fs::path in_file = TESTS_DIR / (case_name + ".in");
    fs::path out_file = TESTS_DIR / (case_name + ".out");

    if (FULL_LOGGING) {
        std::cout << "Running test '" << case_name << "'... " << std::flush;
    }

    try {
        // 1. Read input data
        std::ifstream input_stream(in_file);
        if (!input_stream) {
            print_colored("⚠️ SKIPPED (Input file not found)", "bold");
            std::cout << std::endl;
            return false;
        }
        std::string input_str;
        int input_int;
        std::getline(input_stream, input_str);
        input_stream >> input_int;
        
        // 2. Read expected output
        if (!fs::exists(out_file)) {
             print_colored("⚠️ SKIPPED (Output file not found)", "bold");
             std::cout << std::endl;
             return false;
        }
        std::string expected_output = read_file_content(out_file);
        trim_trailing(expected_output);

        // 3. Call the user's solution function
        std::string actual_output = std::to_string(solution(input_str, input_int));
        trim_trailing(actual_output);
        
        // 4. Compare results
        if (actual_output == expected_output) {
            if (FULL_LOGGING) {
                print_colored("✅ PASSED", "green");
                std::cout << std::endl;
            }
            return true;
        } else {
            if (FULL_LOGGING) {
                print_colored("❌ FAILED", "red");
                std::cout << std::endl;
                std::cout << "   - Input          : text=\"" << input_str << "\", n=" << input_int << std::endl;
                std::cout << "   - Expected output: " << expected_output << std::endl;
                std::cout << "   - Actual output  : " << actual_output << std::endl;
            }
            return false;
        }

    } catch (const std::exception& e) {
        print_colored("❌ FAILED", "red");
        std::cout << std::endl;
        std::cout << "   - Error: An exception occurred: " << e.what() << std::endl;
        return false;
    }
}

// --- Main Driver ---

int main() {
    if (FULL_LOGGING) {
        print_colored("--- Starting C++ Tester ---", "bold");
        std::cout << std::endl;
    }

    std::vector<std::string> test_cases = find_test_cases(TESTS_DIR);
    if (test_cases.empty()) {
        std::cout << "No test cases found in '" << TESTS_DIR.string() << "'." << std::endl;
        return 0;
    }

    int passed_count = 0;
    for (const auto& test_case : test_cases) {
        if (run_test(test_case)) {
            passed_count++;
        }
    }
    
    if (FULL_LOGGING) {
        std::cout << std::endl;
        print_colored("--- Test Summary ---", "bold");
        std::cout << std::endl;
    }

    int total_count = test_cases.size();
    std::string summary_color = (passed_count == total_count) ? "green" : "red";
    print_colored("Passed " + std::to_string(passed_count) + " out of " + std::to_string(total_count) + " tests.", summary_color);
    std::cout << std::endl;
    
    // Return a non-zero exit code if any test failed
    return (passed_count == total_count) ? 0 : 1;
}
