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
import subprocess
import re
from pathlib import Path

def run(cmd, error=False):
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
    output = ""

    print("Running: {}".format(cmd))
    print("----- OUTPUT -------")
    while True:
        out = process.stdout.readline()
        out = out.decode("utf-8")

        ret = process.poll()
        if out == "" and ret is not None:
            break

        if out != "":            
            output += out
            print(out.rstrip(os.linesep))

    print("----END OUTPUT------")
    print()

    if ret != 0 and not error:
        raise Exception("Failed cmd: {}\n".format(cmd))
    if ret == 0 and error:
        raise Exception(
            "Cmd succeded (failure expected): {}\n".format(cmd))

    return output

def get_project_version(cmake_file):
    cmakeFile = Path(cmake_file) 
    with cmakeFile.open("r") as f:
        content = f.read()
        pkgVersion = re.search(r"project((.|\n)*)VERSION (.*)", content).group(3)
    return pkgVersion