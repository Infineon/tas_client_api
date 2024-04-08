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

//! \addtogroup Trace_API
//! \{

// TAS includes
#include "tas_client_impl.h"
#include "tas_client_server_con.h"
#include "tas_pkt_handler_trc.h"
#include "tas_pkt_mailbox_if.h"

//! \brief Class for receiving continuous trace data.
class CTasClientTrc final
{

public:

	//! \brief 
	CTasClientTrc();

	//! \brief Subscribe to a trace channel.
	//! \param trct Trace type
	//! \param stream Optional stream identifier, it allows to differentiate between independent trace streams from the same device
	//! \param chso	Subscribe exclusively to a channel, default \ref TAS_CHSO_DEFAULT
	//! \returns \ref TAS_ERR_NONE on sucess, otherwise any other relevant TAS error code
	tas_return_et subscribe(tas_trc_type_et* trct, uint8_t stream = 0, tas_chso_et chso = TAS_CHSO_DEFAULT);

	//! \brief Unsubscribe from a trace channel or selected trace stream
	//! \param stream Optional stream identifier
	//! \returns \ref TAS_ERR_NONE on sucess, otherwise any other relevant TAS error code
	tas_return_et unsubscribe(uint8_t stream = 0);

	//! \brief Receive trace data. It is a blocking function with timeout.
	//! \param timeout_ms Configure timeout in milliseconds
	//! \param trace_data Pointer to a trace data buffer
	//! \param length Pointer to length of the received data
	//! \param trcs Pointer to trace stream status
	//! \param stream Pointer to a stream identifier
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et rcv_trace(uint32_t timeout_ms, const void** trace_data, uint32_t *length, tas_trcs_et* trcs, uint8_t* stream = nullptr);


	// The following methods are only needed for special use cases and debugging

	//! \brief Send a ping to a target and obtain connection info
	//! \details Check the connection which was established with CTasClientServerCon::session_start() before.\n
	//! Only needed for special use cases and debugging.
	//! \param con_info Pointer to a variable to which connection info should be stored
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et target_ping(tas_con_info_st* con_info);

	//! \brief Trace client object constructor
	//! \details !!Only needed for testing purposes. Not for regular TAS clients!!
	//! \param mb_if Mailbox interface
	explicit CTasClientTrc(CTasPktMailboxIf* mb_if);

private:

	tas_error_info_st mEi;			//!< \brief Contains current error information. 

	CTasPktMailboxIf* mMbIfTrc;		//!< \brief Packet mailbox interface.

	CTasPktHandlerTrc* mTphTrc;		//!< \brief Packet handler object. Used for constructing and parsing packets.

};

//! \} // end of group Trace_API