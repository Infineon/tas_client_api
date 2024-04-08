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

//! \addtogroup Client_API
//! \{
// TAS includes
#include "tas_client.h"

// Standard includes
#include <cassert>
#include <cstdio>

#ifdef _DEBUG
enum { TAS_DEFAULT_TIMEOUT_MS = -1 }; 		//!< \brief No timeout
#else
enum { TAS_DEFAULT_TIMEOUT_MS = 20000 }; 	//!< \brief 20s
#endif

//! \brief TAS error information including c-string and an error code.
struct tas_error_info_st {
	char info[TAS_INFO_STR_LEN];	//!< \brief c-string describing the error
	tas_return_et tas_err;			//!< \brief Corresponding error code
};

//! \brief Clean up error information. 
//! \param ei Pointer to an error information
//! \returns \ref TAS_ERR_NONE
inline tas_return_et tas_clear_error_info(tas_error_info_st* ei)
{
	ei->info[0] = 0;
	ei->tas_err = TAS_ERR_NONE;
	return TAS_ERR_NONE;
}

//! \brief Forces the caller to handel server connection error
//! \param ei Pointer to an error information
//! \returns \ref TAS_ERR_SERVER_CON
inline tas_return_et tas_client_handle_error_server_con(tas_error_info_st* ei)
{
	assert(false);
	snprintf(ei->info, TAS_INFO_STR_LEN, "ERROR: Server connection");
	ei->tas_err = TAS_ERR_SERVER_CON;
	return ei->tas_err;
}

//! \} // end of group Client_API