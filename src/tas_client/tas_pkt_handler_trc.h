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

//! \addtogroup Packet_Handler_TRC
//! \{

// TAS includes
#include "tas_pkt_handler_base.h"

//! \brief Derived packet handler class for handling trace packets
//! \details The top level API is not completely fixed. Therefore, the definitions might still change.
class CTasPktHandlerTrc : public CTasPktHandlerBase
{

public:

	//! \brief Trace packet handler object constructor.
	//! \param ei pointer to the TAS error info 
	explicit CTasPktHandlerTrc(tas_error_info_st* ei);

	//! \brief Get a request to subscribe to a trace channel.
	//! \param stream trace stream identifier
	//! \param chso channel subscribe option: none, exclusive
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_subscribe(uint8_t stream, tas_chso_et chso);

	//! \brief Get a response from a subscribe request.
	//! \param pkt_rsp pointer to the response packet
	//! \param trct pointer to the trace type
	//! \param chso pointer to channel subscribe option
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et set_pkt_rsp_subscribe(const uint32_t* pkt_rsp, tas_trc_type_et *trct, tas_chso_et* chso);

	//! \brief Get a request to unsubscribe from a trace channel.
	//! \param stream trace stream identifier
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_unsubscribe(uint8_t stream);

	//! \brief Get a response from the unsubscribe request.
	//! \param pkt_rsp pointer to the response packet
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et set_pkt_rsp_unsubscribe(const uint32_t* pkt_rsp);

	//! \brief Get received trace data from a response packet
	//! \param pkt_rsp pointer to the response packet
	//! \param trace_data pointer to the received trace data
	//! \param length pointer to the length of trace data
	//! \param trcs pointer to the trace stream state
	//! \param stream pointer to the trace stream identifier
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et set_pkt_rcv_trace(const uint32_t* pkt_rsp, const void** trace_data, uint32_t* length, tas_trcs_et* trcs, uint8_t* stream);


private:

	void mEnforceDerivedClass() { ; }

	//! \brief Set function usage error in case the trace channel or a stream was exclusively claimed by another client.
	//! \returns \ref TAS_ERR_FN_USAGE
	tas_return_et mSetPktRspErrTraceClaimed(uint8_t stream);

	//! \brief Set function not supported in case continuous trace not supported by this device, access HW or server.
	//! \returns \ref TAS_ERR_FN_NOT_SUPPORTED
	tas_return_et mSetPktRspErrTraceNotSupported();

	uint8_t mStream;    //!< \brief Check response of subscribe/unsubscribe
	tas_chso_et mChso;  //!< \brief Check response of subscribe

};

//! \} // end of group Packet_Handler_TRC