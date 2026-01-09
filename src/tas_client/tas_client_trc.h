/*
 *  Copyright (c) 2026 Infineon Technologies AG.
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

//! \addtogroup Trace_API
//! \{

// TAS includes
#include "tas_client_chl.h"
#include "tas_client_server_con.h"

//! \brief Class for receiving continuous trace data.
class CTasClientTrc final
{

public:

	//! \brief object constructor
	//! \param client_name Mandatory client name as a c-string
	explicit CTasClientTrc(const char* client_name);

	//! \brief Get current error information string
	//! \returns pointer to a c-string containing current error information.
	const char* get_error_info() const { return mClientChl.get_error_info(); }

	//! \brief Establishes a connection to a TAS server.
	//! \details In case of \ref TAS_ERR_SERVER_LOCKED use \ref server_unlock() and optional \ref get_server_challenge() before.
	//! \param ip_addr Hostname of a TAS server, can be an IP address or a domain based hostname
	//! \param port_num Server's port number, no need to specify if default is used: \ref TAS_PORT_NUM_SERVER_DEFAULT
	tas_return_et server_connect(const char* ip_addr, uint16_t port_num = TAS_PORT_NUM_SERVER_DEFAULT)
	{
		return mClientChl.server_connect(ip_addr, port_num);
	}

	//! \brief Get server's information.
	//! \returns pointer to the server information from the last \ref server_connect() call, \c nullptr if no server connected
	const tas_server_info_st* get_server_info() const { return mClientChl.get_server_info(); }

	//! \brief Get a challenge from a server.
	//! \returns the server challenge from the last \ref server_connect() call, \c 0 if no challenge
	uint64_t get_server_challenge() const { return mClientChl.get_server_challenge(); }

	//! \brief Unlock a server.
	//! \param key pointer to a key storage
	//! \param key_length length of the key in bytes
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et server_unlock(const void* key, uint16_t key_length) { return mClientChl.server_unlock(key, key_length); }

	//! \brief Start a connection session
	//! \param identifier Unique access HW name or IP address of device as a c-string
	//! \param session_name Unique session name as a c-string
	//! \param session_pw Session password, specify to block other clients joining this session
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et session_start(const char* identifier, const char* session_name = "", const char* session_pw = "")
	{
		return mClientChl.session_start(identifier, session_name, session_pw, TAS_CHL_TGT_TRC);
	}

	//! \brief Subscribe trace
	//! \param chso	Subscribe exclusively, default \ref TAS_CHSO_DEFAULT is not exclusive
	//! \param prio Priority for trace data transfer (0 highest, 31 lowest). Considered, if there is an arbitration with channel traffic.
	//! \param trct Trace type, default \ref TAS_TRC_T_MTSC_IFTG
	//! \param stream Optional stream identifier, it allows to differentiate between independent trace streams from the same device
	//! \returns \ref TAS_ERR_NONE on sucess, otherwise any other relevant TAS error code
	tas_return_et subscribe(tas_chso_et chso = TAS_CHSO_DEFAULT, uint8_t prio = 31, tas_trc_type_et trace_type = TAS_TRC_T_ANY, uint8_t stream = 0);

	//! \brief Get received trace data. It is a blocking function with timeout.
	//! \param timeout_ms Configure timeout in milliseconds
	//! \param trace_data Pointer to a trace data buffer (64 bit address aligned)
	//! \param length Pointer to length of the received data
	//! \param trc_state Pointer to trace stream state
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et get_trace(uint32_t timeout_ms, const uint32_t** trace_data, uint32_t *length, tas_trc_state_st* trc_state);

	//! \brief Performs a check if device reset has occurred.
	//! \returns \c true if reset occurred, otherwise \c false
	bool device_reset_occurred() { return mClientChl.device_reset_occurred(); }


	// The following methods are only needed for special use cases and debugging

	//! \brief Send a ping to a target and obtain connection info
	//! \details Check the connection which was established with CTasClientServerCon::session_start() before.\n
	//! Only needed for special use cases and debugging.
	//! \param con_info Pointer to a variable to which connection info should be stored
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et target_ping(tas_con_info_st* con_info) { return mClientChl.target_ping(con_info); }

private:

	CTasClientChl mClientChl;

	tas_trc_subscribe_st mTrcSubscribe = {};

	tas_trc_state_st mTrcStateLast = {};  // For debugging
};

//! \} // end of group Trace_API