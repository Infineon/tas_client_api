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
#include "tas_pkt_mailbox_if.h"

// TAS Socket includes
#include "tas_tcp_socket.h"

//! \brief Derived mailbox class utilizing socket connection.
class CTasPktMailboxSocket : public CTasPktMailboxIf
{

public:
	CTasPktMailboxSocket(const CTasPktMailboxSocket&) = delete; //!< \brief delete the copy constructor
	CTasPktMailboxSocket operator= (const CTasPktMailboxSocket&) = delete; //!< \brief delete copy-assignment operator

	//! \brief Mailbox constructor.
	CTasPktMailboxSocket() = default;

	//! \brief Mailbox destructor for clean up.
	~CTasPktMailboxSocket()
	{
		mSocketDisconnect();
	}

	//! \brief Connect to a TAS server.
	//! \param ip_addr server's IP address or a hostname
	//! \param port_num server's port number
	//! \returns \c true on success, otherwise \c false
	bool server_connect(const char* ip_addr, uint16_t port_num);

	// CTasPktMailboxIf
	void config(uint32_t timeout_receive_ms, uint32_t max_num_bytes_rsp);
	bool connected() { return (mSocket != nullptr); }
	bool send(const uint32_t* rq, uint32_t num_pl2_pkt = 1);
	bool receive(uint32_t* rsp, uint32_t* num_bytes_rsp);
	bool execute(const uint32_t* rq, uint32_t* rsp, uint32_t num_pl2_pkt = 1, uint32_t* num_bytes_rsp = nullptr);

private:

	//! \brief Receive a PL2 packet
	//! \returns \c true on success, otherwise \c false
	bool mReceivePl2Pkt();

	//! \brief Send data through the socket.
	//! \param rq pointer to a request buffer
	//! \param num_bytes length of the request in bytes
	//! \returns \c true on success, otherwise \c false
	bool mSocketSend(const uint32_t* rq, uint32_t num_bytes);

	//! \brief Receive data from the socket. 
	//! \param w index in the mRspBuf
	//! \param num_bytes number of expected bytes
	//! \returns \c true on success, otherwise \c false
	bool mSocketReceive(uint32_t w, uint32_t num_bytes); 

	//! \brief Disconnect the socket. Used in case of a fetal error.
	void mSocketDisconnect()  
	{
		delete mSocket;
		mSocket = nullptr;
	}

	CTasTcpSocket* mSocket = nullptr;		//!< \brief Pointer to a socket instance which is connected to a TAS server

	uint32_t mTimeoutReceiveMs = 0;	//!< \brief Timeout value in milliseconds for the receive operation

	uint32_t  mMaxNumBytesRsp = 0;	//!< \brief Defines the maximum number of bytes in a response packet

	uint32_t* mRspBuf = nullptr;	//!< \brief Pointer to a response packet buffer
	uint32_t  mNumBytesRsp = 0;		//!< \brief Number of bytes in the response packet buffer
};

//! \} // end of group Client_API