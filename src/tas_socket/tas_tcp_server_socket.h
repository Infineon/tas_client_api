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
#include "tas_tcp_socket.h"

//! \brief TCP server socket class
class CTasTcpServerSocket : public CTasSocket 
{
public:
	//! \brief TCP Server socket constructor
	//! \details This creates an object that represents a stream socket which uses the TCP protocol
	CTasTcpServerSocket();

	//! \brief Open a listening socket
	//! \param port port number on which to listen for new connections
	//! \returns \c false if listen fails, otherwise \c true
	bool listen(unsigned short port);

	//! \brief Bind to an address and open a listening socket
	//! \param addr IP address to which the listening socket is bound to
	//! \param port port number on which to listen for new connections
	//! \returns \c false if listen fails, otherwise \c true
	bool listen(const char* addr, unsigned short port);
	
	//! \brief Accept a new connection
	//! \returns a pointer to a TCP socket object for which the connection was accepted
	CTasTcpSocket* accept();
};