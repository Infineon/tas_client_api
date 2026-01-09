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
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.build import check_min_cppstd
from conan.errors import ConanInvalidConfiguration
from conan.tools.env import VirtualRunEnv, VirtualBuildEnv

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
        if self.settings.os != "Windows" and self.settings.arch == "x86":
            raise ConanInvalidConfiguration("32-bit (x86) architecture not supported on non-Windows platforms.")

    def system_requirements(self):
        # define system dependencies
        packages = ["setuptools", "virtualenv", "wheel"]
        subprocess.check_call([sys.executable, "-m", "pip", "install"] + packages)        

    def requirements(self):
        # define dependencies
        if self.options.python:
            self.requires("pybind11/2.13.5")

    def build_requirements(self):
        # define build tools dependencies
        pass

    def layout(self):
        cmake_layout(self)
        # This "includedirs" starts in the source folder, which is "." by default setting in cmake_layout                 
        self.cpp.source.components["tas_client"].includedirs = ["src/tas_client"]
        self.cpp.source.components["tas_socket"].includedirs = ["src/tas_socket"]
        # Support both single-config and multi-config CMake generators by advertising
        # both possible output locations (non-existent libdirs are ignored by consumers).
        build_type = str(self.settings.build_type)
        self.cpp.build.components["tas_client"].libdirs = [
            "src/tas_client",
            f"src/tas_client/{build_type}",
        ]
        self.cpp.build.components["tas_socket"].libdirs = [
            "src/tas_socket",
            f"src/tas_socket/{build_type}",
        ]

    def generate(self):
        # convert conan variables into build-system files
        deps = CMakeDeps(self)         # -> creates FindXXX.cmake (sets paths to XXX lib files in conan cache)
        deps.generate()
        tc = CMakeToolchain(self)    # -> conantoolchain.cmake (variables translated from conan settings)

        # Customize preset names based on architecture
        if self.settings.arch == "x86":
            tc.presets_prefix = "conan-build32"
            
        tc.cache_variables["TAS_CLIENT_API_BUILD_PYTHON"] = self.options.python
        tc.cache_variables["TAS_CLIENT_API_BUILD_DOCS"] = self.options.docs
        tc.cache_variables["TAS_CLIENT_API_BUILD_TEST"] = self.options.tests
        tc.generate()

        re = VirtualRunEnv(self)
        re.generate()
        # Generate .env file for Visual Studio Code debug environment
        result = []
        envvars = re.vars()
        for name, value in envvars.items():
            result.append("{}={}".format(name, value))
        
        content = "\n".join(result)        
        open(re.basename + "-" + str(self.settings.build_type).lower() + ".env", "w", encoding="utf-8").write(content)

        be = VirtualBuildEnv(self)
        be.generate()
        result = []
        envvars = be.vars()
        for name, value in envvars.items():
            result.append("{}={}".format(name, value))
            
        content = "\n".join(result)        
        open(be.basename + "-" + str(self.settings.build_type).lower() + ".env", "w", encoding="utf-8").write(content)
        

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

        if self.settings.os == "Windows":
            self.cpp_info.components["tas_socket"].system_libs.append("ws2_32")
