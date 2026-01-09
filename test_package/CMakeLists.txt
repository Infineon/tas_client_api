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
cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(tas_client_api CONFIG REQUIRED)

add_executable(pkg_test src/main.cpp)
target_link_libraries(pkg_test tas_client_api::tas_client)
