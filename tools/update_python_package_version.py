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
import sys
import toml
from utilities import get_active_branch_name

def add_dev_suffix_to_version(pyproject_file):
    try:
        # Read the pyproject.toml file
        with open(pyproject_file, 'r') as file:
            pyproject_data = toml.load(file)

        # Get the version and add "dev" suffix
        version = pyproject_data['project']['version']

        if not version.endswith('dev'):
            version += 'dev'
            pyproject_data['project']['version'] = version

        # Write updated data back to the pyproject.toml file
        with open(pyproject_file, 'w') as file:
            toml.dump(pyproject_data, file)

        print(f"Updated version to: {version}")

    except FileNotFoundError:
        print(f"Error: The file {pyproject_file} does not exist.")
    except KeyError as e:
        print(f"Error: Missing expected key in the TOML file - {e}")
    except toml.TomlDecodeError as e:
        print(f"Error: Failed to decode TOML file - {e}")

if __name__ == "__main__":
    if get_active_branch_name() == "develop":
        if len(sys.argv) != 2:
            print("Usage: python update_version.py <path_to_pyproject.toml>")
        else:
            pyproject_file = sys.argv[1]
            add_dev_suffix_to_version(pyproject_file)
    else:
        print("Nothing to do, exiting...")