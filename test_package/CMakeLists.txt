cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(tas_client_api CONFIG REQUIRED)

add_executable(pkg_test src/main.cpp)
target_link_libraries(pkg_test tas_client_api::tas_client)
