# -----------------------------------------------------------------------------
# list of sources:
# -----------------------------------------------------------------------------
set(TAS_SOCKET_HDRS
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_conn_socket.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_socket.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_tcp_server_socket.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_tcp_socket.h"
)

set(TAS_SOCKET_SRCS
    "${TAS_SOCKET_HDRS}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_conn_socket.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_socket.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_tcp_server_socket.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_tcp_socket.cpp"
)

# -----------------------------------------------------------------------------
# include soruces in the following targets:
# -----------------------------------------------------------------------------

# Python wrapper
tac_add_target_sources(PyTAS "${TAS_SOCKET_SRCS}")

# -----------------------------------------------------------------------------
# tas socket lib
# -----------------------------------------------------------------------------
set(LIB_NAME tas_socket)

# generate IDE virtual folders where supported
set(REC ".*([.]cpp|[.]h)")

set(REG1 ".*/src/tas_socket/")
source_group("tas_socket" REGULAR_EXPRESSION "${REG1}${REC}")

# -----------------------------------------------------------------------------
# Add executable, its includes, and libraries
# -----------------------------------------------------------------------------
add_library(${LIB_NAME} STATIC ${TAS_SOCKET_SRCS})

target_include_directories(${LIB_NAME}
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tas_socket>"
)

# -----------------------------------------------------------------------------
# Compile definitions
# -----------------------------------------------------------------------------
if (MSVC)
    target_compile_definitions(${LIB_NAME} PRIVATE
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
    target_compile_definitions(${LIB_NAME} PRIVATE
        "UNIX"
    )
endif()

# -----------------------------------------------------------------------------
# Compile and link options
# -----------------------------------------------------------------------------
if (MSVC)
    target_compile_options(${LIB_NAME} PRIVATE
        /W3
        /MP
        "$<$<CONFIG:Release>:"
            "/O2"
        ">"
    )

    target_link_options(${LIB_NAME} PRIVATE
        /SUBSYSTEM:CONSOLE
    )
elseif (UNIX)
    target_compile_options(${LIB_NAME} PRIVATE
        -Wall;
    )
endif()

# -----------------------------------------------------------------------------
# Install
# -----------------------------------------------------------------------------
# library
install(TARGETS ${LIB_NAME} 
    EXPORT TasClientApiTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" 
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tas_socket"
)

# header files
install(FILES ${TAS_SOCKET_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tas_socket)
