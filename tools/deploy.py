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
import re
import platform
from pathlib import Path
from utilities import run, get_active_branch_name


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
print("Uploading conan package...")
run(f"conan create . --user=ifx --channel={pkgMaturity}")
if platform.system() == "Windows":
    run(f"conan create . --user=ifx --channel={pkgMaturity} -pr:b default -pr:h win32") # 32-bit Windows build
run(f"conan upload tas_client_api/{pkgVersion}@ifx/{pkgMaturity} -r=conan-atv-debug-tools-local")
print("...done!")

# Uploading TAS Client API python wrapper wheel
print("Uploading Python wheels...")
if gitBranch == "master":
    repo = "local"
else:
    repo = "local-dev"

python_vers = ["python39", "python310", "python311", "python312", "python313"]
for python_ver in python_vers:
    if platform.system() == "Windows":
        path = os.path.join("build", f"{python_ver}-tas_client_api", "python")
        if os.path.exists(path):
            run(f"build\\{python_ver}-tas_client_api\\python\\pytas_venv\\Scripts\\python -m twine upload -r {repo} build/{python_ver}-tas_client_api/python/dist/*.whl")
        else:
            print(f"Python wrapper not build for {python_ver}, skipped uploading the wheel.")

    else:
        path = os.path.join("build", f"{python_ver}-tas_client_api", "Release", "python")
        if os.path.exists(path):
            run(f"build/{python_ver}-tas_client_api/Release/python/pytas_venv/bin/python3 -m twine upload -r {repo} build/{python_ver}-tas_client_api/Release/python/dist/*.whl")
        else:
            print(f"Python wrapper not build for {python_ver}, skipped uploading the wheel.")
print("...done!")

# Restoring the environment
os.chdir(current)