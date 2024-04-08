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
#include "tas_pkt_handler_trc.h"

// Standard includes
#include <cassert>
#include <cstdio>
#include <memory>

CTasPktHandlerTrc::CTasPktHandlerTrc(tas_error_info_st* ei)
	: CTasPktHandlerBase(ei) {}

const uint32_t* CTasPktHandlerTrc::get_pkt_rq_subscribe(uint8_t stream, tas_chso_et chso)
{
	mStream = stream;
	mChso = chso;

	assert(chso <= TAS_CHSO_EXCLUSIVE);
	const uint32_t pl1PktSize = sizeof(tas_pl1rq_trc_subscribe_st);
	mRqBuf[0] = 4 + pl1PktSize;
	auto pkt = (tas_pl1rq_trc_subscribe_st*)&mRqBuf[1];
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_TRC_SUBSCRIBE;
	pkt->reserved = 0;
	pkt->stream = stream;
	pkt->chso = chso;
	pkt->reserved1 = 0;
	return mRqBuf;
}

tas_return_et CTasPktHandlerTrc::set_pkt_rsp_subscribe(const uint32_t* pkt_rsp, tas_trc_type_et* trct, tas_chso_et* chso)
{
	*chso = TAS_CHSO_DEFAULT;

	auto pkt = (const tas_pl1rsp_trc_subscribe_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_trc_subscribe_st);
	if (pkt_rsp[0] != 4 + pl1PktSize) {
		return mSetPktRspErrConnectionProtocol();
	}
	assert(pkt->reserved == 0);
	assert(pkt->reserved1 == 0);
	if ((pkt->cmd != TAS_PL1_CMD_TRC_SUBSCRIBE) || (pkt->wl != (pl1PktSize / 4) - 1)) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->stream != mStream) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->chso > TAS_CHSO_EXCLUSIVE) {
		return mSetPktRspErrConnectionProtocol();
	}

	if (pkt->err != TAS_PL_ERR_NO_ERROR) {
		if (pkt->err == TAS_PL1_ERR_CMD_FAILED) {
			return mSetPktRspErrTraceClaimed(mStream);
		}
		else if (pkt->err == TAS_PL_ERR_NOT_SUPPORTED) {
			return mSetPktRspErrTraceNotSupported();
		}
		else {
			return mSetPktRspErrConnectionProtocol();
		}
	}

	*trct = (tas_trc_type_et)pkt->trct;

	assert(pkt->chso == mChso);
	*chso = (tas_chso_et)pkt->chso;

	return tas_clear_error_info(mEip);
}

const uint32_t* CTasPktHandlerTrc::get_pkt_rq_unsubscribe(uint8_t stream)
{
	mStream = stream;

	const uint32_t pl1PktSize = sizeof(tas_pl1rq_trc_unsubscribe_st);
	mRqBuf[0] = 4 + pl1PktSize;
	auto pkt = (tas_pl1rq_trc_unsubscribe_st*)&mRqBuf[1];
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_TRC_UNSUBSCRIBE;
	pkt->stream = stream;
	pkt->reserved = 0;
	return mRqBuf;
}

tas_return_et CTasPktHandlerTrc::set_pkt_rsp_unsubscribe(const uint32_t* pkt_rsp)
{
	auto pkt = (const tas_pl1rsp_trc_unsubscribe_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_trc_unsubscribe_st);
	if (pkt_rsp[0] != 4 + pl1PktSize) {
		return mSetPktRspErrConnectionProtocol();
	}
	if ((pkt->cmd != TAS_PL1_CMD_TRC_UNSUBSCRIBE) || (pkt->wl != (pl1PktSize / 4) - 1) || (pkt->err != TAS_PL_ERR_NO_ERROR)) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->stream != mStream) {
		return mSetPktRspErrConnectionProtocol();
	}

	return tas_clear_error_info(mEip);
}

tas_return_et CTasPktHandlerTrc::set_pkt_rcv_trace(const uint32_t* pkt_rsp, const void** trace_data, uint32_t* length, tas_trcs_et* trcs, uint8_t* stream)
{
	*trace_data = nullptr;
	*length = 0;
	*trcs = TAS_TRCS_CONT;
	*stream = 0xFF;

	auto pkt = (const tas_pl1rsp_trc_data_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_trc_data_st);
	if ((pkt_rsp[0] < 4 + pl1PktSize + 32) || (pkt_rsp[0] % 4) || (pkt_rsp[0] > TAS_PL2_MAX_PKT_SIZE)) {
		return mSetPktRspErrConnectionProtocol();
	}
	if ((pkt->cmd != TAS_PL1_CMD_TRC_DATA) || (pkt->wl != (pl1PktSize / 4) - 1) || (pkt->trcs > TAS_TRCS_START_AI)) {
		return mSetPktRspErrConnectionProtocol();
	}

	*trcs = (tas_trcs_et)pkt->trcs;
	*stream = pkt->stream;

	return tas_clear_error_info(mEip);
}

tas_return_et CTasPktHandlerTrc::mSetPktRspErrTraceClaimed(uint8_t stream)
{
	if (stream == 0)
		snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Continuous trace exclusively claimed by another client");
	else
		snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Continuous trace for stream %d exclusively claimed by another client", stream);
	mEip->tas_err = TAS_ERR_FN_USAGE;
	return mEip->tas_err;
}

tas_return_et CTasPktHandlerTrc::mSetPktRspErrTraceNotSupported()
{
	snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Continuous trace not supported by this device, access HW or server");
	mEip->tas_err = TAS_ERR_FN_NOT_SUPPORTED;
	return mEip->tas_err;
}
