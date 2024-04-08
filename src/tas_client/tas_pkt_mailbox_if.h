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

// Standard includes
#include <cstdint>

//! \brief A pure virtual class defining the packet mailbox interface
struct CTasPktMailboxIf
{

public:

	//! \brief Base class destructor
	virtual ~CTasPktMailboxIf() = default;

	//! \brief Configure receive timeout and response packet size.
	//! \param timeout_receive_ms timeout value in milliseconds
	//! \param max_num_bytes_rsp maximum size of a response in bytes
	virtual void config(uint32_t timeout_receive_ms, uint32_t max_num_bytes_rsp) = 0;

	//! \brief Check if connected.
	//! \returns \c true if yes, otherwise \c false
	virtual bool connected() = 0;

	//! \brief Send out a request packet.
	//! \details The method call is blocking until all PL2 packets have been sent.
	//! \param rq pointer to a request packet buffer, can contain more than one PL2 defined by the num_pl2_pkt value
	//! \param num_pl2_pkt number of PL2 packets within the buffer
	//! \returns \c true on success, \c false in case of an error
	virtual bool send(const uint32_t* rq, uint32_t num_pl2_pkt = 1) = 0;

	//! \brief Receive response packet.
	//! \details The method call is blocking until a packet is received or a timeout occurred.
	//! \param rsp pointer to a response packet buffer
	//! \param num_bytes_rsp pointer to a storage for the number of bytes in the response packet
	//! \returns \c true on success, \c false in case of an error, timeout is not an error!
	virtual bool receive(uint32_t* rsp, uint32_t* num_bytes_rsp) = 0;

	//! \brief Send out a request and wait for a response
	//! \details The method call is blocking until all PL2 packets have been sent and received or a timeout occurred.
	//! If num_pl2_pkt = 1, rq is a single PL2 packet. In this case num_bytes_rsp is not needed as well.
	//! \param rq pointer to a request packet buffer, can contain more than one PL2 defined by the num_pl2_pkt value
	//! \param rsp pointer to a response packet buffer
	//! \param num_pl2_pkt number of PL2 packets within the buffer
	//! \param num_bytes_rsp pointer to a storage for the number of bytes in the response packet
	//! \returns \c true on success, \c false in case of an error, timeout is not an error!
	virtual bool execute(const uint32_t* rq, uint32_t* rsp, uint32_t num_pl2_pkt = 1, uint32_t* num_bytes_rsp = nullptr) = 0;

};

//! \} // end of group Client_API