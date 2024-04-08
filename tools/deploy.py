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
import re
import platform
from pathlib import Path
from utilities import run

# Helper function to get the current git branch name
def get_active_branch_name():
    head_dir = Path(".") / ".git" / "HEAD"
    with head_dir.open("r") as f: content = f.read().splitlines()

    for line in content:
        if line[0:4] == "ref:":
            return line.partition("refs/heads/")[2]

# Get the current working dir
current = os.getcwd()
print(f"cwd: {current}")

# Get the current active branch
gitBranch = get_active_branch_name()
print(f"Current git branch: {gitBranch}")

# Determine package maturity
pkgMaturity  = "dev"
if gitBranch == "master":
    pkgMaturity = "stable"

# Get teh package version from the top level CMakeLists file
# Prerequisite is that project definition includes the version property
cmakeFile = Path(".") / "CMakeLists.txt"
pkgVersion = "x.x.x"
with cmakeFile.open("r") as f:
    content = f.read()
    pkgVersion = re.search(r"project((.|\n)*)VERSION (.*)", content).group(3)
    print(f"Package version: {pkgVersion}")

# Create the TAS Client API package & upload it 
run(f"conan create . --user=ifx --channel={pkgMaturity}")
run(f"conan upload tas_client_api/{pkgVersion}@ifx/{pkgMaturity} -r=conan-atv-debug-tools-local")

# Uploading TAS Client API python wrapper wheel
if platform.system() == "Windows":
    if os.path.exists(".\\build\\python\\"):
        os.chdir(".\\build\\python\\")
        run("python setup.py bdist_wheel upload -r local")
    else:
        print("Python wrapper not build, skipped uploading the wheel.")

else:
    if os.path.exists("./build/Release/python/"):
        os.chdir("./build/Release/python/")
        run("python3 setup.py bdist_wheel upload -r local")
    else:
        print("Python wrapper not build, skipped uploading the wheel.")

# Restoring the environment
os.chdir(current)