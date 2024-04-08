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
import subprocess
from utilities import run

current = os.getcwd()
process = None
# start a TAS server
if platform.system() == "Windows":
    process = subprocess.Popen("C:\\Windows\\System32\\tas_server.exe", stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(process.pid)
    os.chdir(".\\build")

else:
    process = subprocess.Popen("/usr/local/bin/tas_server", stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    os.chdir("./build/Release/")

# Execute tests
run("ctest -C Release")

# kill the running TAS server
process.terminate()

os.chdir(current)