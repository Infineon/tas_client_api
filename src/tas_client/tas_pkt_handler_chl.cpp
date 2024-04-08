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
#include "tas_pkt_handler_chl.h"

// Standard includes
#include <cassert>
#include <cstdio>
#include <memory>


CTasPktHandlerChl::CTasPktHandlerChl(tas_error_info_st* ei)
	: CTasPktHandlerBase(ei)
{
	// From CTasPktHandlerBase, allocated and set here in the derived class:
	mMaxRqSize = TAS_PL1_CHL_MAX_MSG_SIZE + 64;
	mRqBuf = new uint32_t[mMaxRqSize / 4];
	mRqWiMax = mMaxRqSize / 4;
	mMaxRspSize = mMaxRqSize;

	mDeviceResetCount = 0;
}

CTasPktHandlerChl::~CTasPktHandlerChl()
{
	delete[] mRqBuf;
}

const uint32_t* CTasPktHandlerChl::get_pkt_rq_subscribe(uint8_t chl, tas_cht_et cht, tas_chso_et chso, uint8_t prio)
{
	assert(chl < TAS_CHL_NUM_MAX);
	mChl = chl;
	assert((cht == TAS_CHT_SEND) || (cht == TAS_CHT_RCV) || (cht == TAS_CHT_BIDI));
	mCht = cht;
	assert(chso <= TAS_CHSO_EXCLUSIVE);
	mChso = chso;
	assert(prio <= TAS_CHL_LOWEST_PRIORITY);

	const uint32_t pl1PktSize = sizeof(tas_pl1rq_chl_subscribe_st);
	mRqBuf[0] = 4 + pl1PktSize;
	auto pkt = (tas_pl1rq_chl_subscribe_st*)&mRqBuf[1];
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_CHL_SUBSCRIBE;
	pkt->reserved = 0;
	pkt->chl = chl;
	pkt->cht = cht;
	pkt->chso = chso;
	pkt->prio = prio;
	return mRqBuf;
}

tas_return_et CTasPktHandlerChl::set_pkt_rsp_subscribe(const uint32_t* pkt_rsp, tas_cht_et* cht, tas_chso_et* chso, uint8_t* prio)
{
	*cht = TAS_CHT_NONE;
	*chso = TAS_CHSO_DEFAULT;
	*prio = 0xFF;

	auto pkt = (const tas_pl1rsp_chl_subscribe_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_chl_subscribe_st);
	if (pkt_rsp[0] != 4 + pl1PktSize) {
		return mSetPktRspErrConnectionProtocol();
	}
	assert(pkt->reserved == 0);
	if ((pkt->cmd != TAS_PL1_CMD_CHL_SUBSCRIBE) || (pkt->wl != (pl1PktSize / 4) - 1)) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->chl != mChl) {
		return mSetPktRspErrConnectionProtocol();
	}
	if ((pkt->err == TAS_PL_ERR_NO_ERROR) && (pkt->cht != TAS_CHT_SEND) && (pkt->cht != TAS_CHT_RCV) && (pkt->cht != TAS_CHT_BIDI)) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->chso > TAS_CHSO_EXCLUSIVE) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->prio > TAS_CHL_LOWEST_PRIORITY) {
		return mSetPktRspErrConnectionProtocol();
	}

	if (pkt->err != TAS_PL_ERR_NO_ERROR) {
		if (pkt->err == TAS_PL1_ERR_CMD_FAILED && pkt->cht != mCht) {
			assert(pkt->cht == TAS_CHT_NONE);
			if (mChso == TAS_CHSO_EXCLUSIVE) {
				snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Could not exclusively subscribe to channel %d since already used by another client", mChl);
			}
			else {
				assert(mChso == TAS_CHSO_DEFAULT);
				snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Could not subscribe to channel %d since exclusively used by another client", mChl);
			}
			mEip->tas_err = TAS_ERR_CHL_SETUP;
			return mEip->tas_err;
		}
		else {
			return mSetPktRspErrConnectionProtocol();
		}
	}

	*cht  = (tas_cht_et)pkt->cht;
	*chso = (tas_chso_et)pkt->chso;
	*prio = pkt->prio;

	return tas_clear_error_info(mEip);
}

const uint32_t* CTasPktHandlerChl::get_pkt_rq_unsubscribe(uint8_t chl)
{
	assert(chl < TAS_CHL_NUM_MAX);
	mChl = chl;

	const uint32_t pl1PktSize = sizeof(tas_pl1rq_chl_unsubscribe_st);
	mRqBuf[0] = 4 + pl1PktSize;
	auto pkt = (tas_pl1rq_chl_unsubscribe_st*)&mRqBuf[1];
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_CHL_UNSUBSCRIBE;
	pkt->chl = chl;
	pkt->reserved = 0;
	return mRqBuf;
}

tas_return_et CTasPktHandlerChl::set_pkt_rsp_unsubscribe(const uint32_t* pkt_rsp)
{
	auto pkt = (const tas_pl1rsp_chl_unsubscribe_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_chl_unsubscribe_st);
	if (pkt_rsp[0] != 4 + pl1PktSize) {
		return mSetPktRspErrConnectionProtocol();
	}
	if ((pkt->cmd != TAS_PL1_CMD_CHL_UNSUBSCRIBE) || (pkt->wl != (pl1PktSize / 4) - 1) || (pkt->err != TAS_PL_ERR_NO_ERROR)) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->chl != mChl) {
		return mSetPktRspErrConnectionProtocol();
	}

	return tas_clear_error_info(mEip);
}

const uint32_t* CTasPktHandlerChl::get_pkt_send_msg(uint8_t chl, const void* msg, uint16_t msg_length, uint32_t init)
{
	mChl = 0xFF;  // The next rcv msg can be from any subscribed channel
	assert(chl < TAS_CHL_NUM_MAX);

	uint16_t msgLength = (init == 0) ? msg_length : msg_length + 4;
	uint16_t msgWl = (msgLength + 3) / 4;

	const uint32_t pl1PktSize = sizeof(tas_pl1rq_chl_subscribe_st);
	mRqBuf[0] = 4 + pl1PktSize + msgWl * 4;
	auto pkt = (tas_pl1rq_chl_msg_c2d_st*)&mRqBuf[1];
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_CHL_MSG_C2D;
	pkt->reserved = 0;
	pkt->chl = chl;
	pkt->msg_length = msgLength;

	if (init != 0) {
		pkt->cho = TAS_CHO_INIT;
		mRqBuf[3] = init;
		assert(msg_length <= 252);
		memcpy(&mRqBuf[4], msg, msg_length);
	}
	else {
		pkt->cho = TAS_CHO_NONE;
		assert(msg_length <= 256);
		memcpy(&mRqBuf[3], msg, msg_length);
	}

	return mRqBuf;
}

tas_return_et CTasPktHandlerChl::set_pkt_rcv_msg(const uint32_t* pkt_rsp, uint8_t chl, const void** msg, uint16_t* msg_length, uint32_t* init)
{
	*msg = nullptr;
	*msg_length = 0;
	*init = 0;

	auto pkt = (const tas_pl1rsp_chl_msg_d2c_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_chl_msg_d2c_st);

	if ((pkt_rsp[0] == 4 + pl1PktSize) && 
		(pkt->cmd == TAS_PL1_CMD_CHL_MSG_D2C) && 
		(pkt->err == TAS_PL1_ERR_DEV_RESET)) {
		if ((pkt->wl != 1) || (pkt->chl != 0) || (pkt->cho != 0) || (pkt->msg_length != 0)) {
			return mSetPktRspErrConnectionProtocol();
		}
		mDeviceResetCount++;
		return tas_clear_error_info(mEip);  // Reset notification received
	}

	if (pkt_rsp[0] < 4 + pl1PktSize + 4) {  // Minimum length
		return mSetPktRspErrConnectionProtocol();
	}
	assert(pkt->reserved == 0);
	if ((pkt->cmd != TAS_PL1_CMD_CHL_MSG_D2C) || (pkt->wl != (pl1PktSize / 4) - 1)) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->err == TAS_PL1_ERR_DEV_RESET) {
		mDeviceResetCount++;
	}
	else if (pkt->err != TAS_PL_ERR_NO_ERROR) {
		return mSetPktRspErrConnectionProtocol();
	}
	if ((pkt->chl >= TAS_CHL_NUM_MAX) || (pkt->cho > TAS_CHO_INIT)) {
		return mSetPktRspErrConnectionProtocol();
	}
	
	if (uint32_t pl2Length = 4 + 8 + ((pkt->msg_length + 3) / 4) * 4; pkt_rsp[0] != pl2Length) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->chl != chl) {
		return mSetPktRspErrConnectionProtocol();
	}

	const uint32_t offset = (sizeof(uint32_t) + sizeof(tas_pl1rsp_chl_msg_d2c_st)) / sizeof(uint32_t); // packet size (1 word) + msg_d2c_st (2 words)
	if (pkt->cho == TAS_CHO_INIT) {
		assert(pkt->msg_length >= 4);  // Is already checked above
		*msg_length = pkt->msg_length - 4;
		*msg = &pkt_rsp[offset+1];
		*init = pkt_rsp[offset];
	}
	else {
		assert(pkt->cho == TAS_CHO_NONE);
		*msg_length = pkt->msg_length;
		*msg = &pkt_rsp[offset];
		*init = 0;
	}

	return tas_clear_error_info(mEip);
}
