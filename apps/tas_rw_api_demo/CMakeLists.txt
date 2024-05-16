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