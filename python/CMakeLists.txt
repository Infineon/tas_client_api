# Check if python wrapper should be built
if (NOT TAS_CLIENT_API_BUILD_PYTHON)
    return()
endif()

# Uncomment and spcify the root of Python installation if specific version should be used, otherwise the system default 
# is selected 
if (WIN32)
    #set(Python3_ROOT_DIR "C:\\Program Files\\Python311")
elseif (UNIX)
    #set(Python3_ROOT_DIR "/usr/bin/python3.11")
endif()

# Look for python executable
find_package(Python3 COMPONENTS Interpreter Development)
# Based on pybind11 this should be after find_package(Python3). However, with Python debug libs installed there is an issues. 
find_package(pybind11 CONFIG)

# Solution for an open issue due to installed python debug libs: https://github.com/pybind/pybind11/issues/3403
set_target_properties(Python3::Module PROPERTIES
        MAP_IMPORTED_CONFIG_DEBUG ";RELEASE")

if (NOT pybind11_FOUND)
    message(FATAL_ERROR "pybind11 not found ${pybind11_FOUND}. Dependecies managed by conan, run 'conan install .' first")
endif()

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
        binding/tas_python_client_trc.cpp
        binding/tas_python_client_trc.hpp
)

target_include_directories(PyTAS 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/binding"
)

set(PYTHON_PROJECT PyTAS)
message(STATUS "Python project: ${PYTHON_PROJECT}")
set(PYTHON_PROJECT_DIR ${PROJECT_BINARY_DIR}/python/src/${PYTHON_PROJECT})
message(STATUS "Python project build path: ${PYTHON_PROJECT_DIR}")

# -----------------------------------------------------------------------------
# Python package
# -----------------------------------------------------------------------------
file(GENERATE OUTPUT ${PYTHON_PROJECT_DIR}/__init__.py CONTENT "__version__ = \"${PROJECT_VERSION}\"\n\nfrom PyTAS import *")

# setup.py.in contains cmake variable e.g. @PYTHON_PROJECT@ and
# generator expression e.g. $<TARGET_FILE_NAME:pyFoo>
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/templates/setup.py.in
    ${PROJECT_BINARY_DIR}/python/setup.py.in
    @ONLY
)

# This step is required to trigger the generator expressions
file(GENERATE
    OUTPUT ${PROJECT_BINARY_DIR}/python/setup.py
    INPUT ${PROJECT_BINARY_DIR}/python/setup.py.in
)

add_custom_command(
    OUTPUT dist/timestamp
    COMMAND ${CMAKE_COMMAND} -E remove_directory dist
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_PROJECT_DIR}/.libs
   
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyTAS> ${PYTHON_PROJECT_DIR}

    COMMAND ${Python3_EXECUTABLE} setup.py sdist bdist_wheel
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/dist/timestamp

    DEPENDS
        setup.py

    BYPRODUCTS
        ${PYTHON_PROJECT}
        ${PYTHON_PROJECT}.egg-info
        build
        dist

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND_EXPAND_LISTS
)

add_custom_target(python_package ALL
    DEPENDS
        dist/timestamp
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/python
)

# -----------------------------------------------------------------------------
# Python tests
# -----------------------------------------------------------------------------
set(VENV_EXECUTABLE ${Python3_EXECUTABLE} -m virtualenv)
set(VENV_DIR ${CMAKE_CURRENT_BINARY_DIR}/venv)
if(WIN32)
    set(VENV_Python3_EXECUTABLE ${VENV_DIR}/Scripts/python.exe)
else()
    set(VENV_Python3_EXECUTABLE ${VENV_DIR}/bin/python)
endif()

add_custom_command(TARGET python_package POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${VENV_DIR}
    COMMAND ${VENV_EXECUTABLE} -p ${Python3_EXECUTABLE} ${VENV_DIR}
    COMMAND ${VENV_Python3_EXECUTABLE} -m pip install numpy
    COMMAND ${VENV_Python3_EXECUTABLE} -m pip install --find-links=${CMAKE_CURRENT_BINARY_DIR}/dist ${PYTHON_PROJECT}
    BYPRODUCTS ${VENV_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Create venv and install ${PYTHON_PROJECT}"
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
      COMMAND ${VENV_Python3_EXECUTABLE} ${FILE_NAME}
      WORKING_DIRECTORY ${VENV_DIR})
  endif()
  message(STATUS "Configuring test ${FILE_NAME} done")
endfunction()

add_python_test(${CMAKE_CURRENT_SOURCE_DIR}/tests/tas_python_client_rw_test.py)
add_python_test(${CMAKE_CURRENT_SOURCE_DIR}/tests/tas_python_client_server_test.py)