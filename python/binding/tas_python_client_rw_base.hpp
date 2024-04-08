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
#include <tas_client_rw_base.h>

typedef struct {
	uint64_t addr;
	uint32_t num_bytes;
	tas_rw_trans_type_et type;
	pybind11::bytes data;
} TasRwTransaction;

typedef enum : uint8_t {
	TAS_AM_0   = 0,    // Default for device access
	TAS_AM_1   = 1,    // Auxiliary address map for device access. E.g. direct EEC access of AURIX EDs
	TAS_AM_12  = 12,   // All address maps equal or higher than 12 are for special usage
	TAS_AM_14  = 14,   // Exclusively used in the TasProxyServer inside of the device
	TAS_AM_15  = 15,   // Exclusively used inside of the TasServer
	TAS_AM_132 = 132,  // Converted to TAS_AM15 during TasClient packet request generation
}TasAddrMap;

class TasClientRwBase{
	public:
		pybind11::tuple read8(uint64_t addr, uint8_t addr_map = TAS_AM0);

		tas_return_et write8(uint64_t addr, uint8_t value, uint8_t addr_map = TAS_AM0);

		pybind11::tuple read16(uint64_t addr, uint8_t addr_map = TAS_AM0);

		tas_return_et write16(uint64_t addr, uint16_t value, uint8_t addr_map = TAS_AM0);

		pybind11::tuple read32(uint64_t addr, uint8_t addr_map = TAS_AM0);
		
		tas_return_et write32(uint64_t addr, uint32_t value, uint8_t addr_map = TAS_AM0);
		
		pybind11::tuple read64(uint64_t addr, uint8_t addr_map = TAS_AM0);
		
		tas_return_et write64(uint64_t addr, uint64_t value, uint8_t addr_map = TAS_AM0);
		
		pybind11::tuple read(uint64_t addr, uint32_t num_bytes, uint8_t addr_map = TAS_AM0);
		
		pybind11::tuple write(uint64_t addr, pybind11::bytes data, uint8_t addr_map = TAS_AM0);

		pybind11::tuple execute_trans(pybind11::list &trans);

		tas_return_et fill32(uint64_t addr, uint32_t value, uint32_t num_bytes, uint8_t addr_map = TAS_AM0);

		tas_return_et fill64(uint64_t addr, uint64_t value, uint32_t num_bytes, uint8_t addr_map = TAS_AM0);

	    TasClientRwBase(CTasClientRwBase& c);

		TasClientRwBase(uint32_t max_rsp_size = CTasPktHandlerRw::PKT_BUF_SIZE_DEFAULT);

	private:
		CTasClientRwBase* clientRwBase;

};
