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

//! \addtogroup Packet_Handler_CHL
//! \{

// TAS includes
#include "tas_pkt_handler_base.h"

//! \brief Derived packet handler class for handling channel packets
class CTasPktHandlerChl : public CTasPktHandlerBase
{

public:
	CTasPktHandlerChl(const CTasPktHandlerChl&) = delete; //!< \brief delete the copy constructor
	CTasPktHandlerChl operator= (const CTasPktHandlerChl&) = delete; //!< \brief delete copy-assignment operator

	//! \brief Channel packet handler object constructor.
	//! \param ei pointer to the TAS error info 
	explicit CTasPktHandlerChl(tas_error_info_st* ei);

	//! \brief Packet handler object destructor for cleanup 
	~CTasPktHandlerChl();

	//! \brief Get a request packet for subscribing to a channel.
	//! \details The length of the request in bytes is pkt_rq[0].
	//! \param chl channel number:  0 to \ref TAS_CHL_NUM_MAX - 1
	//! \param cht channel type: send, recv, bidi
	//! \param chso channel subscribe option: none, exclusive
	//! \param prio channel priority
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_subscribe(uint8_t chl, tas_cht_et cht, tas_chso_et chso, uint8_t prio);
	
	//! \brief Get a response from a subscribe request.
	//! \param pkt_rsp pointer to the response packet
	//! \param cht pointer to the channel type value: send, recv, bidi
	//! \param chso pointer to the channel subscribe option value: none, exclusive
	//! \param prio pointer to the channel priority value
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et set_pkt_rsp_subscribe(const uint32_t* pkt_rsp, tas_cht_et* cht, tas_chso_et* chso, uint8_t *prio);

	//! \brief Get a request packet for unsubscribing from a channel.
	//! \details The length of the request in bytes is pkt_rq[0].
	//! \param chl channel number of a subscribed channel
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_unsubscribe(uint8_t chl);
	
	//! \brief Get a response from a unsubscribe request.
	//! \param pkt_rsp pointer to the response packet
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et set_pkt_rsp_unsubscribe(const uint32_t* pkt_rsp);

	//! \brief Get a request packet for sending a message.
	//! \details The length of the request in bytes is pkt_rq[0].
	//! \param chl channel number of a subscribed channel
	//! \param msg pointer to the message data
	//! \param msg_length length of the message data
	//! \param init init word, should be 0 if not used
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_send_msg(uint8_t chl, const void* msg, uint16_t msg_length, uint32_t init);
	
	//! \brief Get a response for a received message.
	//! \param pkt_rsp pointer to the response packet
	//! \param chl channel number of a subscribed channel
	//! \param msg pointer to the received message data
	//! \param msg_length pointer to the length of the message data
	//! \param init pointer to the init word
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et set_pkt_rcv_msg(const uint32_t* pkt_rsp, uint8_t chl, const void** msg, uint16_t* msg_length, uint32_t* init);

private:

	void mEnforceDerivedClass() final { ; }

	uint8_t mChl = 0xFF;       //!< \brief Check response of subscribe/unsubscribe
	tas_cht_et mCht = TAS_CHT_NONE;    //!< \brief Check response of subscribe
	tas_chso_et mChso = TAS_CHSO_DEFAULT;  //!< \brief Check response of subscribe

};

//! \} // end of group Packet_Handlers_CHL