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

//! \defgroup Client_API TAS Client Application Programming Interface

//! \defgroup Read_Write_API Read/Write (RW)
//! \ingroup  Client_API
//! \defgroup Channel_API Channel (CHL)
//! \ingroup  Client_API
//! \defgroup Trace_API Trace (TRC)
//! \ingroup Client_API

//! \addtogroup Client_API
//! \{
// TAS includes
#include "tas_pkt.h"

// Standard includes
#include <cstdint>
#include <cstddef>

constexpr size_t TAS_INFO_STR_LEN = 256;     //!< \brief Maximum allowed string for TAS information

//! \brief TAS error codes
enum tas_return_et : uint16_t {
	TAS_ERR_NONE             = 0,       //!< \brief No error

	TAS_ERR_GENERAL          = 0x0F00,	//!< \brief General error. E.g. unexpected behavior of the TasServer

	TAS_ERR_FN_NOT_SUPPORTED = 0x0100,  //!< \brief Function is e.g. not implemented
	TAS_ERR_FN_USAGE         = 0x0110,  //!< \brief TAS API not correctly used
	TAS_ERR_FN_PARAM         = 0x0120,  //!< \brief Caller passed an invalid parameter

	TAS_ERR_SERVER_CON       = 0x0200,  //!< \brief Server connection error
	TAS_ERR_SERVER_LOCKED    = 0x0210,  //!< \brief Server is locked

	TAS_ERR_DEVICE_ACCESS    = 0x0400,  //!< \brief Cannot access device (power-down, reset active, etc.)
	TAS_ERR_DEVICE_LOCKED    = 0x0410,  //!< \brief Device is locked

	TAS_ERR_RW_READ          = 0x0600,  //!< \brief Read access failed
	TAS_ERR_RW_WRITE         = 0x0610,  //!< \brief Write access failed

	TAS_ERR_CHL_SETUP        = 0x0800,  //!< \brief Channel setup failed
	TAS_ERR_CHL_SEND         = 0x0810,  //!< \brief Sending of the message has failed (e.g. buffer full). Can try again.
	TAS_ERR_CHL_RCV          = 0x0820,  //!< \brief No message was received. Can try again.

	TAS_ERR_TRC_RCV          = 0x0A00,  //!< \brief No trace data was received. Can try again.
};

//! \brief Connection options for device_connect() method.
enum tas_clnt_dco_et : uint16_t {  
	TAS_CLNT_DCO_HOT_ATTACH     = 0,								//!< \brief Default is hot attach
	TAS_CLNT_DCO_RESET          = TAS_DEV_CON_FEAT_RESET,			//!< \brief Triggers a reset
	TAS_CLNT_DCO_RESET_AND_HALT = TAS_DEV_CON_FEAT_RESET_AND_HALT,	//!< \brief Triggers a reset and halt
	TAS_CLNT_DCO_UNKNOWN        = TAS_DEV_CON_FEAT_UNKNOWN,        	//!< \brief Connect to "unknown" device -> no device interaction
	TAS_CLNT_DCO_UNKNOWN_RESET  = TAS_DEV_CON_FEAT_UNKNOWN_RESET,  	//!< \brief Reset "unknown" device -> no device interaction
};

//! \brief Transaction types.
enum tas_rw_trans_type_et : uint8_t {
	TAS_RW_TT_RD   = 1,		//!< \brief Indicates a read transaction
	TAS_RW_TT_WR   = 2,		//!< \brief Indicates a write transaction
	TAS_RW_TT_FILL = 6,		//!< \brief Indicates a fill transaction
};

//! \brief Read/write transaction description
struct tas_rw_trans_st {
	uint64_t addr;				//!< \brief 64-bit address on which the action is performed 
	uint32_t num_bytes;			//!< \brief Number of bytes to be read/written
	uint16_t acc_mode;			//!< \brief Type of access mode to be used
	uint8_t  addr_map;			//!< \brief Address map
	tas_rw_trans_type_et type;	//!< \brief Transaction type as defined by \ref tas_rw_trans_type_et
	union {
		const void* wdata;		//!< \brief Data to be written
		void* rdata;			//!< \brief Data to be read
	};
};

//! \brief Read/write transaction response
struct tas_rw_trans_rsp_st {
	uint16_t num_bytes_ok;		//!< \brief Number of bytes successfully read/written
	tas_pl_err_et8 pl_err;		//!< \brief PL Error code \ref tas_pl_err_et8
};
//! \} // end of group Client_API