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
#include "tas_pkt_handler_server_con.h"
#include "tas_pkt.h"

// Standard includes
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <memory>

CTasPktHandlerServerCon::CTasPktHandlerServerCon(tas_error_info_st* ei)
	: CTasPktHandlerBase(ei)
{
	mConInfo = {};

	mSessionName[0] = '\0';

	mPl1CntOutstandingLast = 0xFFC0;  // Enforce early overrun in test setups

	// From CTasPktHandlerBase, allocated and set here in the derived class:
	mMaxRqSize = MAX_PKT_RQ_SIZE + 64;
	mRqBuf = new uint32_t[mMaxRqSize / 4];
	mRqWiMax = mMaxRqSize / 4;
	mMaxRspSize = MAX_PKT_RSP_SIZE;
}

CTasPktHandlerServerCon::~CTasPktHandlerServerCon()
{
	delete[] mRqBuf;
}

const uint32_t* CTasPktHandlerServerCon::get_pkt_rq_server_connect(const char* client_name, const char* user_name, uint32_t client_pid)
{
	const uint32_t pl1PktSize = sizeof(tas_pl1rq_server_connect_st);
	mRqBuf[0] = 4 + pl1PktSize;
	auto pkt = (tas_pl1rq_server_connect_st*)&mRqBuf[1];
	*pkt = {};
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_SERVER_CONNECT;
	snprintf(pkt->client_name, TAS_NAME_LEN32, "%s", client_name);
	snprintf(pkt->user_name, TAS_NAME_LEN16, "%s", user_name);
	pkt->client_pid = client_pid;
	return mRqBuf;
}

tas_return_et CTasPktHandlerServerCon::set_pkt_rsp_server_connect(const uint32_t* pkt_rsp, const tas_server_info_st** server_info, uint64_t* challenge)
{
	mServerInfo = {};
	mServerChallenge = 0;

	*server_info = &mServerInfo;
	*challenge = mServerChallenge;

	auto pkt = (const tas_pl1rsp_server_connect_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_server_connect_st);
	if (pkt_rsp[0] != 4 + pl1PktSize) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if ((pkt->cmd != TAS_PL1_CMD_SERVER_CONNECT) || (pkt->wl != (pl1PktSize / 4) - 1)) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if (!((pkt->err == TAS_PL_ERR_NO_ERROR) || (pkt->err == TAS_PL1_ERR_CMD_FAILED) || (pkt->err == TAS_PL1_ERR_SERVER_LOCKED))) {
		return mSetPktRspErrConnectionProtocol();
	}
	else {
		memcpy(&mServerInfo, &pkt->server_info, sizeof(tas_server_info_st));
		assert(strlen(mServerInfo.server_name) < TAS_NAME_LEN64);
		assert(strlen(mServerInfo.date) < 16);
		mServerInfo.server_name[TAS_NAME_LEN64 - 1] = 0;
		mServerInfo.date[16 - 1] = 0;
		mServerChallenge = pkt->challenge;

		if (pkt->err == TAS_PL_ERR_NO_ERROR) {
			return tas_clear_error_info(mEip);
		}
		else if (pkt->err == TAS_PL1_ERR_SERVER_LOCKED) {
			return mSetPktRspErrServerLocked();
		}
		else {
			assert(pkt->err == TAS_PL1_ERR_CMD_FAILED);
			assert(false);
			return mSetPktRspErrServerConnectionSetup();
		}
	}
}

const uint32_t* CTasPktHandlerServerCon::get_pkt_rq_server_unlock(const void* key, uint16_t key_length)
{
	assert(false);
	return nullptr;
}

tas_return_et CTasPktHandlerServerCon::set_pkt_rsp_server_unlock(const uint32_t* pkt_rsp)
{
	assert(false);
	return tas_return_et();
}

const uint32_t* CTasPktHandlerServerCon::get_pkt_rq_get_targets(uint8_t start_index)
{
	mStartIndex = start_index;

	const uint32_t pl1PktSize = sizeof(tas_pl1rq_get_targets_st);
	mRqBuf[0] = 4 + pl1PktSize;
	auto pkt = (tas_pl1rq_get_targets_st*)&mRqBuf[1];
	*pkt = {};
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_GET_TARGETS;
	pkt->start_index = start_index;
	return mRqBuf;
}

tas_return_et CTasPktHandlerServerCon::set_pkt_rsp_get_targets(const uint32_t* pkt_rsp, uint8_t* num_target, uint8_t* num_now)
{
	*num_target = *num_now = 0;

	if (mStartIndex == 0) {
		mNumTarget = 0;
	}

	assert(pkt_rsp[0] < MAX_PKT_RSP_SIZE - 64);
	auto pkt = (const tas_pl1rsp_get_targets_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_get_targets_st);
	if (pkt_rsp[0] != 4 + pl1PktSize + pkt->num_now*sizeof(tas_target_info_st)) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if ((pkt->cmd != TAS_PL1_CMD_GET_TARGETS) || (pkt->wl != (pkt_rsp[0] - 8) / 4)) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if (!((pkt->err == TAS_PL_ERR_NO_ERROR) || (pkt->err == TAS_PL1_ERR_SERVER_LOCKED))) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if ( (pkt->start_index != mStartIndex) ||
			  ((mStartIndex > 0) && (mNumTarget < NUM_TARGET_MAX) && (pkt->num_target != mNumTarget))) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if ((pkt->start_index != 0) && (((uint32_t)pkt->start_index + pkt->num_now) > mNumTarget)) {
		return mSetPktRspErrConnectionProtocol();
	}
	else {
		if (pkt->err == TAS_PL_ERR_NO_ERROR) {
			if (mStartIndex == 0) {
				mNumTarget = pkt->num_target;
				if (mNumTarget > NUM_TARGET_MAX) {
					assert(false);
					mNumTarget = NUM_TARGET_MAX;
				}
			}
			memcpy(&mTargetInfo[mStartIndex], &pkt_rsp[3], pkt->num_now * sizeof(tas_target_info_st));
			*num_target = mNumTarget;
			*num_now = pkt->num_now;
			return tas_clear_error_info(mEip);
		}
		else {
			assert(pkt->err == TAS_PL1_ERR_SERVER_LOCKED);
			assert(pkt->num_now == 0);
			return mSetPktRspErrServerLocked();
		}
	}
}

void CTasPktHandlerServerCon::get_target_info(const tas_target_info_st** target_info, uint32_t* num_target) const
{
	*target_info = (mNumTarget > 0) ? mTargetInfo.data() : nullptr;
	*num_target = mNumTarget;
}

const uint32_t* CTasPktHandlerServerCon::get_pkt_rq_get_target_clients(const char* identifier, uint8_t start_index)
{
	mStartIndex = start_index;

	const uint32_t pl1PktSize = sizeof(tas_pl1rq_get_clients_st);
	mRqBuf[0] = 4 + pl1PktSize;
	tas_pl1rq_get_clients_st* pkt = (tas_pl1rq_get_clients_st*)&mRqBuf[1];
	*pkt = {};
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_GET_CLIENTS;
	pkt->start_index = start_index;
	snprintf(pkt->identifier, TAS_NAME_LEN64, "%s", identifier);
	return mRqBuf;
}

tas_return_et CTasPktHandlerServerCon::set_pkt_rsp_get_target_clients(const uint32_t* pkt_rsp, uint8_t* num_client, uint8_t* num_now)
{
	*num_client = *num_now = 0;

	if (mStartIndex == 0) {
		mSessionName[0] = '\0';
		mSessionStartTimeUs = 0;
		mNumTargetClient = 0;
	}

	auto pkt = (const tas_pl1rsp_get_clients_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_get_clients_st);
	if (pkt_rsp[0] != 4 + pl1PktSize + pkt->num_now * sizeof(tas_target_client_info_st)) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if ((pkt->cmd != TAS_PL1_CMD_GET_CLIENTS) || (pkt->wl != (pkt_rsp[0] - 8) / 4)) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if (!((pkt->err == TAS_PL_ERR_NO_ERROR) || (pkt->err == TAS_PL_ERR_PARAM) || (pkt->err == TAS_PL1_ERR_SERVER_LOCKED))) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if ((pkt->start_index != mStartIndex) ||
		((mStartIndex > 0) && (mNumTargetClient < NUM_TARGET_CLIENT_MAX) && (pkt->num_client != mNumTargetClient))) {
		return mSetPktRspErrConnectionProtocol();
	}
	else if ((pkt->start_index != 0) && (((uint32_t)pkt->start_index + pkt->num_now) > mNumTargetClient)) {
		return mSetPktRspErrConnectionProtocol();
	}
	else {
		if (pkt->err == TAS_PL_ERR_NO_ERROR) {
			if (mStartIndex == 0) {
				snprintf(mSessionName.data(), TAS_NAME_LEN16, "%s", pkt->session_name);
				mSessionStartTimeUs = pkt->session_start_time_us;
				mNumTargetClient = pkt->num_client;
				if (mNumTargetClient > NUM_TARGET_CLIENT_MAX) {
					assert(false);
					mNumTargetClient = NUM_TARGET_CLIENT_MAX;
				}
			}
			int wiTciStart = (4 + sizeof(tas_pl1rsp_get_clients_st)) / 4;
			memcpy(mTargetClientInfo.data(), &pkt_rsp[wiTciStart], pkt->num_now * sizeof(tas_target_client_info_st));
			return tas_clear_error_info(mEip);
		}
		else if (pkt->err == TAS_PL_ERR_PARAM) {
			assert(pkt->num_now == 0);
			snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Target does not exist");
			mEip->tas_err = TAS_ERR_FN_PARAM;
			return mEip->tas_err;
		}
		else {
			assert(pkt->err == TAS_PL1_ERR_SERVER_LOCKED);
			assert(pkt->num_now == 0);
			return mSetPktRspErrServerLocked();
		}
	}
}

int ttci_compare_by_connect_time(const void* _tcci0, const void* _tcci1)
{
	auto tcci0 = (const tas_target_client_info_st*)_tcci0;
	auto tcci1 = (const tas_target_client_info_st*)_tcci1;
	return (tcci0->client_connect_time > tcci1->client_connect_time) ? 1 : -1;
}

void CTasPktHandlerServerCon::get_target_clients_info(const char** session_name, uint64_t* session_start_time_us,
	const tas_target_client_info_st** target_client_info, uint32_t* num_client)
{
	qsort(mTargetClientInfo.data(), mNumTargetClient, sizeof(tas_target_client_info_st), ttci_compare_by_connect_time);

	*session_name = mSessionName.data();
	*session_start_time_us = mSessionStartTimeUs;
	*num_client = mNumTargetClient;
	*target_client_info = mTargetClientInfo.data();
}


const uint32_t* CTasPktHandlerServerCon::get_pkt_rq_session_start(tas_client_type_et client_type, const char* identifier, 
	const char* session_name, const char* session_pw, tas_chl_target_et chl_target, uint64_t chl_param)
{
	assert(strlen(identifier) < TAS_NAME_LEN64);
	assert(strlen(session_name) < TAS_NAME_LEN16);
	assert(strlen(session_pw) < TAS_NAME_LEN16);

	const uint32_t pl1PktSize = sizeof(tas_pl1rq_session_start_st);
	mRqBuf[0] = 4 + pl1PktSize;
	auto pkt = (tas_pl1rq_session_start_st*)&mRqBuf[1];
	*pkt = {};
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_SESSION_START;
	pkt->client_type = (uint8_t)client_type;
	pkt->param8[0] = (uint8_t)chl_target;
	pkt->param64 = chl_param;
	snprintf(pkt->identifier,   TAS_NAME_LEN64, "%s", identifier);
	snprintf(pkt->session_name, TAS_NAME_LEN16, "%s", session_name);
	snprintf(pkt->session_pw,   TAS_NAME_LEN16, "%s", session_pw);

	return mRqBuf;
}

tas_return_et CTasPktHandlerServerCon::set_pkt_rsp_session_start(tas_client_type_et client_type, const uint32_t* pkt_rsp)
{
	return set_pkt_rsp_ping(TAS_PL1_CMD_SESSION_START, client_type, pkt_rsp);
}

const uint32_t* CTasPktHandlerServerCon::get_pkt_rq_device_unlock_get_challenge(tas_dev_unlock_cr_option_et ulcro)
{
	return nullptr;
}

tas_return_et CTasPktHandlerServerCon::set_pkt_rsp_device_unlock_get_challenge(const uint32_t* pkt_rsp, const void** challenge, uint16_t* challenge_length)
{
	return tas_return_et();
}

const uint32_t* CTasPktHandlerServerCon::get_pkt_rq_device_unlock_set_key(tas_dev_unlock_option_et ulo, const void* key, uint16_t key_length)
{
	assert(key_length <= TAS_UNLOCK_MAX_KEY_LEN);
	assert(key_length % 4 == 0);
	const uint32_t pl1PktSize = sizeof(tas_pl1rq_set_device_key_st);
	mRqBuf[0] = 4 + pl1PktSize + (((key_length + 3) / 4) * 4);
	auto pkt = (tas_pl1rq_set_device_key_st*)&mRqBuf[1];
	*pkt = {};
	pkt->wl = (uint8_t)((pl1PktSize / 4) - 1 + ((key_length + 3) / 4));
	pkt->cmd = TAS_PL1_CMD_SET_DEVICE_KEY;
	pkt->ulo = ulo;
	pkt->key_length = (key_length > TAS_UNLOCK_MAX_KEY_LEN) ? TAS_UNLOCK_MAX_KEY_LEN : key_length;
	if (key_length > 0) {
		void* pktKey = (char*)&mRqBuf[1] + pl1PktSize;
		memcpy(pktKey, key, pkt->key_length);
	}
	return mRqBuf;
}

tas_return_et CTasPktHandlerServerCon::set_pkt_rsp_device_unlock_set_key(const uint32_t* pkt_rsp)
{
	auto pkt = (const tas_pl1rsp_set_device_key_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_set_device_key_st);
	if (pkt_rsp[0] != 4 + pl1PktSize) {
		return mSetPktRspErrConnectionProtocol();
	}
	if ((pkt->cmd != TAS_PL1_CMD_SET_DEVICE_KEY) || (pkt->wl != (pl1PktSize / 4) - 1)) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->err != TAS_PL_ERR_NO_ERROR) {
		return mSetPktRspErrConnectionProtocol();
	}

	return tas_clear_error_info(mEip);
}


const uint32_t* CTasPktHandlerServerCon::get_pkt_rq_device_connect(tas_clnt_dco_et option)
{
	const uint32_t pl1PktSize = sizeof(tas_pl1rq_device_connect_st);
	mRqBuf[0] = 4 + pl1PktSize;
	auto pkt = (tas_pl1rq_device_connect_st*)&mRqBuf[1];
	*pkt = {};
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_DEVICE_CONNECT;
	pkt->option = option;
	mDeviceConnectOption = pkt->option;
	return mRqBuf;
}

tas_return_et CTasPktHandlerServerCon::set_pkt_rsp_device_connect(const uint32_t* pkt_rsp, uint16_t* dev_con_feat_used, uint32_t* device_type)
{
	auto pkt = (const tas_pl1rsp_device_connect_st*)&pkt_rsp[1];

	*dev_con_feat_used = 0;
	*device_type = 0;

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_device_connect_st);
	if (pkt_rsp[0] != 4 + pl1PktSize) {
		return mHandlePktError(pkt_rsp, TAS_PL1_CMD_DEVICE_CONNECT);
	}
	assert(pkt->reserved == 0);
	if ((pkt->cmd != TAS_PL1_CMD_DEVICE_CONNECT) || (pkt->wl != (pl1PktSize / 4) - 1)) {
		return mSetPktRspErrConnectionProtocol();
	}

	*dev_con_feat_used = pkt->feat_used;
	*device_type = pkt->device_type;  // 0 if no device connected

	if (pkt->err == TAS_PL_ERR_NO_ERROR) {
		assert(*dev_con_feat_used == mDeviceConnectOption);
		return tas_clear_error_info(mEip);
	}
	else if (pkt->err == TAS_PL1_ERR_DEV_ACCESS) {
		assert(*dev_con_feat_used <= mDeviceConnectOption);
		assert(*device_type == 0);
		return mSetPktRspErrDeviceAccess();
	}
	else if (pkt->err == TAS_PL1_ERR_DEV_LOCKED) {
		assert(*dev_con_feat_used <= mDeviceConnectOption);
		assert(*device_type != 0);
		return mSetPktRspErrDeviceLocked();
	} 
	else if (pkt->err == TAS_PL1_ERR_CMD_FAILED) {
		if (*device_type != 0) {
			assert(mDeviceConnectOption != 0);
			assert(*dev_con_feat_used < mDeviceConnectOption);  // Not all were successfully used
			if ((mDeviceConnectOption & TAS_DEV_CON_FEAT_RESET) && !(*dev_con_feat_used & TAS_DEV_CON_FEAT_RESET)) {
				snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Reset not supported for this device connection");
			}
			else if ((mDeviceConnectOption & TAS_DEV_CON_FEAT_RESET_AND_HALT) && !(*dev_con_feat_used & TAS_DEV_CON_FEAT_RESET_AND_HALT)) {
				snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Halt after reset not supported for this device connection");
			}
			mEip->tas_err = TAS_ERR_FN_PARAM;
		}
		else {  // *device_type == 0
			snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: No device connected");
			mEip->tas_err = TAS_ERR_DEVICE_ACCESS;
		}
		return mEip->tas_err;
	}
	else {
		return mSetPktRspErrConnectionProtocol();
	}
}

const uint32_t* CTasPktHandlerServerCon::get_pkt_rq_device_reset_count()
{
	const uint32_t pl1PktSize = sizeof(tas_pl1rq_device_reset_count_st);
	mRqBuf[0] = 4 + pl1PktSize;
	auto pkt = (tas_pl1rq_device_reset_count_st*)&mRqBuf[1];
	*pkt = {};
	pkt->wl = (pl1PktSize / 4) - 1;
	pkt->cmd = TAS_PL1_CMD_DEVICE_RESET_COUNT;
	return mRqBuf;
}

tas_return_et CTasPktHandlerServerCon::set_pkt_rsp_device_reset_count(const uint32_t* pkt_rsp, tas_reset_count_st* reset_count)
{
	auto pkt = (const tas_pl1rsp_device_reset_count_st*)&pkt_rsp[1];

	const uint32_t pl1PktSize = sizeof(tas_pl1rsp_device_reset_count_st);
	if (pkt_rsp[0] != 4 + pl1PktSize) {
		return mSetPktRspErrConnectionProtocol();
	}
	if ((pkt->cmd != TAS_PL1_CMD_DEVICE_RESET_COUNT) || (pkt->wl != (pl1PktSize / 4) - 1)) {
		return mSetPktRspErrConnectionProtocol();
	}
	if (pkt->err != TAS_PL_ERR_NO_ERROR) {
		return mSetPktRspErrConnectionProtocol();
	}
	memcpy(reset_count, &pkt->reset_count, sizeof(*reset_count));
	return tas_clear_error_info(mEip);
}

tas_return_et CTasPktHandlerServerCon::mSetPktRspErrServerConnectionSetup()
{
	assert(false);
	snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Server connection setup failed");
	mEip->tas_err = TAS_ERR_SERVER_CON;
	return mEip->tas_err;
}

tas_return_et CTasPktHandlerServerCon::mSetPktRspErrServerLocked()
{
	snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Server is locked");
	mEip->tas_err = TAS_ERR_SERVER_LOCKED;
	return mEip->tas_err;
}

tas_return_et CTasPktHandlerServerCon::mSetPktRspErrDeviceAccess()
{
	snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Device access (power-down, reset active, etc.)");
	mEip->tas_err = TAS_ERR_DEVICE_ACCESS;
	return mEip->tas_err;
}

tas_return_et CTasPktHandlerServerCon::mSetPktRspErrDeviceLocked()
{
	snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Device is locked");
	mEip->tas_err = TAS_ERR_DEVICE_LOCKED;
	return mEip->tas_err;
}



