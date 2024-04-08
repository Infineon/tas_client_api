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
#include "tas_conn_socket.h"

//! \brief TCP socket class
class CTasTcpSocket : public CTasConnSocket 
{
public:
	//! \brief TCP socket constructor
	//! \details This creates an object that represents a stream socket which uses the TCP protocol	
	CTasTcpSocket();

protected:
	//! \brief a friend class definition
	//! \details make the CTasTcpServerSocket class a friend of CTasTcpSocket class so it can access 
	//! CTasTcpSocket(int tcp_sock_desc) constructor, which should not be available to the user
	friend class CTasTcpServerSocket; 

	//! \brief TCP socket constructor with a socket descriptor
	explicit CTasTcpSocket(int tcp_sock_desc);
};