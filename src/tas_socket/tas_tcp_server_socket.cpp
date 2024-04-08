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

// Socket lib includes
#include "tas_tcp_server_socket.h"

CTasTcpServerSocket::CTasTcpServerSocket() : CTasSocket(SOCK_STREAM, IPPROTO_TCP)
{
	
}

bool CTasTcpServerSocket::listen(unsigned short port)
{
	if (!set_local_port(port))
		return false;

	if (::listen(get_socket_desc(), 5) == SOCKET_ERROR) 
	{
		return false; // listen failed
	}

	return true;
}

bool CTasTcpServerSocket::listen(const char* addr, unsigned short port)
{
	if (!set_local_addr_and_port(addr, port))
		return false;

	if (::listen(get_socket_desc(), 5) == SOCKET_ERROR)
	{
		return false; // listen failed 
	}
	return true;
}

CTasTcpSocket* CTasTcpServerSocket::accept() 
{
	int sock;
	if ((sock = (int)::accept(get_socket_desc(), nullptr, nullptr)) == INVALID_SOCKET) 
	{
		// accept failed
		return nullptr;
	}

	return new CTasTcpSocket(sock);
}