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
#include "tas_pkt_handler_base.h"
#include "tas_pkt.h"
#include "tas_utils.h"

// Standard includes
#include <cassert>
#include <cstring>
#include <cstdio>
#include <memory>
#include <array>

CTasPktHandlerBase::CTasPktHandlerBase(tas_error_info_st* ei)
	: mEip(ei) {}

tas_return_et CTasPktHandlerBase::set_pkt_rsp_ping(tas_pl_cmd_et cmd, tas_client_type_et client_type, const uint32_t* pkt_rsp)
{
	assert((cmd == TAS_PL1_CMD_SESSION_START) || (cmd == TAS_PL1_CMD_PING));

	mConInfo = {};

	auto pkt = (const tas_pl1rsp_ping_st*)&pkt_rsp[1];

	// perhaps there is a better way to reuse this in tests for target side parser
	const uint32_t pl1PktSize = client_type == TAS_CLIENT_TYPE_UNKNOWN ? sizeof(tas_pl1rsp_ping_d2s_st) : sizeof(tas_pl1rsp_ping_st);
	if (pkt_rsp[0] != 4 + pl1PktSize) {
		return mHandlePktError(pkt_rsp, cmd);
	}
	else if ((pkt->cmd != cmd) || (pkt->wl != (pl1PktSize / 4) - 1)) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if ((PROTOC_VER < pkt->protoc_ver_min) || (PROTOC_VER > pkt->protoc_ver_max)) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if (!( (pkt->err == TAS_PL_ERR_NO_ERROR) || (pkt->err == TAS_PL1_ERR_CMD_FAILED) ||
				(pkt->err == TAS_PL1_ERR_SESSION) || (pkt->err == TAS_PL_ERR_NOT_SUPPORTED))) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if (pkt->err == TAS_PL1_ERR_SESSION) {
		snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Session name or password");
		mEip->tas_err = TAS_ERR_FN_PARAM;
		return mEip->tas_err;
	}
	else if (pkt->err == TAS_PL_ERR_NOT_SUPPORTED) {
		snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Parameter refused by server");
		mEip->tas_err = TAS_ERR_FN_PARAM;
		return mEip->tas_err;
	}
	else if (pkt->err == TAS_PL1_ERR_CMD_FAILED) {
		if (client_type == TAS_CLIENT_TYPE_CHL) {
			snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Could not read channel description from device");
			mEip->tas_err = TAS_ERR_CHL_SETUP;
		}
		else {
			snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Target not connected");
			mEip->tas_err = TAS_ERR_DEVICE_ACCESS;
		}
		return mEip->tas_err;
	}
	else {
		assert(pkt->err == TAS_PL_ERR_NO_ERROR);
		uint32_t mpsRq = pkt->con_info.max_pl2rq_pkt_size;
		uint32_t mpsRsp = pkt->con_info.max_pl2rsp_pkt_size;
		uint16_t msgLenC2d = pkt->con_info.msg_length_c2d;
		uint16_t msgLenD2c= pkt->con_info.msg_length_d2c;
		if ((mpsRq % 4 != 0) || (mpsRq < TAS_PL2_MAX_PKT_SIZE_MIN) || (mpsRq > TAS_PL2_MAX_PKT_SIZE)) {
			return mSetPktRspErrConnectionProtocol();
		}
		else if ((mpsRsp % 4 != 0) || (mpsRsp < TAS_PL2_MAX_PKT_SIZE_MIN) || (mpsRsp > TAS_PL2_MAX_PKT_SIZE)) {
			return mSetPktRspErrConnectionProtocol();
		}
		else if ((msgLenC2d % 4 != 0) || (msgLenC2d > TAS_PL1_CHL_MAX_MSG_SIZE)) {
			return mSetPktRspErrConnectionProtocol();
		}
		else if ((msgLenD2c % 4 != 0) || (msgLenD2c > TAS_PL1_CHL_MAX_MSG_SIZE)) {
			return mSetPktRspErrConnectionProtocol();
		}
		else {  // Success
			memcpy(&mConInfo, &pkt->con_info, sizeof(tas_con_info_st));
			assert(!(mConInfo.identifier[0] == 0 && client_type != TAS_CLIENT_TYPE_UNKNOWN)); // perhaps there is a better way to identify target parser			
		
			if (std::array<uint32_t, 4> zeros = {}; memcmp(mConInfo.device_id, zeros.data(), 16) == 0) {
				mConInfo.device_id_hash = 0;
				snprintf(mConInfo.device_id_hash_str, 6, "NoUid");
			}
			else {
				mConInfo.device_id_hash = tasutil_crc32(mConInfo.device_id, 16);
				tasutil_hash32_to_str(mConInfo.device_id_hash, mConInfo.device_id_hash_str);
			}
			return tas_clear_error_info(mEip);
		}
	}
}

const uint32_t* CTasPktHandlerBase::get_pkt_rq_ping(tas_pl_cmd_et cmd)
{
	assert(cmd == TAS_PL1_CMD_PING);
	const uint32_t pl1PktSize = sizeof(tas_pl1rq_ping_st);
	mRqBuf[0] = 4 + pl1PktSize;
	auto pkt = (tas_pl1rq_ping_st*)&mRqBuf[1];
	*pkt = {};
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = cmd;
	return mRqBuf;
}

tas_return_et CTasPktHandlerBase::mHandlePktError(const uint32_t* pkt_rsp, tas_pl_cmd_et cmd)
{
	if (auto pkt = (const tas_pl1rsp_header_st*)&pkt_rsp[1]; (pkt_rsp[0] == 4 + sizeof(tas_pl1rsp_header_st)) && 
		(pkt->cmd == cmd)) {

		switch (pkt->err) {
		case TAS_PL_ERR_USAGE:
			snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Wrong TAS API usage");
			mEip->tas_err = TAS_ERR_FN_USAGE;
			// No assertion to allow testing this error
			break;
		case TAS_PL_ERR_NOT_SUPPORTED:
			snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Function is not supported");
			mEip->tas_err = TAS_ERR_FN_NOT_SUPPORTED;
			assert(false);
			break;
		case TAS_PL1_ERR_CMD_FAILED:
			snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Function failed at TasServer");
			mEip->tas_err = TAS_ERR_GENERAL;
			assert(false);
			break;
		default:
			snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Unknown error");
			mEip->tas_err = TAS_ERR_GENERAL;
			assert(false);
		}
		return mEip->tas_err;
	}
	return mSetPktRspErrConnectionProtocol();
}

tas_return_et CTasPktHandlerBase::mSetPktRspErrConnectionProtocol()
{
	assert(false);
	snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Server connection protocol");
	mEip->tas_err = TAS_ERR_SERVER_CON;
	return mEip->tas_err;
}

tas_return_et CTasPktHandlerBase::mSetPktRspErrDeviceAccess()
{
	// No assertion to allow testing this error
	snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Device access failed");
	mEip->tas_err = TAS_ERR_DEVICE_ACCESS;
	return mEip->tas_err;
}


