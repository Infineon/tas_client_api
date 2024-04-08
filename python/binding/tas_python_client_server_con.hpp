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
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <tas_client_server_con.h>
#include <tas_device_family.h>

namespace py = pybind11;

class TasClientServerCon{
	public:

		TasClientServerCon(CTasClientServerCon& c);

		const char* get_error_info();

		tas_return_et server_connect(const char* ip_addr, uint16_t port_num = TAS_PORT_NUM_SERVER_DEFAULT);

		const char* get_server_ip_addr();

		uint16_t get_server_port_num();
		
		const tas_server_info_st get_server_info();

		uint64_t get_server_challenge();

		tas_return_et server_unlock(py::bytes key);

		py::tuple get_targets();

		py::tuple get_target_clients(const char* identifier);

		const tas_con_info_st get_con_info();

		tas_device_family_t get_device_family(uint32_t device_type);

		py::tuple device_unlock_get_challenge(tas_dev_unlock_cr_option_et ulcro);

		tas_return_et device_unlock_set_key(tas_dev_unlock_option_et ulo, py::bytes key);

		tas_return_et device_connect(tas_clnt_dco_et dco);

		bool device_reset_occurred();

	private:
		CTasClientServerCon* clientServerCon;
};