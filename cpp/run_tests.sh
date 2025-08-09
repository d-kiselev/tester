#!/bin/bash

set -e

CPP_FILES="tester.cpp solution_to_test.cpp"
EXECUTABLE_NAME="tester"

g++ -std=c++17 -Wall $CPP_FILES -o $EXECUTABLE_NAME
./$EXECUTABLE_NAME
