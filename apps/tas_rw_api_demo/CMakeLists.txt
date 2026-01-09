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
# tas_rw_api_demo
# -----------------------------------------------------------------------------
set(EXE_NAME tas_rw_api_demo)

# -----------------------------------------------------------------------------
# Relevant source files and their virtual folders for IDE (source groups)
# -----------------------------------------------------------------------------
set(NO_GROUP_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_rw_api_demo_main.cpp"
)

# generate IDE virtual folders where supported
source_group("" FILES ${NO_GROUP_SRCS})

# -----------------------------------------------------------------------------
# Find relevant dependencies
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Add executable, its includes, and libraries
# -----------------------------------------------------------------------------
add_executable(${EXE_NAME}
    ${NO_GROUP_SRCS}
)

target_link_libraries(${EXE_NAME} tas_client)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Compile definitions
# -----------------------------------------------------------------------------
if (MSVC)
    target_compile_definitions(${EXE_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CRT_SECURE_NO_WARNINGS"
        "_WIN32"
    )
elseif (UNIX)
    target_compile_definitions(${EXE_NAME} PRIVATE
        "UNIX"
    )
endif()

# -----------------------------------------------------------------------------
# Compile and link options
# -----------------------------------------------------------------------------
if (MSVC)
    target_compile_options(${EXE_NAME} PRIVATE
        /W3
        /MP
        "$<$<CONFIG:Release>:"
            "/O2"
        ">"
    )

    target_link_options(${EXE_NAME} PRIVATE
        /SUBSYSTEM:CONSOLE
    )
elseif (UNIX)
    target_compile_options(${EXE_NAME} PRIVATE
        -Wall;
    )
    target_link_libraries(${EXE_NAME} pthread dl)
endif()

# -----------------------------------------------------------------------------
# Install
# -----------------------------------------------------------------------------
install(TARGETS ${EXE_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT applications)