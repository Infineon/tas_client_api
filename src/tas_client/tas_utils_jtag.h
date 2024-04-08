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

//! \addtogroup TAS_utils
//! \{

// TAS includes
#include "tas_client_rw.h"

//! \brief Options for JTAG scan
enum tasutil_jtag_scan_et : uint32_t {
	TJS_NONE = 0,			//!< \brief no special option
	TJS_TAP_RESET = 0x01,  	//!< \brief Initial TAP controller reset with state machine sequence
};  

//! \brief JTAG scan configuration
struct tasutil_jtag_scan_st {
	tasutil_jtag_scan_et tjs;	//!< \brief scan option
	tas_jtag_set_ir_st   ir;    //!< \brief Instruction register. If ir.width is 0 -> no IR update
	uint32_t        num_bits;   //!< \brief Number of data scan bits
	const uint64_t* data_in;    //!< \brief input data, nullptr means zeros are shifted in
	uint64_t*       data_out;   //!< \brief output data, nullptr means output is ignored
};

//! \brief Execute a JTAG scan operation
//! \param tcrw pointer to RW client object, which will execute the operation
//! \param scan pointer to a list of scan configurations
//! \param num_scan number of scan configurations
//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
tas_return_et tasutil_jtag_scan(CTasClientRw* tcrw, const tasutil_jtag_scan_st* scan, uint8_t num_scan);

//! \} // end of group TAS_utils