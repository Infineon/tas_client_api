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

//! \addtogroup Channel_API
//! \{

// TAS includes
#include "tas_client_impl.h"
#include "tas_client_server_con.h"
#include "tas_pkt_handler_chl.h"
#include "tas_pkt_mailbox_if.h"

//! \brief Class for sending and receiving massages through the TAS channel interface
class CTasClientChl final : public CTasClientServerCon
{

public:

	//! \brief Channel object constructor
	//! \param client_name Mandatory client name as a c-string
	explicit CTasClientChl(const char* client_name);

	//! \brief Start a connection session
	//! \details A session can only be started when a channel description is available in the TasServer.
	//! The channel description is read from a device by the first ClientChl.session_start() call.
	//! If this is not possible (e.g. device in halt after reset state), session_start() fails.
	//! The TasServer forgets the channel description, when no ClientChl is connected to a target.
	//! \param identifier Unique access HW name or IP address of device as a c-string
	//! \param session_name Unique session name as a c-string
	//! \param session_pw Session password, specify to block other clients joining this session
	//! \param chl_target channel target in case of a CHL client 
	//! \param chl_param channel parameters in case of a CHL client
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et session_start(const char* identifier, const char* session_name = "", const char* session_pw = "", 
								tas_chl_target_et chl_target = TAS_CHL_TGT_DMM, uint64_t chl_param = 0);

	//! \brief Subscribe to a channel.
	//! \details A channel can be claimed as exclusive (no other client can subscribe anymore).
	//! Claiming will fail if the channel was already subscribed by another client.
	//! For prioritized channels 0 is highest and TAS_CHL_LOWEST_PRIORITY lowest priority.
	//! Priorities can only be assigned to one channel except the lowest priority.\n
	//! prio [in]  : Requested priority (default TAS_CHL_LOWEST_PRIORITY)\n
	//! prio [out] : Assigned priority (equal or lower than requested)\n
	//! It is not possible to subscribe to more than one channel.
	//! \param chl Channel number, can be between 0 and 31
	//! \param cht Channel type, can be send, receive or bidirectional
	//! \param chso	Subscribe exclusively to a channel. 
	//! \param prio Channel's priority
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et subscribe(uint8_t chl, tas_cht_et cht, tas_chso_et chso = TAS_CHSO_EXCLUSIVE, uint8_t* prio = nullptr);

	//! \brief Unsubscribe from a channel. 
	//! \details Unsubscribing from an unsubscribed channel has no effect.
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et unsubscribe();

	//! \brief Send a message through a subscribed channel
	//! \details \ref tas_con_info_st.msg_length_c2d sets the limit for msg_length plus 4 bytes for init if init is not 0
	//! \param msg Pointer to message content
	//! \param msg_length Length of the message in Bytes
	//! \param init Optional init word, default 0
	tas_return_et send_msg(const void* msg, uint16_t msg_length, uint32_t init = 0);

	//! \brief Receive a message through a subscribed channel
	//! \details \ref tas_con_info_st.msg_length_d2c sets the limit for msg_length plus 4 bytes for init if init is not 0
	//! \param timeout_ms Timeout value in milliseconds. Blocking function with timeout. Timeout can be 0 for non-blocking.
	//! \param msg Pointer to a message buffer
	//! \param msg_length Pointer to a variable for storing the message length
	//! \param init Pointer to a variable for storing the init word
	tas_return_et rcv_msg(uint32_t timeout_ms, const void** msg, uint16_t* msg_length, uint32_t* init);

	//! \brief Performs a check if device reset has occurred.
	//! \returns \c true if reset occurred, otherwise \c false
	bool device_reset_occurred() override
	{
		if (!mSessionStarted) {
			return false;
		}

		uint32_t resetCount = mTphChl.get_device_reset_count();

		if (mDeviceResetCount > resetCount) {
			assert(mDeviceResetCount == ~0U);  // device_connect() was not called
			assert(false);  // Wrong usage
			return false;
		}

		bool resetOccurred = (resetCount > mDeviceResetCount);

		mDeviceResetCount = resetCount;

		return resetOccurred;
	}

	//! \brief Connect to a target device. Can only be called when no channel is subscribed.
	//! \param dco Connection option, _RESET, _RESET_AND_HALT, _UNLOCK, default hot attach
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et device_connect(tas_clnt_dco_et dco) override;

	// The following methods are only needed for special use cases and debugging

	//! \brief Send a ping to a target and obtain connection info
	//! \details Check the connection which was established with CTasClientServerCon::session_start() before.\n
	//! The channel description will be read from the device only once if the channel information is present.\n
	//! Only needed for special use cases and debugging.
	//! \param con_info Pointer to a variable to which connection info should be stored
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et target_ping(tas_con_info_st* con_info);

	//! \brief Channel object constructor. Only used for special test setups.
	//! \param mb_if Mailbox interface
	explicit CTasClientChl(CTasPktMailboxIf* mb_if);

private:

	tas_error_info_st mEi; //!< \brief Contains current error information. 

	CTasPktMailboxIf* mMbIfChl; //!< \brief Packet mailbox interface.

	CTasPktHandlerChl mTphChl; //!< \brief Packet handler object. Used for constructing and parsing packets.

	tas_cht_et mChlCht = TAS_CHT_NONE;		//!< \brief Channel type
	uint8_t    mChlNum = TAS_CHL_NUM_MAX;	//!< \brief Channel number

};

//! \} // end of group Channel_API