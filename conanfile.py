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
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.build import check_min_cppstd
from conan.errors import ConanInvalidConfiguration

# for installing missing python packages
import os
import re
import sys
import subprocess
from pathlib import Path

# Conan recipe class 
class TasClientApiRecipe(ConanFile):
    name = "tas_client_api"

    def set_version(self):
        cmakeFile = Path(os.path.join(self.recipe_folder, "CMakeLists.txt")) 
        pkgVersion = "x.x.x"
        with cmakeFile.open("r") as f:
            content = f.read()
            pkgVersion = re.search(r"project((.|\n)*)VERSION (.*)", content).group(3)
        
        self.version = pkgVersion.strip()

    package_type = "library"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False], 
        "fPIC": [True, False],
        "python": [True, False],
        "docs": [True, False],
        "tests": [True, False]
        }
    default_options = {
        "shared": False, 
        "fPIC": True,
        "python": False,
        "docs": False,
        "tests": False
        }

    # Source in same place as this recipe
    exports_sources = "apps/*", "cmake/*", "data/*", "docs/*", "python/*", "src/*", "CMakeLists.txt"

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def validate(self):
        check_min_cppstd(self, "17")        
        if self.settings.os == "Windows" and self.settings.arch != "x86_64":
            raise ConanInvalidConfiguration("Only 64-bit architecture supported on Windows.")

    def system_requirements(self):
        # define system dependencies
        packages = ["setuptools", "virtualenv", "wheel"]
        subprocess.check_call([sys.executable, "-m", "pip", "install"] + packages)        

    def requirements(self):
        # define dependencies
        self.requires("pybind11/2.10.4")

    def build_requirements(self):
        # define build tools dependencies
        self.tool_requires("cmake/3.27.0")

    def layout(self):
        cmake_layout(self)
        # This "includedirs" starts in the source folder, which is "." by default setting in cmake_layout                 
        self.cpp.source.components["tas_client"].includedirs = ["src/tas_client"]
        self.cpp.source.components["tas_socket"].includedirs = ["src/tas_socket"]
        # compiled libraries "libdirs" will be inside the "build" folder, depending
        # on the platform they will be in "build/Release" or directly in "build" folder
        if self.settings.os != "Windows":
            self.cpp.build.components["tas_client"].libdirs = ["src/tas_client"]
            self.cpp.build.components["tas_socket"].libdirs = ["src/tas_socket"]
        else:
            self.cpp.build.components["tas_client"].libdirs = ["src/tas_client/"+str(self.settings.build_type)]
            self.cpp.build.components["tas_socket"].libdirs = ["src/tas_socket/"+str(self.settings.build_type)]

    def generate(self):
        # convert conan variables into build-system files
        deps = CMakeDeps(self)         # -> creates FindXXX.cmake (sets paths to XXX lib files in conan cache)
        deps.generate()
        tc = CMakeToolchain(self)    # -> conantoolchain.cmake (variables translated from conan settings)        
        tc.cache_variables["TAS_CLIENT_API_BUILD_PYTHON"] = self.options.python
        tc.cache_variables["TAS_CLIENT_API_BUILD_DOCS"] = self.options.docs
        tc.cache_variables["TAS_CLIENT_API_BUILD_TEST"] = self.options.tests
        tc.generate()
        

    def build(self):
        # invoke the build system, reading generated files
        cmake = CMake(self)                 # CMake helper auto-formats CLI arguments for CMake
        cmake.configure()                   # cmake -DCMAKE_TOOLCHAIN_FILE=conantoolchain.cmake
        cmake.build()                       # cmake --build .

    def package(self):
        # copy artifacts from "build" to "package" directory
        cmake = CMake(self)                 # For CMake projects which define an install target, leverage it
        cmake.install()                     # cmake --build . --target=install
                                            # sets CMAKE_INSTALL_PREFIX to appropriate directory in conan cache

    def package_info(self):
        # declare whats in the package for consumers
        self.cpp_info.components["tas_client"].libs = ["tas_client"]
        self.cpp_info.components["tas_client"].requires = ["tas_socket"]
        self.cpp_info.components["tas_client"].includedirs = ["include/tas_client"]

        self.cpp_info.components["tas_socket"].libs = ["tas_socket"]
        self.cpp_info.components["tas_socket"].includedirs = ["include/tas_socket"]