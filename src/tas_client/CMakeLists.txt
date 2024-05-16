# -----------------------------------------------------------------------------
# list of sources:
# -----------------------------------------------------------------------------
set(TAS_HDRS
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_am15_am14.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client_chl.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client_impl.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client_rw_base.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client_rw.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client_server_con.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client_trc.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_debug.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_device_family.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_handler_base.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_handler_chl.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_handler_rw.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_handler_server_con.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_handler_trc.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_mailbox_if.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_mailbox_socket.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_utils_ifx.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_utils_jtag.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_utils_client.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_utils.h"
)

set(TAS_SRCS
    "${TAS_HDRS}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client_chl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client_rw_base.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client_server_con.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_client_trc.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_handler_base.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_handler_chl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_handler_rw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_handler_server_con.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_handler_trc.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_pkt_mailbox_socket.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_utils_ifx.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_utils_jtag.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_utils_os.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tas_utils.cpp"
)

# -----------------------------------------------------------------------------
# include soruces in the following targets:
# -----------------------------------------------------------------------------
# Python wrapper
tac_add_target_sources(PyTAS "${TAS_SRCS}")

# -----------------------------------------------------------------------------
# tas socket lib
# -----------------------------------------------------------------------------
set(LIB_NAME tas_client)

# generate IDE virtual folders where supported
set(REC ".*([.]cpp|[.]h)")

set(REG1 ".*/src/tas_client/")
source_group("tas_client" REGULAR_EXPRESSION "${REG1}${REC}")

# -----------------------------------------------------------------------------
# Add executable, its includes, and libraries
# -----------------------------------------------------------------------------
add_library(${LIB_NAME} STATIC ${TAS_SRCS})

target_include_directories(${LIB_NAME}
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tas_client>"
)

target_link_libraries(${LIB_NAME} tas_socket)

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
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tas_client"
)

# header files
install(FILES ${TAS_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tas_client)
