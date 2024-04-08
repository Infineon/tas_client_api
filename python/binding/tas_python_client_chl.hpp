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
#include <pybind11/pytypes.h>
#include <tas_client_chl.h>
#include "tas_python_client_server_con.hpp"


class TasClientChl : public TasClientServerCon
{
	public:
		tas_return_et session_start(const char* identifier, const char* session_id = "", const char* session_pw = "", tas_chl_target_et chl_target = TAS_CHL_TGT_DMM, uint64_t chl_param = 0);

		tas_return_et subscribe(uint8_t chl, tas_cht_et cht, tas_chso_et chso = TAS_CHSO_EXCLUSIVE, uint8_t* prio = nullptr);

		tas_return_et unsubscribe();

		tas_return_et send_msg(pybind11::bytes message, uint32_t init = 0);

		tas_return_et send_string(std::string message, uint32_t init = 0);

		pybind11::tuple rcv_msg(uint32_t timeout_ms);

		pybind11::tuple rcv_string(uint32_t timeout_ms);

		TasClientChl(const char* client_name) : client(client_name), TasClientServerCon(client){};

	private:
		CTasClientChl client;

};