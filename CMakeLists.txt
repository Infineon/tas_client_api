# -----------------------------------------------------------------------------
# Disable in-source builds to prevent source tree corruption.
# -----------------------------------------------------------------------------
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "FATAL: In-source builds are not allowed. 
        You should create a seperate directory for build files.")
endif()
# -----------------------------------------------------------------------------
# Minimum supported versions
# -----------------------------------------------------------------------------
include(cmake/TasClientApiMinDepVersions.cmake)
cmake_minimum_required(VERSION "${MIN_VER_CMAKE}" FATAL_ERROR)

# -----------------------------------------------------------------------------
# Project definition
# -----------------------------------------------------------------------------
project(tas_client_api
    VERSION 1.0.0
    DESCRIPTION "Infineon's Tool Access Socket (TAS) Client API"
    HOMEPAGE_URL "https://www.infineon.com/DAS"
    LANGUAGES CXX
)
set(TAS_CLIENT_COPYRIGHT "(c) Infineon Technologies AG 2023")

# -----------------------------------------------------------------------------
# Break in case of popular CMake configuration mistakes
# -----------------------------------------------------------------------------
if(NOT CMAKE_SIZEOF_VOID_P GREATER 0)
    message(FATAL_ERROR "CMake fails to determine the bitness of the target 
    platform. Please check your CMake and compiler installation. If you are 
    cross-compiling then ensure that your CMake toolchain file correctly sets 
    the compiler details.")
endif()

# -----------------------------------------------------------------------------
# Project options
# -----------------------------------------------------------------------------
option(TAS_CLIENT_API_BUILD_PYTHON "Build Python interface if enabled" OFF)
option(TAS_CLIENT_API_BUILD_TEST "Build test binaries if enabled" OFF)
option(TAS_CLIENT_API_BUILD_DOCS "Build documentation using doxygen if enabled" OFF)

# -----------------------------------------------------------------------------
# Soruce and test files
# -----------------------------------------------------------------------------
include(cmake/TasClientApiUtils.cmake)
include(GNUInstallDirs)

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(CTest)

add_subdirectory(apps/tas_rw_api_demo)
add_subdirectory(apps/tas_chl_api_demo)
add_subdirectory(python)
add_subdirectory(src)
add_subdirectory(docs)

# -----------------------------------------------------------------------------
# Install
# -----------------------------------------------------------------------------
# export targets
install(EXPORT TasClientApiTargets 
    FILE TasClientApiTargets.cmake
    NAMESPACE TasClientApi::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TasClientApi
)

# export targets to be used from a build directory
export(EXPORT TasClientApiTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/TasClientApiTargets.cmake"
    NAMESPACE TasClientApi::
)

# Package definition
# provides configure_package_config_file() which enables creation of relocatable packages. 
include(CMakePackageConfigHelpers)

# Package configuration file
configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/templates/TasClientApiConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/TasClientApiConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TasClientApi
)

# Package version file
# If no specific version set, it takes PROJECT_VERSION
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/TasClientApiConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TasClientApiConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TasClientApiConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TasClientApi
)