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
#include "tas_client_impl.h"
#include "tas_pkt_mailbox_socket.h"
#include "tas_pkt_handler_server_con.h"

// Standard includes
#include <array>

//! \brief Class for TAS Client - TAS Server interaction.
//! Common for Read/Write, Channel, and Trace clients.
class CTasClientServerCon  
{

public:
	CTasClientServerCon(const CTasClientServerCon&) = delete; //!< \brief delete the copy constructor
	CTasClientServerCon operator= (const CTasClientServerCon&) = delete; //!< \brief delete copy-assignment operator

	//! \brief Object destructor. Used for cleanup.
	~CTasClientServerCon();

	//! \brief Get current error information string
	//! \returns pointer to a c-string containing current error information.
	const char* get_error_info() const { return mEip->info; }
	
	//! \brief Establishes a connection to a TAS server.
	//! \details In case of \ref TAS_ERR_SERVER_LOCKED use \ref server_unlock() and optional \ref get_server_challenge() before.
	//! \param ip_addr Hostname of a TAS server, can be an IP address or a domain based hostname
	//! \param port_num Server's port number, no need to specify if default is used: \ref TAS_PORT_NUM_SERVER_DEFAULT
	tas_return_et server_connect(const char* ip_addr, uint16_t port_num = TAS_PORT_NUM_SERVER_DEFAULT);
	
	//! \brief Get the server's IP address.
	//! \returns pointer to a c-string containing IP address in dot format
	const char* get_server_ip_addr() const { return mServerIpAddr.data(); }

	//! \brief Get the server's port number.
	//! \returns server's port number
	uint16_t get_server_port_num() const { return mServerPortNum; }

	//! \brief Get server's information.
	//! \returns pointer to the server information from the last \ref server_connect() call, \c nullptr if no server connected
	const tas_server_info_st* get_server_info() const { return mServerInfo; }

	//! \brief Get a challenge from a server.
	//! \returns the server challenge from the last \ref server_connect() call, \c 0 if no challenge
	uint64_t get_server_challenge() const { return mServerChallenge; }

	//! \brief Unlock a server.
	//! \param key pointer to a key storage
	//! \param key_length length of the key in bytes
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et server_unlock(const void* key, uint16_t key_length);

	// XXXXX Server setup
	// Do we need a challenge type description? Or is it always a string? -> adapt get_server_challenge()
	// How to do the encryption - end to end?

	//! \brief Get a list of targets connected to a server.
	//! \details The target_info array lists all targets that are connected to the server
	//! target_info.identifier contains the unique access HW name or the IP address of the device.
	//! Several clients (target_info.num_client) can be connected to one target.
	//! \param target_info pointer to a list of targets
	//! \param num_target pointer to a number of targets in the list
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et get_targets(const tas_target_info_st** target_info, uint32_t* num_target);
	
	//! \brief Get a list of clients connected to a target.
	//! \details The target_client_info array lists all clients that are connected to the target selected by identifier.
	//! Allows to get the session_name of a running session. Please note that a tool may not automatically
	//! connect to a running session with a retrieved session name. Otherwise there is a high risk for 
	//! unintended conflicts between different users.
	//!
	//! tas_return_et session_start(const char* identifier, const char* session_name = "", const char* session_pw = "")
	//! Start a session by establishing a target connection.
	//! A target can be a device or just an access HW without a device connected
	//! session_name and session_pw protect from uncontrolled accesses by other clients.
	//! If no client is connected to a target, the first client sets session_name and session_pw. 
	//!
	//! Once a session was successfully started it is not allowed to call session_start() again.
	//! A session is ended by the destructor. 
	//! \param identifier Pointer to a c-string containing target's identifier
	//! \param session_name Pointer to a session name of a running session
	//! \param session_start_time_us Pointer to a start time of the running session
	//! \param target_client_info Pointer to a list of clients connected to the target identified by the identifier
	//! \param num_client Pointer to a number of clients in the list
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et get_target_clients(const char* identifier, const char** session_name, 
									 uint64_t* session_start_time_us,
		                             const tas_target_client_info_st** target_client_info, 
									 uint32_t* num_client);
	
	//! \brief Get connection information of an established connection.
	//! \returns pointer to the connection information
	const tas_con_info_st* get_con_info() const { return mTphsc.get_con_info(); }

	//! \brief Get a challenge for device unlocking.
	//! \param ulcro select device specific challenge response sequence options
	//! \param challenge pointer to a challenge
	//! \param challenge_length pointer to a challenge length
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et device_unlock_get_challenge(tas_dev_unlock_cr_option_et ulcro, const void** challenge, uint16_t* challenge_length);

	//! \brief Set key for device unlocking.
	//! \param ulo specify when the unlocking occurs \ref tas_dev_unlock_option_et
	//! \param key pointer to a key storage
	//! \param key_length length of the key in bytes, maximum length is 512 bytes (TAS_UNLOCK_MAX_KEY_LEN). If \c 0, the key in the TasServer is cleared
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et device_unlock_set_key(tas_dev_unlock_option_et ulo, const void* key, uint16_t key_length);

	//! \brief Connect to a device.
	//! It will use the provided key for unlocking if needed.
	//! \param dco connect option, connect to a device with hot attach, reset, or reset and halt
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	virtual tas_return_et device_connect(tas_clnt_dco_et dco) = 0;
	
	//! \brief Check if device reset has occurred
	//! \details Prerequisite is that there was a device interaction in between.
	//! For ClientRw this means an RW access and for ClientChl that a message was received.
	//! \ref device_connect() sets the reference point.
	//! The reset information is collected from all device accesses by all clients before.
	//! Not all hardware setups will support this feature.
	//! \returns \c true if at least one device reset occurred since the last call
	virtual bool device_reset_occurred() = 0;

protected:

	//! \brief Client-Server connection object constructor. Instantiated by respective client object constructor.
	//! \details User name is retrieved form the OS.
	//! \param client_name pointer to a c-string containing the client's name
	//! \param ei pointer to an error info
	//! \param mb_if respective mailbox interface
	CTasClientServerCon(const char* client_name, tas_error_info_st* ei, CTasPktMailboxIf* mb_if = nullptr);
	
	//! \brief Start a session.
	//! \param client_type specifies the type of a client, RW, CHL, or TRC
	//! \param identifier Unique access HW name or IP address of device as a c-string
	//! \param session_name pointer to a c-string containing the session's name
	//! \param session_pw pointer to a c-string containing the session's password
	//! \param chl_target channel target in case of a CHL client 
	//! \param chl_param channel parameters in case of a CHL client
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et mSessionStart(tas_client_type_et client_type, 
		                        const char* identifier, const char* session_name, const char* session_pw,
								tas_chl_target_et chl_target = TAS_CHL_TGT_UNKNOWN, uint64_t chl_param = 0);

	//! \brief Connect to a device.
	//! \param dco connect option, connect to a device with hot attach, reset, or reset and halt
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et mDeviceConnect(tas_clnt_dco_et dco);

	CTasPktMailboxIf* mMbIf = nullptr;	//!< \brief Mailbox interface.

	CTasPktMailboxSocket* mMbSocket = nullptr;	//!< \brief Mailbox interface with a socket connection.

	bool mSessionStarted = false;	//!< \brief boolean flag which indicates if the session was already started

	bool mRcvChlActive = false; //!< \brief For TAS_CHT_RCV and TAS_CHT_BIDI some client functions are not supported

	uint32_t mDeviceResetCount = ~0;	//!< \brief device reset counter

private:

	//! \brief Handle a failed attempt to connect to a server. 
	//! \param ip_addr pointer to a c-string containing server's hostname
	//! \param port_num server's port number
	//! \returns \ref TAS_ERR_SERVER_CON
	tas_return_et mHandleErrorServerConnect(const char* ip_addr, uint16_t port_num);

	//! \brief Handle wrong usages of receive and bi-directional channels.
	//! \returns \ref TAS_ERR_FN_USAGE
	tas_return_et mHandleErrorRcvChlActive();

	std::array<char, TAS_NAME_LEN32> mClientName;  	//!< \brief Client name as in \ref tas_pl1rq_server_connect_st
	
	std::array<char, TAS_NAME_LEN16> mUserName;		//!< \brief Name of the user, who created this client

	uint32_t mClientPid;				//!< \brief Process of the client application

	CTasPktHandlerServerCon mTphsc;		//!< \brief Packet handler object. Used for constructing and parsing packets.

	tas_error_info_st* mEip;  			//!< \brief Pointer to single object in primary client class

	//! \brief Object limits.
	enum { 
		IP_ADDR_NAME_BUF_SIZE = 256 	//!< \brief Maximum length of a server's hostname
	};
	std::array<char, IP_ADDR_NAME_BUF_SIZE> mServerIpAddr;	//!< \brief local storage for server's hostname
	uint16_t mServerPortNum = 0;					//!< \brief local storage for server's port number

	const tas_server_info_st* mServerInfo;			//!< \brief pointer to the server information
	uint64_t                  mServerChallenge;		//!< \brief challenge received from a server

};

//! \} // end of group Client_API