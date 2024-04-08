# -----------------------------------------------------------------------------
# Source documentation 
# -----------------------------------------------------------------------------
if (NOT TAS_CLIENT_API_BUILD_DOCS)
    return()
endif()

find_package(Doxygen)
if(Doxygen_FOUND)
    # Doxygen settings
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