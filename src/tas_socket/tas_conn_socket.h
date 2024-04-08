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

//! \ingroup socket_lib

#pragma once

// Socket lib includes
#include "tas_socket.h"

// Standard includes
#include <array>

//! \brief A class for socket data transmission operations
class CTasConnSocket : public CTasSocket
{
public:
	
	//! \brief Establish a connection with a remote, blocking mode by default.
	//! \details Set timeout_ms to non-zero value for non-blocking mode.
	//! \param hostname remote's IP address or it's hostname
	//! \param port remote's port number
	//! \param timeout_ms timeout in milliseconds befor attempted is canceled, default: -1
	//! \returns \c true on successful connection, otherwise \c false
	bool connect(const char* hostname, unsigned short port, int timeout_ms = -1);

	//! \brief Send data through an established connection
	//! \details It is not guaranteed that all the data is sent, check the return value
	//! If the timeout is not provided this is a blocking operation and it may block forever
	//! \param buf a pointer to a buffer containing the data to be transmitted
	//! \param len the length, in bytes, of the data in buffer pointed to by the buf parameter
	//! \param timeout_ms timeout in milliseconds, default: -1
	//! \returns the total number of bytes sent, which can be less than the number requested to be sent in the len 
	//! parameter, \c -1 in case of an error, \c 0 if the connection has been gracefully closed
	int send(const void* buf, int len, int timeout_ms = -1);

	//! \brief Tries to send all the data specified by len parameter through an established connection
	//! \details If the timeout is not provided this is a blocking operation and it may block forever
	//! \param buf a pointer to a buffer containing the data to be transmitted
	//! \param len the length, in bytes, of the data in buffer pointed to by the buf parameter
	//! \param timeout_ms timeout in milliseconds, default: -1
	//! \returns the total number of bytes sent, which can be less than the number requested to be sent in the len 
	//! parameter, \c -1 in case of an error, \c 0 if the connection has been gracefully closed
	int sendAll(const void* buf, int len, int timeout_ms = -1);

	//! \brief Receive data through an established connection
	//! \details It is not guaranteed that all the data is received, check the return value
	//! If the timeout is not provided this is a blocking operation and it may block forever
	//! \param buf pointer to a data buffer for storing the incoming data
	//! \param len the length, in bytes, of the buffer pointed to by the buf parameter or expected number of incoming 
	//! bytes which should be <= buffer size
	//! \param timeout_ms timeout in milliseconds, default: -1 
	//! \returns the number of bytes received and the buffer pointed to by the buf parameter will contain this data 
	//! received, \c -1 in case of an error, \c 0 if the connection has been gracefully closed
	int recv(void* buf, int len, int timeout_ms = -1);

	//! \brief Tries to receive all the data specified by the len parameter through an established connection
	//! \details If the timeout is not provided this is a blocking operation and it may block forever
	//! \param buf pointer to a data buffer for storing the incoming data
	//! \param len the length, in bytes, of the buffer pointed to by the buf parameter or expected number of incoming 
	//! bytes which should be <= buffer size
	//! \param timeout_ms timeout in milliseconds, default: -1 no timeout
	//! \returns the number of bytes received and the buffer pointed to by the buf parameter will contain this data 
	//! received, \c -1 in case of an error, \c 0 if the connection has been gracefully closed
	int recvAll(void* buf, int len, int timeout_ms = -1);

	//! \brief Retrieves remote's IP address
	//! \returns remote's IP address as c-string in dot notation
	const char* get_remote_ip();

	//! \brief Retrieves remote's port number
	//! \returns remote's port number 
	unsigned short get_remote_port();

private:
	//! \brief Privet method for non-blocking connect
	//! \param saptr pointer to a sockaddr struct
	//! \param salen length of the struct in Bytes
	//! \param timeout_ms timeout in milliseconds
	//! \returns \c true if connected successfully, otherwise \c false
	bool mConnectNonblock(const struct sockaddr* saptr, int salen, unsigned int timeout_ms);

	//! \brief Timeout helper function on Unix systems
	//! \param timeout_ms  timeout in milliseconds
	void mSleepMs(int timeout_ms);

	std::array<char, INET6_ADDRSTRLEN> mRemoteIp; //!< \brief buffer for remote's IP address

	//! \brief Last measured tick for mSleepMs function
	std::chrono::milliseconds mLastTick = std::chrono::milliseconds(0);

protected:
	//! \brief constructor with socket type and protocol
	//! \details Only available in derived classes
	CTasConnSocket(int type, int protocol);

	//! \brief constructor with socket descriptor
	//! \details Only available in derived classes
	explicit CTasConnSocket(int conn_sock_desc);
};
