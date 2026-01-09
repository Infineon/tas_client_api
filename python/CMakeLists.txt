#[[
 Copyright (c) 2026 Infineon Technologies AG.

 This file is part of TAS Client, an API for device access for Infineon's 
 automotive MCUs. 

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ****************************************************************************************************************]]

# Check if python wrapper should be built
if (NOT TAS_CLIENT_API_BUILD_PYTHON)
    return()
endif()

# -----------------------------------------------------------------------------
# Python project definition
# -----------------------------------------------------------------------------
set(PYTHON_PROJECT PyTAS)
set(PYTHON_PROJECT_VERSION ${CMAKE_PROJECT_VERSION})
set(PYTHON_PROJECT_DESCRIPTION "Python wrapper for the TAS Client API")
message(STATUS "Python project: ${PYTHON_PROJECT}")
# Detect the platform
if(WIN32)
    set(OS_CLASSIFIER "Operating System :: Microsoft :: Windows")
elseif(UNIX)
    if(APPLE)
        set(OS_CLASSIFIER "Operating System :: MacOS")
    else()
        set(OS_CLASSIFIER "Operating System :: POSIX :: Linux")
    endif()
else()
    set(OS_CLASSIFIER "Operating System :: Other OS")
endif()

set(PYTHON_PROJECT_DIR ${PROJECT_BINARY_DIR}/python/src/${PYTHON_PROJECT})
message(STATUS "Python project build path: ${PYTHON_PROJECT_DIR}")

# These are pymcds package dependencies
set(DEPENDENCIES_LIST "numpy")

set(PYTHON_PROJECT_DEPENDENCIES "")
foreach(ITEM IN LISTS DEPENDENCIES_LIST)
    # Append the quoted item to the new list
    list(APPEND PYTHON_PROJECT_DEPENDENCIES "\"${ITEM}\"")
endforeach(ITEM IN LISTS )

string(REPLACE ";" "," PYTHON_PROJECT_DEPENDENCIES "${PYTHON_PROJECT_DEPENDENCIES}")

# -----------------------------------------------------------------------------
# Find relevant dependencies
# -----------------------------------------------------------------------------
# Uncomment and spcify the root of Python installation if specific version should be used, otherwise the system default 
# is selected 
if (WIN32)
    #set(Python_ROOT_DIR "C:\\Program Files\\Python311")
elseif (UNIX)
    #set(Python3_ROOT_DIR "/usr/bin/python3.11")
endif()

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

if (NOT pybind11_FOUND)
    message(FATAL_ERROR "pybind11 not found ${pybind11_FOUND}. Dependecies managed by conan, run 'conan install . -o python=True' first")
endif()

message(STATUS "Python executable found by pybind11: ${Python_EXECUTABLE}")

# ================================ General configuration ======================================
# Solution for an open issue due to installed python debug libs: https://github.com/pybind/pybind11/issues/3403
set_target_properties(Python::Module PROPERTIES
        MAP_IMPORTED_CONFIG_DEBUG ";RELEASE")

# Setup virtual pyton environment for testing and stub gen
# Setup variables for targeting a virtual environment
set(VENV_DIR "${CMAKE_CURRENT_BINARY_DIR}/pytas_venv")
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(VENV_BIN "${VENV_DIR}/Scripts")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(VENV_BIN "${VENV_DIR}/bin")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Create virtual environment
if(NOT EXISTS ${VENV_DIR})
    message(STATUS "Creating virtual environment...")
    execute_process(
        COMMAND ${Python_EXECUTABLE} -m venv ${VENV_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    cmake_path(GET Python_EXECUTABLE FILENAME Python_EXECUTABLE_NAME)
    find_program(Python_VENV_EXECUTABLE NAMES ${Python_EXECUTABLE_NAME} PATHS ${VENV_BIN} NO_CACHE NO_DEFAULT_PATH)
    message(STATUS "Python executable in venv: ${Python_VENV_EXECUTABLE}")

    if(NOT Python_VENV_EXECUTABLE)
        message(FATAL_ERROR "Python executable not found in virtual environment: ${Python_VENV_EXECUTABLE}")
    endif()

    # Windows-specific: run activate.bat before pip commands
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        execute_process(
            COMMAND cmd /c "${VENV_DIR}\\Scripts\\activate && ${Python_VENV_EXECUTABLE} -m pip install --upgrade pip"
            RESULT_VARIABLE PIP_UPGRADE_RESULT
            ERROR_VARIABLE PIP_UPGRADE_ERROR
            OUTPUT_VARIABLE PIP_UPGRADE_OUTPUT
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    else()
        # macOS/Linux: source activate and run pip commands
        execute_process(
            COMMAND /bin/bash -c "source ${VENV_DIR}/bin/activate && ${Python_VENV_EXECUTABLE} -m pip install --upgrade pip"
            RESULT_VARIABLE PIP_UPGRADE_RESULT
            ERROR_VARIABLE PIP_UPGRADE_ERROR
            OUTPUT_VARIABLE PIP_UPGRADE_OUTPUT
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()

    if(NOT PIP_UPGRADE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to upgrade pip. See error: ${PIP_UPGRADE_ERROR}")
    endif()

    # Install required Python packages
    message(STATUS "Venv exe: ${Python_VENV_EXECUTABLE}")
    execute_process(
        COMMAND ${Python_VENV_EXECUTABLE} -m pip install pip-system-certs
        COMMAND ${Python_VENV_EXECUTABLE} -m pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt  
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

cmake_path(GET Python_EXECUTABLE FILENAME Python_EXECUTABLE_NAME)
find_program(Python_VENV_EXECUTABLE NAMES ${Python_EXECUTABLE_NAME} PATHS ${VENV_BIN} NO_CACHE NO_DEFAULT_PATH)
message(STATUS "Python executable in venv: ${Python_VENV_EXECUTABLE}")

if(NOT Python_VENV_EXECUTABLE)
    message(FATAL_ERROR "Python executable not found in virtual environment: ${Python_VENV_EXECUTABLE}")
endif()


# =============================== CMake target - bindings_library =============================
pybind11_add_module(PyTAS
        binding/tas_python_binding.cpp
        binding/tas_python_binding.hpp
        binding/tas_python_client_server_con.cpp
        binding/tas_python_client_server_con.hpp
        binding/tas_python_client_rw_base.cpp
        binding/tas_python_client_rw_base.hpp
        binding/tas_python_client_rw.hpp
        binding/tas_python_client_chl.cpp
        binding/tas_python_client_chl.hpp
)

target_include_directories(PyTAS 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/binding"
)



# -----------------------------------------------------------------------------
# Python package
# -----------------------------------------------------------------------------
file(GENERATE OUTPUT ${PYTHON_PROJECT_DIR}/__init__.py CONTENT "__version__ = \"${PROJECT_VERSION}\"\n\nfrom PyTAS import *")

# pyproject.toml.in contains cmake variable e.g. @PYTHON_PROJECT@ and
# generator expression e.g. $<TARGET_FILE_NAME:pyFoo>
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/templates/pyproject.toml.in
    ${PROJECT_BINARY_DIR}/python/pyproject.toml.in
    @ONLY
)

# This step is required to trigger the generator expressions
file(GENERATE
    OUTPUT ${PROJECT_BINARY_DIR}/python/pyproject.toml
    INPUT ${PROJECT_BINARY_DIR}/python/pyproject.toml.in
)

# Add a custom command to run the Python script after generating pyproject.toml
add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/python/pyproject.toml  # Output file
    COMMAND ${Python_VENV_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/update_python_package_version.py ${PROJECT_BINARY_DIR}/python/pyproject.toml
    DEPENDS 
        ${PROJECT_BINARY_DIR}/python/pyproject.toml.in  # Dependency file
        ${CMAKE_CURRENT_BINARY_DIR}/stub_gen_done
    COMMENT "Running update_python_package_version.py to add dev suffix to version"
    VERBATIM
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
# -----------------------------------------------------------------------------

# Prepare the python package layout in the ${PYTHON_PROJECT_DIR} directory 
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/copy_done

    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_PROJECT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_PROJECT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyTAS> ${PYTHON_PROJECT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${PROJECT_BINARY_DIR}/python
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${PROJECT_BINARY_DIR}/python
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/setup.py ${PROJECT_BINARY_DIR}/python
    COMMAND ${CMAKE_COMMAND} -E touch ${PROJECT_BINARY_DIR}/__init__.py

    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/copy_done

    DEPENDS
        PyTAS
        ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND_EXPAND_LISTS
)
# -----------------------------------------------------------------------------

# Stub file generation
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stub_gen_done

    COMMAND ${Python_VENV_EXECUTABLE} -m pybind11_stubgen
        -o ${CMAKE_CURRENT_BINARY_DIR}/src
        PyTAS.PyTAS
        --enum-class-locations "TasChso:PyTAS.PyTAS.TasChso"
        --enum-class-locations "TasDefaultPorts:PyTAS.PyTAS.TasDefaultPorts"
        --enum-class-locations "TasChlTarget:PyTAS.PyTAS.TasChlTarget"
        --enum-class-locations "TasTrcType:PyTAS.PyTAS.TasTrcType"
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/stub_gen_done

    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/copy_done

    BYPRODUCTS 
        ${PYTHON_PROJECT_DIR}/PyTAS.pyi

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src
)
# -----------------------------------------------------------------------------

# Python package generation
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/python_package_created

    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/dist
    COMMAND ${Python_VENV_EXECUTABLE} -m build
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/python_package_created

    DEPENDS 
        ${CMAKE_CURRENT_BINARY_DIR}/stub_gen_done
        ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml

    BYPRODUCTS
        ${PYTHON_PROJECT}
        ${PYTHON_PROJECT}.egg-info
        ${CMAKE_CURRENT_BINARY_DIR}/dist

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
# -----------------------------------------------------------------------------

add_custom_target(python_package ALL
    DEPENDS 
        ${CMAKE_CURRENT_BINARY_DIR}/python_package_created
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Python tests
# -----------------------------------------------------------------------------
# Check if the Python executable exists
if(NOT Python_VENV_EXECUTABLE)
    message(FATAL_ERROR "Python executable not found in virtual environment.")
endif()

# Install the .whl file using a custom script to ensure platform independence
file(WRITE ${CMAKE_BINARY_DIR}/install_whl.py
"import os
import subprocess
import glob

whl_dir = os.path.join('${CMAKE_CURRENT_BINARY_DIR}', 'dist')
whl_files = glob.glob(os.path.join(whl_dir, '*.whl'))

if whl_files:
    for whl_file in whl_files:
        print(f'Installing {whl_file}')
        subprocess.check_call(['${Python_VENV_EXECUTABLE}', '-m', 'pip', 'install', '--force-reinstall', '--no-deps', '--upgrade', whl_file])
else:
    raise FileNotFoundError('No .whl file found in the specified directory')
")

add_custom_command(TARGET python_package POST_BUILD
    COMMAND ${Python_VENV_EXECUTABLE} ${CMAKE_BINARY_DIR}/install_whl.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Installing ${PYTHON_PROJECT}"
    VERBATIM
)

# add_python_test()
# CMake function to generate and build python test.
# Parameters:
#  the python filename
# e.g.:
# add_python_test(foo.py)
function(add_python_test FILE_NAME)
  message(STATUS "Configuring test ${FILE_NAME} ...")
  get_filename_component(EXAMPLE_NAME ${FILE_NAME} NAME_WE)

  if(BUILD_TESTING)
    add_test(
      NAME python_test_${EXAMPLE_NAME}
      COMMAND ${Python_VENV_EXECUTABLE} ${FILE_NAME}
      WORKING_DIRECTORY ${VENV_DIR})
  endif()
  message(STATUS "Configuring test ${FILE_NAME} done")
endfunction()

add_python_test(${CMAKE_CURRENT_SOURCE_DIR}/tests/tas_python_client_rw_test.py)
add_python_test(${CMAKE_CURRENT_SOURCE_DIR}/tests/tas_python_client_server_test.py)