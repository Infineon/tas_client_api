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

# -----------------------------------------------------------------------------
# Source documentation 
# -----------------------------------------------------------------------------
if (NOT TAS_CLIENT_API_BUILD_DOCS)
    return()
endif()

find_package(Doxygen)
if(Doxygen_FOUND)
    # Doxygen settings
    # Get the user's home directory
    if(WIN32)
        set(HOME_DIR $ENV{USERPROFILE})
    else()
        set(HOME_DIR $ENV{HOME})
    endif()
    # Set the output directory to a subdirectory in the user's home directory
    set(DOCS_OUTPUT_DIR "${HOME_DIR}/tas_client_api_docs")

    set(DOCS_PROJECT_NAME "TAS Client API")
    set(DOCS_LOGO "${CMAKE_SOURCE_DIR}/data/icons/TAS_icon_small.png")
    set(DOCS_EXTRA_FILES "${CMAKE_SOURCE_DIR}/data/icons/TAS_icon_small.png")
    
    # Directories and files relevant for documentation
    list(APPEND DOCS_SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/mainpage.md)
    list(APPEND DOCS_SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/client_rw_api.md)
    list(APPEND DOCS_SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/client_chl_api.md)
    list(APPEND DOCS_SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/client_trc_api.md)
    list(APPEND DOCS_SRC_DIRS ${CMAKE_SOURCE_DIR}/src)
    list(APPEND DOCS_SRC_DIRS ${CMAKE_SOURCE_DIR}/apps)
    string(REPLACE ";" " \\\n" DOCS_SRC_DIRS "${DOCS_SRC_DIRS}")
    # Doxygen settings

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile) 

    # Doxyfile.in contains cmake variables e.g. @CMAKE_PROJECT_NAME@ 
    configure_file( ${doxyfile_in} ${doxyfile} @ONLY )

    add_custom_target(docs_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM 
    )
else()
    message(STATUS "${CMAKE_PROJECT_NAME}: Doxygen not found, building documentation is skipped")
endif()