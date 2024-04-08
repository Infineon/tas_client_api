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

#pragma once

#include <pybind11/pybind11.h>
#include <tas_client_rw.h>
#include "tas_python_client_rw_base.hpp"
#include "tas_python_client_server_con.hpp"

class TasClientRw : public TasClientRwBase, public TasClientServerCon
{
    public:
        tas_return_et session_start(const char* identifier, const char* session_id = "", const char* session_pw = ""){
		    return client.session_start(identifier, session_id, session_pw);
	    }

        TasClientRw(const char* client_name) : client(client_name), TasClientServerCon(client), TasClientRwBase(client){
        };

    private:
        CTasClientRw client;

};