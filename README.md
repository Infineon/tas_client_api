# Tool Access Socket (TAS) Client API source code
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/Infineon/tas_client_api/commits/master/)
[![PRs welcome](https://img.shields.io/badge/PRs-welcome-brightgreen)](https://github.com/Infineon/tas_client_api/pulls)

This readme file gives an overview how to build and run the code. 

## Prerequisite for building locally
This project uses [conan](https://conan.io) as dependency manager and consequently CMake as the build environment.
All required packages and build tools are retrieved from the [ConanCenter](https://conan.io/center). See conanfile.py 
for the list of dependencies. Therefore, to build this project you will need: 

- [Python](https://www.python.org/), a version that supports Conan2,
- [CMake](https://cmake.org/), minimum version 3.25
- Target compiler: msvc, gcc, etc.
- [Doxygen](https://www.doxygen.nl/), if you want to build the API reference documentation

### Prerequisite for building the python wrapper
Python packages: setuptools, wheel, virtualenv

### Conan
If not already installed, install conan: 
```
python -m pip install conan
```

If it is your first time using conan, you first need to generate a default profile:
```
conan profile detect
```
The generated profile can be then found under *\<home_dir\>/.conan2/profiles*. The default configuration is Release.
More information about conan profiles can be found here https://docs.conan.io/2.0/reference/config_files/profiles.html.
Edit the default conan profile with your favorite text editor so that the **C++17 standard** is used.
```
compiler.cppstd=17
```
Note: make sure your compiler supports C++17.

## Building
First generate the build environment by invoking conan **from the root of this repository**:
```
conan install .
conan install . -s build_type=Debug --build=missing
```
The above step configures the environment in way that only the C++ API will be build. If you want to build the python 
wrapper, the API reference, and/or the tests then you need to specify this by providing an option to the above commands.
See the conanfile.py for available options.
```
conan install . -o python=True -o docs=True -o tests=True
```
By default all options are disabled, i.e. False

### Invoking CMake
Things generally happen in two phases with CMake:
 * Configure the build based on the CMakeLists.txt
 * Perform the build based on the toolchain of your platform (typically msvc on windows and make on Linux)

The project uses **CMakePresets.json** to define global configuration, build, and test presets.

If using Visual Studio refer to [Configure and build with CMake Presets in Visual Studio](https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-170).

If using Visual Studio Code refer to [Configure and build with CMake Presets in Visual Studio Code](https://github.com/microsoft/vscode-cmake-tools/blob/main/docs/cmake-presets.md).

Conan will generate some default cmake presets. For multi-config generators like msvc and ninja, this will be conan-default. 
For single-config generators like Unix Makefiles, this will be conan-release and conan-debug.

**Windows**
```
# Assuming Visual Studio 17 2022 is your VS version and that it matches your default profile
cmake --preset conan-default
# Note: if you have problems during consecutive runs, then specify your platform name and the toolset
cmake --preset conan-default -A x64 -T v143 # In case of VS201: -T v142
```
At this point a Visual Studio solution file is generated, and can be used instead of CLI commands. The solution file is
located under the build folder.

**Linux**
```
cmake --preset conan-release
cmake --preset conan-debug # Assuming conan install . -s build_type=Debug was run prior to this
```

Followed by the build step: 
```
cmake --build --preset conan-release # Builds the release version
cmake --build --preset conan-debug # Builds the debug version, Assuming cmake --preset conan-debug was run prior to this
```
Note: The conan-debug preset will be available only if conan install with build_type set to Debug was run beforehand.

### Using Conan (not recommended)
```
conan build .
```

## Building using a build script
Alternative you can use the CI build script for building the project. For available options run the script with the help
flag (-h/--help). Build the project by invoking the CI build script with required options.

**Windows**
```
python .\tools\build.py
```
**Linux**
```
python3 ./tools/build.py
```

## Installing / exporting binaries
The project is configured in way that the binaries can be installed to a specified location. Use the following command:
```
cmake --install build --prefix=install
```
where build is the build directory, and prefix options defines the root install / export folder.

## Python wrapper
Building the python wrapper using the above steps requires -o python=True conan option when invoking the conan install command.

If you want to use a specific Python version or you have multiple version installed you can tell CMake which one to use
by modifying the following CMake file: python\CMakeLists.txt before executing the above commands.
```
set(Python3_ROOT_DIR "<system path to python root>")
```

The generated python wheel will be located under: build\python\dist
