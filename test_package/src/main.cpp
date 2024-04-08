/*
 *  Copyright (c) 2024 Infineon Technologies AG.
 *
 *  This file is part of TAS Client, an API for device access for Infineon's 
 *  automotive MCUs. 
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  **************************************************************************************************************** */

//********************************************************************************************************************
//------------------------------------------------------Includes------------------------------------------------------
//********************************************************************************************************************
#include "tas_client_rw.h"
#include "tas_utils.h"
#include "tas_device_family.h"

#include <cstdio>
#include <cstring>

//********************************************************************************************************************
//----------------------------------------------------- Main ---------------------------------------------------------
//********************************************************************************************************************
int main(int argc, char** argv)
{
    printf("TAS API demo\n");

    // Create an instance of TAS Client RW 
    CTasClientRw clientRw("DemoClientRw");

    
    // Connect to the server, provide IP address or localhost
    tas_return_et ret; // TAS return value 
    ret = clientRw.server_connect("localhost");
    // Expected to fail, but this is ok, only for testing if conan package is created properly 
    if (ret != TAS_ERR_NONE) {
        printf("Failed to connect to the server, %s\n", clientRw.get_error_info());
    }    

    return 0;
}