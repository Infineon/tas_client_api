#
#  Copyright (c) 2026 Infineon Technologies AG.
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
parser.add_argument("--python_root", help="Path to python root folder")
parser.add_argument("--python_version", help="Specify the Python verision available under python root path")
args = parser.parse_args()

if args.python_root:
    if not args.python_version:
        python_ver = os.path.basename(args.python_root)
        python_ver = python_ver.lower()
    else:
        python_ver = args.python_version        
    config = f"-c tools.cmake.cmake_layout:build_folder_vars=\"['const.{python_ver}', 'self.name']\""

    cmake_python_root = f"-DPython_ROOT_DIR=\"{args.python_root}\""        
    cmake_preset_name = f"conan-{python_ver}-tas_client_api"

    print(f"Python root specified {args.python_root} for {python_ver}")
else:
    config = ""
    cmake_python_root = ""
    cmake_preset_name = "conan"
    print("No python root specified.")

# Generate build environment
options = f"-o \"&:python={str(args.python)}\" -o \"&:docs={str(args.docs)}\" -o \"&:tests={str(args.tests)}\""
if args.config in ["release", "all"]:
    run(f"conan install . {options} {config}")

if args.config in ["debug", "all"]:
    run(f"conan install . {options} -s build_type=Debug --build=missing {config}")

# CMake version used
run("cmake --version")

# CMake configuration 
if platform.system() == "Windows":
    if args.python_root:
        run(f"cmake --preset {cmake_preset_name} {cmake_python_root}") 
    else:
        run("cmake --preset conan-default")
else:
    if args.config in ["release", "all"]:
        run(f"cmake --preset {cmake_preset_name}-release {cmake_python_root}")
    if args.config in ["debug", "all"]:
        run(f"cmake --preset {cmake_preset_name}-debug {cmake_python_root}")
   
# Build step
if args.config in ["release", "all"]:
    if args.sonar:
        if platform.system() == "Windows":
            run(f"build-wrapper-win-x86-64.exe --out-dir bw-output cmake --build --preset {cmake_preset_name}-release")
        else:
            run(f".sonar/build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir bw-output cmake --build --preset {cmake_preset_name}-release")
    else:
        run(f"cmake --build --preset {cmake_preset_name}-release")

if args.config in ["debug", "all"]:
    run(f"cmake --build --preset {cmake_preset_name}-debug")
