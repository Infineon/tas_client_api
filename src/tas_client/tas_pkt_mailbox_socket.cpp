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
// TAS includes
#include "tas_pkt_mailbox_socket.h"
#include "tas_pkt.h"

// Standard includes
#include <cassert>
#include <iostream>

bool CTasPktMailboxSocket::server_connect(const char* ip_addr, uint16_t port_num)
{
	assert(!connected());

	mSocket = new CTasTcpSocket();
	if (!mSocket->connect(ip_addr, port_num))
	{
		return false;
	}
	
	return true;
}

void CTasPktMailboxSocket::config(uint32_t timeout_receive_ms, uint32_t max_num_bytes_rsp)
{
	assert(max_num_bytes_rsp % 4 == 0);
	mTimeoutReceiveMs = timeout_receive_ms;
	mMaxNumBytesRsp = max_num_bytes_rsp;
}

bool CTasPktMailboxSocket::send(const uint32_t* rq, uint32_t num_pl2_pkt)
{
	if (!connected()) {
		assert(false);
		return false;
	}

	uint32_t w = 0;
	for (uint32_t p = 0; p < num_pl2_pkt; p++) {

		uint32_t pktSize = rq[w];
		assert(pktSize % 4 == 0);
		assert(pktSize <= TAS_PL2_MAX_PKT_SIZE);

		if (!mSocketSend(&rq[w], pktSize)) {
			assert(false);
			return false;
		}
		w += pktSize / 4;
	}
	return true;
}

bool CTasPktMailboxSocket::receive(uint32_t* rsp, uint32_t* num_bytes_rsp)
{
	mRspBuf = rsp;

	*num_bytes_rsp = 0;

	assert(connected());
	if (!connected())
		return false;

	mNumBytesRsp = 0;
	if (!mReceivePl2Pkt())
		return false;

	*num_bytes_rsp = mNumBytesRsp;
	return true;
}

bool CTasPktMailboxSocket::execute(const uint32_t* rq, uint32_t* rsp, uint32_t num_pl2_pkt, uint32_t* num_bytes_rsp)
{
	mRspBuf = rsp;

	if (num_bytes_rsp)
		*num_bytes_rsp = 0;

	if (!send(rq, num_pl2_pkt)) {
		return false;
	}

	mNumBytesRsp = 0;
	for (uint32_t p = 0; p < num_pl2_pkt; p++) {
		if (!mReceivePl2Pkt())
			return false;
	}

	if (num_bytes_rsp)
		*num_bytes_rsp = mNumBytesRsp;

	return true;
}

bool CTasPktMailboxSocket::mReceivePl2Pkt()
{
	uint32_t w;
	uint32_t pktSize;

	w = mNumBytesRsp / 4;

	// Get PL2 packet size
	if (!mSocketReceive(w, 4)) {
		return false;    // Timeout case
	}
	pktSize = mRspBuf[w];

	if ((pktSize % 4 != 0) ||
		(pktSize < 8) ||
		(pktSize + (w * 4) > mMaxNumBytesRsp)) {
		assert(false);
		mSocketDisconnect();
		return false;
	}
	if (!mSocketReceive(w + 1, pktSize - 4)) {
		assert(false);  // Timeout at this point is fatal
		mSocketDisconnect();
		return false;
	}
	mNumBytesRsp += pktSize;
	return true;
}

bool CTasPktMailboxSocket::mSocketSend(const uint32_t* rq, uint32_t num_bytes)
{
	assert(num_bytes % 4 == 0);

	if (mSocket->send(rq, num_bytes) <= 0)
	{
		mSocketDisconnect();
		return false;
	}

	return true;
}

bool CTasPktMailboxSocket::mSocketReceive(uint32_t w, uint32_t num_bytes)
{
	mRspBuf[w] = 0;

	if (mSocket->recv(&mRspBuf[w], num_bytes, (int)mTimeoutReceiveMs) <= 0)
	{
		mSocketDisconnect();
		return false;
	}
	
	return (mRspBuf[w] != 0);
}
