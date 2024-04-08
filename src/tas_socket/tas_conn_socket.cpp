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
#include "tas_conn_socket.h"

CTasConnSocket::CTasConnSocket(int type, int protocol) : CTasSocket(type, protocol) {};

CTasConnSocket::CTasConnSocket(int conn_sock_desc) : CTasSocket(conn_sock_desc) {};

bool CTasConnSocket::connect(const char* hostname, unsigned short port, int timeout_ms) 
{
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET; // support only IPv4 addresses
	saddr.sin_port = htons(port);

	struct addrinfo hints; 
	struct addrinfo *res = nullptr;
	struct addrinfo *ptr = nullptr;
	int errcode;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // look only for IPv4 addresses
	hints.ai_socktype = SOCK_STREAM; // look for TCP
	hints.ai_flags = AI_CANONNAME;

	errcode = getaddrinfo(hostname, nullptr, &hints, &res);
	if (errcode != 0)
		return false; // hostname could not be resolved

	bool isConnected = false;
	for (ptr = res; ptr != nullptr && !isConnected; ptr = ptr->ai_next) 
	{
		if (ptr->ai_family == AF_INET)
		{
			if (get_socket_desc() == INVALID_SOCKET) { // Check if the socket was closed in the previous iteration
				if (get_new_socket_desc(ptr->ai_socktype, ptr->ai_protocol) == INVALID_SOCKET) {
					continue;
				}
			}

			saddr.sin_addr = ((struct sockaddr_in*)ptr->ai_addr)->sin_addr;
			if (timeout_ms < 0) { // use blocking connect
				isConnected = (::connect(get_socket_desc(), (sockaddr*)&saddr, sizeof(saddr))) == 0;
			} else { // use non blocking connect with a set timeout, if 0 it returns immediately 
				isConnected = mConnectNonblock((sockaddr*)&saddr, sizeof(saddr), timeout_ms);
			}

			if (!isConnected) {
				this->close();
			}
		}
	}

	if (ptr == nullptr && !isConnected)
		return false;

	freeaddrinfo(res);

	return true;
}

int CTasConnSocket::send(const void* buf, int len, int timeout_ms)
{
	int ret;
	int flags;
	int sockDesc = get_socket_desc();

	if (timeout_ms >= 0) {
		fd_set setW;
		struct timeval timeout;

		FD_ZERO(&setW);
		FD_SET(sockDesc, &setW);

		timeout.tv_sec = timeout_ms == 0 ? 0 : timeout_ms / 1000;
		timeout.tv_usec = timeout_ms == 0 ? 1 : (timeout_ms % 1000) * 1000;

		ret = ::select(sockDesc + 1, nullptr, &setW, nullptr, &timeout);
	}

#ifdef _WIN32
	flags = 0;
#else
	flags = MSG_NOSIGNAL;
#endif
	ret = ::send(sockDesc, (const char*)buf, len, flags);

	return ret;
}

int CTasConnSocket::sendAll(const void* buf, int len, int timeout_ms)
{
	int totalSent = 0;
	int bytesLeft = len;
	int n = 0;

	while (totalSent < len) {
		n = send((const char*)buf + totalSent, bytesLeft, timeout_ms);
		if (n == -1) { break; }
		totalSent += n;
		bytesLeft -= n;
	}

	return n == -1 ? -1 : 0;
}

int CTasConnSocket::recv(void* buf, int len, int timeout_ms)
{
	int ret;
	int flags;
	int sockDesc = get_socket_desc();

	if (timeout_ms >= 0) {
		fd_set setR;
		struct timeval timeout;

		FD_ZERO(&setR);
		FD_SET(sockDesc, &setR);

		timeout.tv_sec = timeout_ms == 0 ? 0 : timeout_ms / 1000;
		timeout.tv_usec = timeout_ms == 0 ? 1 : (timeout_ms % 1000) * 1000;

		ret = ::select(sockDesc + 1, &setR, nullptr, nullptr, &timeout);
		if (ret == 0) {
			return 1;  // Timeout is no error
		}
	}

#ifdef _WIN32
	flags = 0;
#else
	flags = MSG_NOSIGNAL;
#endif
	ret = ::recv(sockDesc, (char*)buf, len, flags);

	return ret;
}

int CTasConnSocket::recvAll(void* buf, int len, int timeout_ms)
{
	int totalRecvd = 0;
	int bytesLeft = len;
	int n = 0;

	while (totalRecvd < len) {
		n = recv((char*)buf + totalRecvd, len, timeout_ms);
		if (n == -1) { break; }
		totalRecvd += n;
		bytesLeft -= n;
	}

	return n == -1 ? -1 : 0;
}

const char* CTasConnSocket::get_remote_ip()
{
	struct sockaddr_storage saddr;

	if (socklen_t saddrLen = sizeof(saddr); getpeername(get_socket_desc(), (struct sockaddr*)&saddr, &saddrLen)) 
	{
		mRemoteIp[0] = '\0';
		return mRemoteIp.data(); // Unable to get remote IP address
	}

	if (saddr.ss_family == AF_INET) // Ipv4
	{
		auto s = (struct sockaddr_in*)&saddr;
		inet_ntop(AF_INET, &s->sin_addr, mRemoteIp.data(), sizeof(mRemoteIp));
	}
	else // Ipv6
	{
		auto s = (struct sockaddr_in6*)&saddr;
		inet_ntop(AF_INET6, &s->sin6_addr, mRemoteIp.data(), sizeof(mRemoteIp));
	}

	return mRemoteIp.data();
}

unsigned short CTasConnSocket::get_remote_port()
{
	struct sockaddr_storage saddr; // suitable for hadnling both ipv4 and ipv6

	if (socklen_t saddrLen = sizeof(saddr); getpeername(get_socket_desc(), (struct sockaddr*)&saddr, &saddrLen) == SOCKET_ERROR) 
	{
		return 0; // Unable to get remote port
	}

	if (saddr.ss_family == AF_INET) // Ipv4 
	{
		auto s = (struct sockaddr_in*)&saddr;
		return ntohs(s->sin_port);
	}
	else // Ipv6
	{
		auto s = (struct sockaddr_in6*)&saddr;
		return ntohs(s->sin6_port);
	}
}

bool CTasConnSocket::mConnectNonblock(const struct sockaddr* saptr, int salen, unsigned int timeout_ms)
{
	int ret = 0;
	unsigned long mode = 1;
	int sockDesc = get_socket_desc();

#ifdef _WIN32
	// set socket into non-blocking mode
	if (ioctlsocket(sockDesc, FIONBIO, &mode) == SOCKET_ERROR)
	{
		return false; // Failed to set non-blocking mode
	}

	if ((::connect(sockDesc, saptr, salen)) == SOCKET_ERROR)
	{
		if ((WSAGetLastError() != WSAEWOULDBLOCK) && (WSAGetLastError() != WSAEINPROGRESS))
		{
			// connect failed
			ret = -1;
		}
		else
		{
			// connection pedning
			fd_set setW;
			FD_ZERO(&setW);
			FD_SET(sockDesc, &setW);

			timeval timeout;
			timeout.tv_sec = timeout_ms / 1000;
			timeout.tv_usec = (timeout_ms % 1000) * 1000;

			if (::select(sockDesc + 1, nullptr, &setW, nullptr, &timeout) == 0)
			{
				// timedout
				WSASetLastError(WSAETIMEDOUT);
				ret = -1;
			}

			if (FD_ISSET(sockDesc, &setW))
			{
				int error = 0; socklen_t len = sizeof(error);
				if (getsockopt(sockDesc, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == 0) {
					WSASetLastError(error);
				}
				if (error != 0) ret = -1;
			}
		}
	}

	// Restore original state
	mode = 0;
	if (ioctlsocket(sockDesc, FIONBIO, &mode) == SOCKET_ERROR)
	{
		return false; // Failed to reset blocking mode 
	}
#else
	// set socket into non-blocking mode
	if (ioctl(sockDesc, FIONBIO, &mode) == SOCKET_ERROR)
	{
		return false; // Failed to set non-blocking mode
	}

	if ((::connect(sockDesc, saptr, salen)) == SOCKET_ERROR)
	{
		if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS))
		{
			// connect failed
			mSleepMs(timeout_ms);
			ret = -1;
		}
		else
		{
			// connection pedning
			fd_set setW;
			FD_ZERO(&setW);
			FD_SET(sockDesc, &setW);

			timeval timeout;
			timeout.tv_sec = timeout_ms / 1000;
			timeout.tv_usec = (timeout_ms % 1000) * 1000;

			if (::select(sockDesc + 1, nullptr, &setW, nullptr, &timeout) == 0)
			{
				// timedout
				errno = ETIMEDOUT;
				ret = -1;
			}

			if (FD_ISSET(sockDesc, &setW))
			{
				int error = 0; 
				if (socklen_t len = sizeof(error); getsockopt(sockDesc, SOL_SOCKET, SO_ERROR, &error, &len) == 0) {
					errno = error;
				}
				if (error != 0) {
					mSleepMs(timeout_ms);
					ret = -1;
				}
			}
		}
	}

	// Restore original state
	mode = 0;
	if (ioctl(sockDesc, FIONBIO, &mode) == SOCKET_ERROR)
	{
		return false; // Failed to reset blocking mode
	}
#endif

	if (ret < 0) {
		return false; // Failed to connect 
	}

	return true;
}

void CTasConnSocket::mSleepMs(int timeout_ms)
{
	auto timeout = std::chrono::milliseconds(timeout_ms);
	auto currentTick = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
	
	if (auto tickDiff = std::chrono::duration_cast<std::chrono::milliseconds>(currentTick - mLastTick); tickDiff < timeout) {
		std::this_thread::sleep_for(timeout - tickDiff);
	}

	mLastTick = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}