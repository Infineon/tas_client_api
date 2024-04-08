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
#include "tas_client_server_con.h"
#include "tas_utils.h"

// Standard includes
#include <cassert>
#include <cstdio>
#include <memory>
#include <array>

CTasClientServerCon::CTasClientServerCon(const char* client_name, tas_error_info_st* ei, CTasPktMailboxIf* mb_if)
	: mTphsc(ei),
	  mEip(ei)
{
	snprintf(mClientName.data(), mClientName.size(), "%s", client_name);

	if (mb_if) { // Only for special test setups
		mMbIf = mb_if;
	}
	else {
		mMbSocket = new CTasPktMailboxSocket();
		mMbIf = mMbSocket;
	}

	mServerIpAddr[0] = 0;
	mServerPortNum   = 0;

	tasutil_get_user_name(mUserName.data());
	mClientPid = tasutil_get_pid();

	mServerInfo = nullptr;
	mServerChallenge = 0;

	mSessionStarted = false;
}

CTasClientServerCon::~CTasClientServerCon()
{
	delete mMbSocket;
}

tas_return_et CTasClientServerCon::server_connect(const char* ip_addr, uint16_t port_num)
{
	if (mServerIpAddr[0] != '\0') {
		assert(false);
		snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Already connected to server");
		mEip->tas_err = TAS_ERR_FN_USAGE;
		return mEip->tas_err;
	}

	mServerInfo = nullptr;
	mServerIpAddr[0] = 0;
	mServerPortNum = 0;
	mServerChallenge = 0;

	bool tasServerConnected = false;
	
#ifdef _WIN32
	if (!tasutil_check_local_tas_server_is_running()) {
		tasutil_start_local_tas_server();
		int numTries = 0;
		for (numTries = 0; numTries < 10; numTries++) {
			if (mMbSocket->server_connect(ip_addr, port_num)) {
				tasServerConnected = true;
				break;
			}
		}
		if (!tasServerConnected) {
			assert(false);
			snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Could not start TasServer on local machine");
			mEip->tas_err = TAS_ERR_SERVER_CON;
			return mEip->tas_err;
		}
		assert(numTries < 4);  // Normal case
	}
#endif

	if (!tasServerConnected && !mMbSocket->server_connect(ip_addr, port_num)) {
		return mHandleErrorServerConnect(ip_addr, port_num);		
	}
	assert(mMbIf == mMbSocket);

	const uint32_t *pktRq = mTphsc.get_pkt_rq_server_connect(mClientName.data(), mUserName.data(), mClientPid);

	std::array<uint32_t, (4 + sizeof(tas_pl1rsp_server_connect_st)) / 4> pktRsp;
	if (!mMbIf->execute(pktRq, pktRsp.data()))
		return tas_client_handle_error_server_con(mEip);

	if (mTphsc.set_pkt_rsp_server_connect(pktRsp.data(), &mServerInfo, &mServerChallenge)) {
		return mEip->tas_err;
	}

	assert(strlen(ip_addr) < IP_ADDR_NAME_BUF_SIZE);
	snprintf(mServerIpAddr.data(), IP_ADDR_NAME_BUF_SIZE, "%s", ip_addr);
	mServerPortNum = port_num;

	return tas_clear_error_info(mEip);
}

tas_return_et CTasClientServerCon::server_unlock(const void* key, uint16_t key_length)
{
	if (mRcvChlActive)
		return mHandleErrorRcvChlActive();

	const uint32_t *pktRq = mTphsc.get_pkt_rq_server_unlock(key, key_length);

	std::array<uint32_t, (4 + sizeof(tas_pl1rsp_server_unlock_st)) / 4> pktRsp;
	if (!mMbIf->execute(pktRq, pktRsp.data()))
		return tas_client_handle_error_server_con(mEip);

	if (mTphsc.set_pkt_rsp_server_unlock(pktRsp.data())) {
		return mEip->tas_err;
	}

	return tas_clear_error_info(mEip);
}

tas_return_et CTasClientServerCon::get_targets(const tas_target_info_st** target_info, uint32_t* num_target)
{
	*num_target = 0;

	if (mRcvChlActive)
		return mHandleErrorRcvChlActive();

	std::array<uint32_t, TAS_MAX_PKT_SIZE_1KB / 4> pktRsp;
	uint8_t numTarget;
	uint8_t numNow;
	uint8_t startIndex = 0;

	do {
		if (const uint32_t* pktRq = mTphsc.get_pkt_rq_get_targets(startIndex); !mMbIf->execute(pktRq, pktRsp.data()))
			return tas_client_handle_error_server_con(mEip);

		if (mTphsc.set_pkt_rsp_get_targets(pktRsp.data(), &numTarget, &numNow))
			return mEip->tas_err;

		startIndex += numNow;
	} 
	while (startIndex < numTarget);
	assert(startIndex == numTarget);

	mTphsc.get_target_info(target_info, num_target);

	return tas_clear_error_info(mEip);
}

tas_return_et CTasClientServerCon::get_target_clients(
	const char* identifier,
	const char** session_name, uint64_t* session_start_time_us,
	const tas_target_client_info_st** target_client_info, uint32_t* num_client
)
{
	*session_name = "";
	*session_start_time_us= 0;
	*target_client_info = nullptr;
	*num_client = 0;

	if (mRcvChlActive)
		return mHandleErrorRcvChlActive();
	
	std::array<uint32_t, TAS_MAX_PKT_SIZE_1KB / 4> pktRsp;
	uint8_t numClient;
	uint8_t numNow;
	uint8_t startIndex = 0;

	do {
		if (const uint32_t *pktRq = mTphsc.get_pkt_rq_get_target_clients(identifier, startIndex); !mMbIf->execute(pktRq, pktRsp.data()))
			return tas_client_handle_error_server_con(mEip);

		if (mTphsc.set_pkt_rsp_get_target_clients(pktRsp.data(), &numClient, &numNow))
			return mEip->tas_err;

		startIndex += numNow;
	} while (startIndex < numClient);
	assert(startIndex == numClient);

	mTphsc.get_target_clients_info(session_name, session_start_time_us, target_client_info, num_client);

	return tas_clear_error_info(mEip);
}

tas_return_et CTasClientServerCon::mSessionStart(tas_client_type_et client_type, const char* identifier, const char* session_name, const char* session_pw,
												 tas_chl_target_et chl_target, uint64_t chl_param)
{
	if (mSessionStarted) {
		assert(false);
		snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Session was already started");
		mEip->tas_err = TAS_ERR_FN_USAGE;
		return mEip->tas_err;
	}
	assert(!((client_type != TAS_CLIENT_TYPE_CHL) && (chl_target != TAS_CHL_TGT_UNKNOWN)));
	assert(!mRcvChlActive);

	mDeviceResetCount = ~0;

	const uint32_t *pktRq = mTphsc.get_pkt_rq_session_start(client_type, identifier, session_name, session_pw, chl_target, chl_param);

	std::array<uint32_t, (4 + sizeof(tas_pl1rsp_session_start_st)) / 4> pktRsp;
	if (!mMbIf->execute(pktRq, pktRsp.data()))
		return tas_client_handle_error_server_con(mEip);

	if (mTphsc.set_pkt_rsp_session_start(client_type, pktRsp.data())) {
		return mEip->tas_err;
	}

	mSessionStarted = true;

	return tas_clear_error_info(mEip);
}

tas_return_et CTasClientServerCon::device_unlock_get_challenge(tas_dev_unlock_cr_option_et ulcro, const void** challenge, uint16_t* challenge_length)
{
	*challenge = nullptr;
	*challenge_length = 0;

	if (mRcvChlActive)
		return mHandleErrorRcvChlActive();

	const uint32_t *pktRq = mTphsc.get_pkt_rq_device_unlock_get_challenge(ulcro);

	std::array<uint32_t, (4 + sizeof(tas_pl1rsp_device_unlock_get_challenge_st)) / 4> pktRsp;
	if (!mMbIf->execute(pktRq, pktRsp.data()))
		return tas_client_handle_error_server_con(mEip);

	if (mTphsc.set_pkt_rsp_device_unlock_get_challenge(pktRsp.data(), challenge, challenge_length)) {
		return mEip->tas_err;
	}

	return tas_clear_error_info(mEip);
}

tas_return_et CTasClientServerCon::device_unlock_set_key(tas_dev_unlock_option_et ulo, const void* key, uint16_t key_length)
{
	if (mRcvChlActive)
		return mHandleErrorRcvChlActive();

	const uint32_t *pktRq = mTphsc.get_pkt_rq_device_unlock_set_key(ulo, key, key_length);

	std::array<uint32_t, (4 + sizeof(tas_pl1rsp_set_device_key_st)) / 4> pktRsp;
	if (!mMbIf->execute(pktRq, pktRsp.data()))
		return tas_client_handle_error_server_con(mEip);

	if (mTphsc.set_pkt_rsp_device_unlock_set_key(pktRsp.data())) {
		return mEip->tas_err;
	}

	return tas_clear_error_info(mEip);
}

tas_return_et CTasClientServerCon::mDeviceConnect(tas_clnt_dco_et dco)
{
	if (mRcvChlActive)
		return mHandleErrorRcvChlActive();

	const uint32_t *pktRq = mTphsc.get_pkt_rq_device_connect(dco);

	std::array<uint32_t, (4 + sizeof(tas_pl1rsp_device_connect_st)) / 4> pktRsp;
	if (!mMbIf->execute(pktRq, pktRsp.data()))
		return tas_client_handle_error_server_con(mEip);

	uint16_t devConFeatUsed;
	if (uint32_t deviceType; mTphsc.set_pkt_rsp_device_connect(pktRsp.data(), &devConFeatUsed, &deviceType)) {
		return mEip->tas_err;
	}
	assert(devConFeatUsed == dco);
	
	return tas_clear_error_info(mEip);
}

tas_return_et CTasClientServerCon::mHandleErrorServerConnect(const char* ip_addr, uint16_t port_num)
{
	snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Server %s port %d", ip_addr, port_num);
	mEip->tas_err = TAS_ERR_SERVER_CON;
	return mEip->tas_err;
}

tas_return_et CTasClientServerCon::mHandleErrorRcvChlActive()
{
	snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Receive channel is active");
	mEip->tas_err = TAS_ERR_FN_USAGE;
	return mEip->tas_err;
}


