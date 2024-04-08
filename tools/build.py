#
#  Copyright (c) 2024 Infineon Technologies AG.
#
#  This file is part of TAS Client, an API for device access for Infineon's 
#  automotive MCUs. 
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  ****************************************************************************************************************#
import os
import platform
import argparse

from utilities import run

# Input arguments parsing
parser = argparse.ArgumentParser("Build script for building TAS Client API")
parser.add_argument("-c", "--config", choices=["release", "debug", "all"], default="release", help="Specifies configuration type to build")
parser.add_argument("-p", "--python", help="Build the python wrapper of the C++ API", action="store_true")
parser.add_argument("-d", "--docs", help="Build the API reference documentation", action="store_true")
parser.add_argument("-t", "--tests", help="Build the test executables", action="store_true")
parser.add_argument("-sq", "--sonar", help="Build using SonarQube build wrapper. Only in release mode.", action="store_true")
args = parser.parse_args()

# Generate build environment
if args.config in ["release", "all"]:
    run(f"conan install . -o python={str(args.python)} -o docs={str(args.docs)} -o tests={str(args.tests)}")

if args.config in ["debug", "all"]:
    run(f"conan install . -o python={str(args.python)} -o docs={str(args.docs)} -o tests={str(args.tests)} -s build_type=Debug --build=missing")

# CMake version used
run("cmake --version")

# CMake configuration 
if platform.system() == "Windows":
    run("cmake --preset conan-default")
else:
    if args.config in ["release", "all"]:
        run("cmake --preset conan-release")
    if args.config in ["debug", "all"]:
        run("cmake --preset conan-debug")
   
# Build step
if args.config in ["release", "all"]:
    if args.sonar:
        if platform.system() == "Windows":
            run("build-wrapper-win-x86-64.exe --out-dir bw-output cmake --build --preset conan-release")
        else:
            run(".sonar/build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir bw-output cmake --build --preset conan-release")
    else:
        run("cmake --build --preset conan-release")

if args.config in ["debug", "all"]:
    run("cmake --build --preset conan-debug")
