/*
 *  Copyright (c) 2026 Infineon Technologies AG.
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
#include "tas_client_chl.h"
#include "tas_utils.h"

// Standard includes
#include <cassert>
#include <cstdio>
#include <memory>
#include <array>

CTasClientChl::CTasClientChl(const char* client_name)
	: CTasClientServerCon(client_name, &mEi)
	, mTphChl(&mEi)
{
	mMbIfChl = mMbSocket;
	mMbIfChl->config(TAS_DEFAULT_TIMEOUT_MS, TAS_PL1_CHL_MAX_MSG_SIZE);

	mChlCht = TAS_CHT_NONE;
	mChlNum = TAS_CHL_NUM_MAX;
	mChlTarget = TAS_CHL_TGT_UNKNOWN;
	mRspPktBuf = nullptr;
}

CTasClientChl::CTasClientChl(CTasPktMailboxIf* mb_if)
	: CTasClientServerCon("", &mEi, mb_if)
	, mMbIfChl(mb_if), mTphChl(&mEi)
{
	mSessionStarted = true;

	mChlCht = TAS_CHT_NONE;
	mChlNum = TAS_CHL_NUM_MAX;

	mRspPktBufSize = RSP_PKT_BUF_SIZE_CHL;
	mRspPktBuf = new uint32_t[mRspPktBufSize / 4];
}

tas_return_et CTasClientChl::session_start(const char* identifier, const char* session_name, const char* session_pw, 
	                                       tas_chl_target_et chl_target, uint64_t chl_param)
{
	mChlTarget = chl_target;

	if (mChlCht != TAS_CHT_NONE) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Channel is already subscribed");
		mEi.tas_err = TAS_ERR_FN_USAGE;
		return mEi.tas_err;
	}

	tas_client_type_et clientType;
	if (chl_target == TAS_CHL_TGT_TRC) {
		clientType = TAS_CLIENT_TYPE_TRC;
		mRspPktBufSize = RSP_PKT_BUF_SIZE_TRC;
	}
	else {
		clientType = TAS_CLIENT_TYPE_CHL;
		mRspPktBufSize = RSP_PKT_BUF_SIZE_CHL;
	}

	const tas_con_info_st* conInfo;
	tas_return_et ret = mSessionStart(clientType, identifier, session_name, session_pw, &conInfo, chl_target, chl_param);
	if (ret == TAS_ERR_NONE) {
		mTphChl.set_con_info(conInfo);
	}

	if (mRspPktBufSize != 0)
		delete mRspPktBuf;
	
	mRspPktBuf = new uint32_t[mRspPktBufSize / 4];

	return ret;
}

tas_return_et CTasClientChl::device_connect(tas_clnt_dco_et dco)
{
	if (mChlCht != TAS_CHT_NONE) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Channel is already subscribed");
		mEi.tas_err = TAS_ERR_FN_USAGE;
		return mEi.tas_err;
	}

	tas_return_et ret = mDeviceConnect(dco);
	mDeviceResetCount = mTphChl.get_device_reset_count();
	return ret;
}

tas_return_et CTasClientChl::target_ping(tas_con_info_st* con_info)
{
	if (mChlCht != TAS_CHT_NONE) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Channel is already subscribed");
		mEi.tas_err = TAS_ERR_FN_USAGE;
		return mEi.tas_err;
	}

	*con_info = {};

	const uint32_t* pktRq = mTphChl.get_pkt_rq_ping(TAS_PL1_CMD_PING);

	std::array<uint32_t, (4 + sizeof(tas_pl1rsp_ping_st)) / 4> pktRsp;
	if (!mMbIfChl->execute(pktRq, pktRsp.data()))
		return tas_client_handle_error_server_con(&mEi);

	if (mTphChl.set_pkt_rsp_ping(TAS_PL1_CMD_PING, TAS_CLIENT_TYPE_CHL, pktRsp.data())) {
		return mEi.tas_err;
	}

	memcpy(con_info, mTphChl.get_con_info(), sizeof(tas_con_info_st));

	return tas_clear_error_info(&mEi);
}


tas_return_et CTasClientChl::subscribe(uint8_t chl, tas_cht_et cht, tas_chso_et chso, uint8_t* prio)
{
	if (!mSessionStarted) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Session not yet started");
		mEi.tas_err = TAS_ERR_FN_USAGE;
		return mEi.tas_err;
	}

	if (mChlCht != TAS_CHT_NONE) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Channel is already subscribed");
		mEi.tas_err = TAS_ERR_FN_USAGE;
		return mEi.tas_err;
	}

	mChlNum = TAS_CHL_NUM_MAX;

	assert(TAS_CHL_NUM_MAX == TAS_CHL_NUM_TRC + 1);
	if (chl > TAS_CHL_NUM_TRC) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Channel number %d is too high. Only 0 to %d allowed", chl, TAS_CHL_NUM_MAX - 1);
		mEi.tas_err = TAS_ERR_FN_PARAM;
		return mEi.tas_err;
	}
	else if (chl == TAS_CHL_NUM_TRC) {
		if (get_server_info()->supp_trc_type == 0) {
			snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: TasServer does not support continuous trace");
			mEi.tas_err = TAS_ERR_FN_NOT_SUPPORTED;
			return mEi.tas_err;
		}
	}
	
	if ((cht != TAS_CHT_SEND) && (cht != TAS_CHT_RCV) && (cht != TAS_CHT_BIDI)) {
		assert(false);
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Channel type is not supported");
		mEi.tas_err = TAS_ERR_FN_PARAM;
		return mEi.tas_err;
	}
	if ((chso != TAS_CHSO_DEFAULT) && (chso != TAS_CHSO_EXCLUSIVE)) {
		assert(false);
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Channel option is not supported");
		mEi.tas_err = TAS_ERR_FN_PARAM;
		return mEi.tas_err;
	}
	uint8_t prioAssigned;
	uint8_t prioRequested = TAS_CHL_LOWEST_PRIORITY;
	if (prio) {
		if (*prio <= TAS_CHL_LOWEST_PRIORITY)
			prioRequested = *prio;
	}

	const uint32_t* pktRq = mTphChl.get_pkt_rq_subscribe(chl, cht, chso, prioRequested);

	std::array<uint32_t, (4 + sizeof(tas_pl1rsp_chl_subscribe_st)) / 4> pktRsp;
	if (!mMbIfChl->execute(pktRq, pktRsp.data()))
		return tas_client_handle_error_server_con(&mEi);

	tas_cht_et chtRsp; tas_chso_et chsoRsp;
	if (mTphChl.set_pkt_rsp_subscribe(pktRsp.data(), &chtRsp, &chsoRsp, &prioAssigned))
		return mEi.tas_err;

	assert(chsoRsp == chso);
	if (prio) {
		assert(prioAssigned <= TAS_CHL_LOWEST_PRIORITY);
		if (*prio <= TAS_CHL_LOWEST_PRIORITY)
			assert(prioAssigned >= *prio);
		*prio = prioAssigned;
	}

	mChlCht = cht;  // Success
	mChlNum = chl;

	mRcvChlActive = ((mChlCht == TAS_CHT_RCV) || (mChlCht == TAS_CHT_BIDI));

	return tas_clear_error_info(&mEi);
}

tas_return_et CTasClientChl::send_msg(const void* msg, uint32_t msg_length, uint32_t init)
{
	if ((mChlCht != TAS_CHT_SEND) && (mChlCht != TAS_CHT_BIDI)) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Channel not subscribed for sending messages");
		mEi.tas_err = TAS_ERR_FN_USAGE;
		return mEi.tas_err;
	}

	uint32_t msgLength = (init == 0) ? msg_length : msg_length + 4;
	if (mChlTarget == TAS_CHL_TGT_TRC) {
		if (msgLength > 260) {
			assert(false);
			snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: msg_length too high for TAS_CHL_TGT_TRC");
			mEi.tas_err = TAS_ERR_FN_PARAM;
			return mEi.tas_err;
		}
	}
	else {
		if (uint32_t msgLengthC2d = mTphChl.get_con_info()->msg_length_c2d; msgLength > msgLengthC2d) {
			assert(false);
			snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: msg_length %d higher than msg_length_c2d %d", msg_length, msgLengthC2d);
			mEi.tas_err = TAS_ERR_FN_PARAM;
			return mEi.tas_err;
		}
	}

	if (const uint32_t* pktRq = mTphChl.get_pkt_send_msg(mChlNum, msg, msg_length, init); !mMbIfChl->send(pktRq))
		return tas_client_handle_error_server_con(&mEi);

	return tas_clear_error_info(&mEi);
}

tas_return_et CTasClientChl::rcv_msg(uint32_t timeout_ms, const uint32_t** msg, uint32_t* msg_length, uint32_t* init)
{
	*msg = nullptr;
	*msg_length = 0;
	*init = 0;

	if ((mChlCht != TAS_CHT_RCV) && (mChlCht != TAS_CHT_BIDI)) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Channel not subscribed for receiving messages");
		mEi.tas_err = TAS_ERR_FN_USAGE;
		return mEi.tas_err;
	}

	mMbIfChl->config(timeout_ms, mRspPktBufSize);

	uint32_t rspNumBytesReceived;
	if (!mMbIfChl->receive(mRspPktBuf, &rspNumBytesReceived)) {
		assert(rspNumBytesReceived == 0);
		tas_clear_error_info(&mEi);
		return TAS_ERR_CHL_RCV;  // Not TAS_ERR_NONE
	}

	const uint32_t* msgRcv; uint16_t msgLengthRcv; uint32_t initRcv;
	if (mTphChl.set_pkt_rcv_msg(mRspPktBuf, mChlNum, &msgRcv, &msgLengthRcv, &initRcv))
		return mEi.tas_err;

	if (mChlTarget == TAS_CHL_TGT_DMM) {
		if (msgLengthRcv > mTphChl.get_con_info()->msg_length_d2c) {
			assert(false);
			snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Received message is longer than msg_length_d2c");
			mEi.tas_err = TAS_ERR_GENERAL;
			return mEi.tas_err;
		}
	}

	*msg = msgRcv;
	*msg_length = msgLengthRcv;
	*init = initRcv;

	return tas_clear_error_info(&mEi);
}


