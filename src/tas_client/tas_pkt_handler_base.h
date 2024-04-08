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

//! \defgroup Packet_Handlers Packet Handlers
//! \brief A set of classes for constructing and parsing TAS packets.

//! \defgroup Packet_Handler_RW Read/Write (RW)
//! \brief Read/Write packet handler related classes and functions.
//! \ingroup  Packet_Handlers
//! \defgroup Packet_Handler_CHL Channel (CHL)
//! \brief Channel packet handler related classes and functions.
//! \ingroup  Packet_Handlers
//! \defgroup Packet_Handler_TRC Trace (TRC)
//! \brief Trace packet handler related classes and functions.
//! \ingroup Packet_Handlers

//! \addtogroup Packet_Handlers
//! \{

// TAS includes
#include "tas_client.h"
#include "tas_client_impl.h"

// Standard includes
#include <cstring>
#include <memory>

//! \brief A class for handling a set of common packet types to all client types.
class CTasPktHandlerBase
{

public:

	//! \brief Packet handler object constructor.
	//! \param ei pointer to the TAS error info 
	explicit CTasPktHandlerBase(tas_error_info_st* ei);

	//! \brief Base class destructor
	virtual ~CTasPktHandlerBase() = default;

	//! \brief Get a ping request packet 
	//! \param cmd command identifier, \ref TAS_PL1_CMD_PING
	//! \returns pointer to the generated request packet
	const uint32_t* get_pkt_rq_ping(tas_pl_cmd_et cmd);

	//! \brief Parses a response from a ping request.
	//! \param cmd command identifier, \ref TAS_PL1_CMD_PING or \ref TAS_PL1_CMD_SESSION_START
	//! \param client_type client type, RW, CHL, or TRC
	//! \param pkt_rsp pointer to the ping response 
	tas_return_et   set_pkt_rsp_ping(tas_pl_cmd_et cmd, tas_client_type_et client_type, const uint32_t* pkt_rsp);

	//! \brief Initial setting of con_info with the data from CTasClientServerCon
	//! \param con_info pointer to the connection info
	void set_con_info(const tas_con_info_st* con_info) { memcpy(&mConInfo, con_info, sizeof(tas_con_info_st)); }

	//! \brief Get the connection information which was retrieved by a ping or session start command.
	//! \details Assuming connection details have not changed this calls saves a ping request. The packet handler object
	//! buffers the connection information which is updated by a ping request.
	//! \returns pointer to the connection information
	const tas_con_info_st* get_con_info() const { return &mConInfo; }

	//! \brief Get the device reset count. Value updated by the derived classes based on the packet error \ref TAS_PL1_ERR_DEV_RESET
	//! or reset notification by the server.
	//! \returns the respective reset count 
	uint32_t get_device_reset_count() const { return mDeviceResetCount; };

protected:

	//! \brief Maps packet level error codes in a response packet to a TAS errro code.
	//! \param pkt_rsp pointer to a response packet
	//! \param cmd command identifier
	//! \returns \ref TAS_ERR_SERVER_CON, otherwise any other relevant TAS error code based on the PL error code
	tas_return_et mHandlePktError(const uint32_t* pkt_rsp, tas_pl_cmd_et cmd);

	//! \brief Set the server conenction error in case of a an issue with a server connection.
	//! \returns \ref TAS_ERR_SERVER_CON
	tas_return_et mSetPktRspErrConnectionProtocol();

	//! \brief Set device access error in case of device access failed.
	//! \returns \ref TAS_ERR_DEVICE_ACCESS
	tas_return_et mSetPktRspErrDeviceAccess();

	tas_error_info_st* mEip;  //!< \brief Pointer to single object in primary client class

	//! \brief protocol version supported by this class
	enum { 
		PROTOC_VER = TAS_PKT_PROTOC_VER_1 //!< \brief tas_pkt.h protocol version implemented in this class
	};  

	// This needs to be allocated and set in the derived class:
	uint32_t* mRqBuf;  //!< \brief pointer to a request buffer. For one or more PL2 packet. Allocated and set in a derived class.
	uint32_t mMaxRqSize; //!< \brief maximum request sizes in bytes. Can be more than one PL2 packet. Allocated and set in a derived class.
	uint32_t mMaxRspSize; //!< \brief maximum response sizes in bytes. Can be more than one PL2 packet. Allocated and set in a derived class.
	uint32_t mRqWiMax;	//!< \brief maximum value of the (word) index in the request buffer

	uint32_t mRqBufWi; //!< \brief current value of the (Word) index in the request buffer

	tas_con_info_st mConInfo = {}; //!< \brief local storage for the connection information

	uint16_t mDeviceConnectOption;	//!< \brief local storage for the device connection option

	uint16_t mPl1CntOutstandingOldest;  //!< \brief cnt of first outstanding request
	uint16_t mPl1CntOutstandingLast = 0xFFC0;    //!< \brief cnt of last outstanding request

	uint32_t mDeviceResetCount;	//!< \brief local storage for the device reset count

private:

	virtual void mEnforceDerivedClass() = 0;  //!< \brief pure virtual method enforce that only used in derived RW or CHL class

};

//! \} // end of group Packet_Handlers