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

//! \defgroup socket_lib TAS Socket Library

#pragma once

// Winsock library on windows
#if defined(_WIN32) || defined(_WIN64)
	#include <WinSock2.h>
	#include <ws2tcpip.h>
	#pragma comment (lib, "Ws2_32.lib")
// socket and other network libraries on unix 
#else
	#include <sys/types.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#define INVALID_SOCKET  (int)(~0)
	#define SOCKET_ERROR -1
#endif

// standard includes
#include <cstring>
#include <chrono>
#include <thread>
#include <array>

//! \brief Base socket class
//! \details Call WSACleanUp() at the end of socket usage in win applications
//! \ingroup socket_lib
class CTasSocket
{
public:
	CTasSocket(const CTasSocket&) = delete; //!< \brief delete the copy constructor
	CTasSocket operator= (const CTasSocket&) = delete; //!< \brief delete copy-assignment operator

	//! \brief socket destructor
	//! closes the socket
	~CTasSocket(); 

	//! \brief use select function on this socket
	//! \param msec timeout in milliseconds
	//! \returns \c -1 on failure 
	int select_socket(const unsigned int msec) const;

	//! \brief set a socket option
	//! \param optname option name
	//! \param arg parameters of the corresponding option
	//! \return \c false if the options was not set, otherwise \c true
	bool set_option(int optname, void* arg) const;

	//! \brief set the (socket's) local port number
	//! \param port port number 
	//! \returns \c false if the local port could not be set, otherwise \c true
	bool set_local_port(unsigned short port) const;

	//! \brief get the (socket's) local port number
	//! \returns port number 
	unsigned short get_local_port() const;

	//! \brief get the local IP address
	//! \returns c-string of an IP address in dot notation
	const char* get_local_addr();

	//! \brief set local IP address and port number
	//! \param addr c-string of an IP address in dot notation
	//! \param port port number
	//! \returns \c false if the operation fails, otherwise \c true
	bool set_local_addr_and_port(const char* addr, unsigned short port) const;

	//! \brief explicitly close the socket
	void close();

private:
	//! \brief storage for the socket's local IP address
	std::array<char, INET_ADDRSTRLEN> mLocalIpv4;
	
	int mSocketDesc; //!< \brief socket descriptor

protected:
	//! \brief get current socket descriptor value
	//! \returns socket descriptor
	int get_socket_desc();

	//! \brief get a new socket descriptor value
	//! \details It assumes that the previous socket was closed
	//! \param type socket type
	//! \param protocol socket protocol
	//! \returns socket descriptor on success or invalid socket descriptor on failure
	int get_new_socket_desc(int type, int protocol);

	CTasSocket(int type, int protocol); //!< \brief socket constructor with its type and protocol 
	explicit CTasSocket(int socket_desc); //!< \brief socket constructor with its descriptor
};