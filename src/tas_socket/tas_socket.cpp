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
#include "tas_socket.h"
// Standard includes
#include <cassert>

#ifdef _WIN32
static bool winsock_init_done = false;
#endif

CTasSocket::CTasSocket(int type, int protocol) 
{
#ifdef _WIN32
	// WinSock initialization, should be done just once
	if (!winsock_init_done) 
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
		{
			// error handling in constructor
		}
		winsock_init_done = true;
	}
#endif
	if ((mSocketDesc = (int)socket(AF_INET, type, protocol)) == INVALID_SOCKET)
	{
		// error handling
	}
	assert(mSocketDesc != INVALID_SOCKET);

	// Optimize latency by disabling Nagle algorithm
	int on = 1;
	int error = setsockopt(mSocketDesc, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
	assert(error == 0);
	(void)error; // Unused beyond assert
}

CTasSocket::CTasSocket(int socket_desc) : mSocketDesc(socket_desc) {}

CTasSocket::~CTasSocket() 
{
	close();
}

int CTasSocket::select_socket(const unsigned int msec) const
{
	if (mSocketDesc < 0)
		return -1;

	struct timeval tval;
	fd_set rset;
	int res;

	if (msec > 0)
	{
		tval.tv_sec = msec / 1000;
		tval.tv_usec = (msec % 1000) * 1000;
	}

	FD_ZERO(&rset);
	FD_SET(mSocketDesc, &rset);

	// block until socket is readable
	res = select(mSocketDesc + 1, &rset, nullptr, nullptr, &tval);

	if (res <= 0)
		return res;
	
	if (!FD_ISSET(mSocketDesc, &rset))
		return -1;

	return 1;
}

bool CTasSocket::set_option(int optname, void* arg) const
{
	switch(optname)
	{
		case SO_RCVBUF:
		{
			int option = *(int*)arg; 
			int option_set;
			socklen_t len = sizeof(socklen_t);
			
			setsockopt(mSocketDesc, SOL_SOCKET, SO_RCVBUF, (char*)&option, sizeof(option));
			getsockopt(mSocketDesc, SOL_SOCKET, SO_RCVBUF, (char*)&option_set, &len);
			
			if (option_set != option)
				return false;
			
			break;
		}
			
		case SO_SNDBUF:
		{
			int option = *(int*)arg; 
			int option_set;
			socklen_t len = sizeof(socklen_t);

			setsockopt(mSocketDesc, SOL_SOCKET, SO_SNDBUF, (char*)&option, sizeof(int));
			getsockopt(mSocketDesc, SOL_SOCKET, SO_SNDBUF, (char*)&option_set, &len);
			
			if (option_set != option)
				return false;

			break;
		}
		
		default:
			return false;
	}

	return true;
}

bool CTasSocket::set_local_port(unsigned short port) const
{
	struct sockaddr_in saddr; 
	
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(port);

	if ((bind(mSocketDesc, (sockaddr*)&saddr, sizeof(saddr))) == SOCKET_ERROR)
	{
		return false; // Unable to set local port
	}

	return true;
}

unsigned short CTasSocket::get_local_port() const
{
	struct sockaddr_in saddr;
	
	if (socklen_t saddrLen = sizeof(saddr); getsockname(mSocketDesc, (sockaddr*)&saddr, &saddrLen) == SOCKET_ERROR) 
	{
		return 0; // Unable to get local port 
	}

	return ntohs(saddr.sin_port);
}

const char* CTasSocket::get_local_addr() 
{
	struct sockaddr_in saddr;
	
	if (socklen_t saddrLen = sizeof(saddr); getsockname(mSocketDesc, (sockaddr*)&saddr, &saddrLen) == SOCKET_ERROR) 
	{
		mLocalIpv4[0] = '\0'; // empty string
		return mLocalIpv4.data(); // Unable to get local address
	}
	return inet_ntop(saddr.sin_family, &saddr.sin_addr, mLocalIpv4.data(), INET_ADDRSTRLEN); // Assumes only Ipv4!
}

bool CTasSocket::set_local_addr_and_port(const char* addr, unsigned short port) const
{
	struct sockaddr_in saddr;

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	inet_pton(AF_INET, addr, &saddr.sin_addr); // what about the return value?

	if ((bind(mSocketDesc, (sockaddr*)&saddr, sizeof(saddr))) == SOCKET_ERROR)
	{
		return false; // Unable to set local address and port
	}

	return true;
}

void CTasSocket::close()
{
#ifdef _WIN32
	closesocket(mSocketDesc);
#else
	::close(mSocketDesc);
#endif
	mSocketDesc = (int)INVALID_SOCKET;
}

int CTasSocket::get_socket_desc() 
{
	return mSocketDesc;
}

int CTasSocket::get_new_socket_desc(int type, int protocol) 
{
	// If the old descriptor is still valid return invalid socket descriptor
	if (mSocketDesc != INVALID_SOCKET) {
		return (int)INVALID_SOCKET;
	}

	mSocketDesc = (int)socket(AF_INET, type, protocol);
	return mSocketDesc;
}