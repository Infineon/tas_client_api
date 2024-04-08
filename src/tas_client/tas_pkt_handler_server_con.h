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

//! \addtogroup Packet_Handlers
//! \{

// TAS includes
#include "tas_pkt_handler_base.h"
#include "tas_am15_am14.h"

// Standard includes
#include <array>

//! \brief Derived packet handler class for handling client-server packets
class CTasPktHandlerServerCon : public CTasPktHandlerBase
{

public:
	CTasPktHandlerServerCon(const CTasPktHandlerServerCon&) = delete; //!< \brief delete the copy constructor
	CTasPktHandlerServerCon operator= (const CTasPktHandlerServerCon&) = delete; //!< \brief delete copy-assignment operator

	//! \brief Client-server packet handler object constructor.
	//! \param ei pointer to the TAS error info 
	explicit CTasPktHandlerServerCon(tas_error_info_st* ei);

	//! \brief Packet handler object destructor for cleanup.
	~CTasPktHandlerServerCon();

	//! \brief Get a request packet for establishing a connection with a server.
	//! \param client_name pointer to a c-string containing the client's name
	//! \param user_name pointer to a c-string containing the user name who started this client
	//! \param client_pid process id of the client application
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_server_connect(const char* client_name, const char* user_name, uint32_t client_pid);

	//! \brief Get a response from a server connect request.
	//! \param pkt_rsp pointer to the response packet
	//! \param server_info pointer to the server information
	//! \param challenge pointer to the server's challenge value
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et   set_pkt_rsp_server_connect(const uint32_t* pkt_rsp, const tas_server_info_st** server_info, uint64_t* challenge);

	//! \brief Not yet implemented.
	//! \param key pointer to a key storage
	//! \param key_length length of the key in bytes
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_server_unlock(const void* key, uint16_t key_length);

	//! \brief Not yet implemented.
	//! \param pkt_rsp pointer to the response packet
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et   set_pkt_rsp_server_unlock(const uint32_t* pkt_rsp);

	//! \brief Get a request for retrieving a list of targets connected to a TAS server.
	//! \param start_index indicates the first position in the list
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_get_targets(uint8_t start_index);

	//! \brief Get a response from a get targets request.
	//! \param pkt_rsp pointer to the response packet
	//! \param num_target pointer to the number targets in the list
	//! \param num_now pointer to the current index
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et   set_pkt_rsp_get_targets(const uint32_t* pkt_rsp, uint8_t* num_target, uint8_t* num_now);

	//! \brief Get information about the targets.
	//! \param target_info pointer to a list of target informations
	//! \param num_target pointer to the number of targets
	void  get_target_info(const tas_target_info_st** target_info, uint32_t* num_target) const;

	//! \brief Get a request for retrieving a list of clients connected to a target.
	//! \param identifier pointer to a c-string containing the target's identifier
	//! \param start_index indicates the first position in the list
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_get_target_clients(const char* identifier, uint8_t start_index);

	//! \brief Get a response from a get target clients request.
	//! \param pkt_rsp pointer to the response packet
	//! \param num_client pointer to the number of clients 
	//! \param num_now pointer to the current index of the client list
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et   set_pkt_rsp_get_target_clients(const uint32_t* pkt_rsp, uint8_t* num_client, uint8_t* num_now);

	//! \brief Get information about the clients connected to a target.
	//! \param session_name pointer to a c-string containing the session name
	//! \param session_start_time_us pointer to the value when the session was started
	//! \param target_client_info pointer to a list of client informations
	//! \param num_client pointer to the number of clients in the list
	void  get_target_clients_info(const char** session_name, uint64_t* session_start_time_us,
								  const tas_target_client_info_st** target_client_info, uint32_t* num_client);

	//! \brief Get a request for a session start.
	//! \param client_type specifies the type of a client: RW, CHL, TRC
	//! \param identifier Unique access HW name or IP address of device as a c-string
	//! \param session_id Unique session name as a c-string
	//! \param session_pw Session password, specify to block other clients joining this session
	//! \param chl_target channel target in case of a CHL client 
	//! \param chl_param channel parameters in case of a CHL client
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_session_start(tas_client_type_et client_type, const char* identifier, const char* session_id, 
											 const char* session_pw, tas_chl_target_et chl_target, uint64_t chl_param);

	//! \brief Get a response from a session start request.
	//! \param client_type specifies the type of a client: RW, CHL, TRC
	//! \param pkt_rsp pointer to the response packet										 
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et   set_pkt_rsp_session_start(tas_client_type_et client_type, const uint32_t* pkt_rsp);

	//! \brief Not yet implemented.
	//! \param ulcro select device specific challenge response sequence options
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_device_unlock_get_challenge(tas_dev_unlock_cr_option_et ulcro);

	//! \brief Not yet implemented.
	//! \param pkt_rsp pointer to the response packet
	//! \param challenge pointer to a challenge
	//! \param challenge_length pointer to a challenge length
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et   set_pkt_rsp_device_unlock_get_challenge(const uint32_t* pkt_rsp, const void** challenge, uint16_t* challenge_length);

	//! \brief Get a request for device unlocking.
	//! \param ulo specify when the unlocking occurs \ref tas_dev_unlock_option_et
	//! \param key pointer to a key storage
	//! \param key_length length of the key in bytes, maximum length is 512 bytes (TAS_UNLOCK_MAX_KEY_LEN). If \c 0, the key in the TasServer is cleared
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_device_unlock_set_key(tas_dev_unlock_option_et ulo, const void* key, uint16_t key_length);

	//! \brief Get a response from a device unlocking request.
	//! \param pkt_rsp pointer to the response packet
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et   set_pkt_rsp_device_unlock_set_key(const uint32_t* pkt_rsp);

	//! \brief Get a request for connecting to a device.
	//! \param option connect option, connect to a device with hot attach, reset, or reset and halt
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_device_connect(tas_clnt_dco_et option);

	//! \brief Get a response from a device connect request.
	//! \param pkt_rsp pointer to the response packet
	//! \param dev_con_feat_used pointer to a device connection feature used
	//! \param device_type pointer to the JTAG ID of a device
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et   set_pkt_rsp_device_connect(const uint32_t* pkt_rsp, uint16_t* dev_con_feat_used, uint32_t* device_type);

	//! \brief Get a request for retrieving the device reset count.
	//! \returns pointer to the request packet
	const uint32_t* get_pkt_rq_device_reset_count();

	//! \brief Get a response for a device reset count count.
	//! \param pkt_rsp pointer to the response packet
	//! \param reset_count pointer to the reset count value.
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et   set_pkt_rsp_device_reset_count(const uint32_t* pkt_rsp, tas_reset_count_st* reset_count);

private:

	//! \brief Default limits.
	enum {
		MAX_PKT_RQ_SIZE  = 1024,	//!< \brief Maximum size of a request packet
		MAX_PKT_RSP_SIZE = 1024,	//!< \brief Maximum size of a response packet
		NUM_TARGET_MAX = 64,		//!< \brief Maximum length of the list containing targets' information
		NUM_TARGET_CLIENT_MAX = 32,	//!< \brief Maximum length of the list containing clients' information connected to a target
	};

	void mEnforceDerivedClass() { ; }

	//! \brief Set the server conenction error in case of a an issue with a server connection setup.
	//! \returns \ref TAS_ERR_SERVER_CON
	tas_return_et mSetPktRspErrServerConnectionSetup();

	//! \brief Set the server lock error in case a server is locked.
	//! \returns \ref TAS_ERR_SERVER_LOCKED
	tas_return_et mSetPktRspErrServerLocked();

	//! \brief Set the device access error in case access to a device failed.
	//! \returns \ref TAS_ERR_DEVICE_ACCESS
	tas_return_et mSetPktRspErrDeviceAccess();

	//! \brief Set the device locked error in case the device is locked and was not unlocked prior the access.
	//! \returns \ref TAS_ERR_DEVICE_LOCKED
	tas_return_et mSetPktRspErrDeviceLocked();

	tas_server_info_st mServerInfo = {};				//!< \brief Server's information buffer
	uint64_t mServerChallenge = 0;						//!< \brief Server's challenge value 

	std::array<tas_target_info_st, NUM_TARGET_MAX> mTargetInfo;	//!< \brief List of target informations
	uint32_t mNumTarget = 0;									//!< \brief number of targets in the list <= \ref NUM_TARGET_MAX

	std::array<char, TAS_NAME_LEN16> mSessionName;	//!< \brief Buffer for the session name c-string
	uint64_t mSessionStartTimeUs;					//!< \brief Session start time in microseconds

	std::array<tas_target_client_info_st, NUM_TARGET_CLIENT_MAX> mTargetClientInfo;	//!< \brief List of client informations for clients which are connected to a target
	uint32_t mNumTargetClient = 0;													//!< \brief number of clients in the list <= \ref NUM_TARGET_CLIENT_MAX

	uint8_t mStartIndex;	//!< \brief Local copy of the start index for any of the lists
};

//! \} // end of group Packet_Handlers