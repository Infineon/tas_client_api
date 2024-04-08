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
#include "tas_utils.h"
#include "tas_device_family.h"
#include "tas_am15_am14.h"

// Standard includes
#include <cassert>
#include <cstring>
#include <cinttypes>
#include <array>

void tasutil_hash32_to_str(uint32_t hash32, char* hash32_str)
{
	assert(TASUTIL_HASH32_STRBUF_SIZE >= 7);
	const char* asciiMap = "ABCDEFGHJKLMNPQRTUVWXYZ012346789";  // 32 selected
	assert(strlen(asciiMap) == 32);
	const uint32_t asciiMapSize = 32;
	uint32_t ami;
	uint32_t value = hash32;
	hash32_str[6] = 0;
	for (uint32_t i = 5; i > 0; i--) {
		ami = value % asciiMapSize;
		hash32_str[i] = asciiMap[ami];
		value = value / asciiMapSize;
	}
	ami = value % (asciiMapSize - 10);  // Without numbers
	hash32_str[0] = asciiMap[ami];
}

void tasutil_get_dev_con_feat_str(uint16_t dev_con_feat, char* st)
{
	size_t bSize = TASUTIL_DEV_CON_FEAT_STR_BUF_SIZE;
	if (dev_con_feat == TAS_DEV_CON_FEAT_NONE) {
		snprintf(st, bSize, "None");
		return;
	}

	int s = 0;
	switch (dev_con_feat & TAS_DEV_CON_FEAT_RESET_AND_HALT) {
	case TAS_DEV_CON_FEAT_NONE:           break;
	case TAS_DEV_CON_FEAT_RESET:          s += snprintf(st + s, bSize - s, "reset, "); break;
	case TAS_DEV_CON_FEAT_RESET_AND_HALT: s += snprintf(st + s, bSize - s, "reset|HAR, "); break;
	default: assert(false);               s += snprintf(st + s, bSize - s, "reset?, "); break;
	}
	switch (dev_con_feat & TAS_DEV_CON_FEAT_UNLOCK_CNR) {
	case TAS_DEV_CON_FEAT_NONE:       break;
	case TAS_DEV_CON_FEAT_UNLOCK:     s += snprintf(st + s, bSize - s, "unlocking, "); break;
	case TAS_DEV_CON_FEAT_UNLOCK_CNR: s += snprintf(st + s, bSize - s, "unlocking|CNR, "); break;
	default: assert(false);           s += snprintf(st + s, bSize - s, "unlock?, "); break;
	}
	assert(s < (TASUTIL_DEV_CON_FEAT_STR_BUF_SIZE - 32));
	assert(s > 2);
	st[s - 2] = 0;  // Remove last ", "
}

int tasutil_get_con_info_str(const tas_con_info_st* ci, char* str_buf, bool is_client_chl)
{
	const char* devConPhys = tasutil_get_dev_con_phys_str(ci->dev_con_phys);

	const char* deviceName = tas_get_device_name_str(ci->device_type);

	std::array<char, TASUTIL_DEV_CON_FEAT_STR_BUF_SIZE> devConFeat;
	tasutil_get_dev_con_feat_str(ci->dev_con_feat, devConFeat.data());

	std::array<char, 64> ipv4Addr;
	if (ci->ipv4_addr != 0) {
		auto ipa = (const uint8_t*)&ci->ipv4_addr;
		snprintf(ipv4Addr.data(), ipv4Addr.size(), "%u.%u.%u.%u (direct TAS Ethernet access supported)", 
			     ipa[3], ipa[2], ipa[1], ipa[0]);
	}
	else {
		snprintf(ipv4Addr.data(), ipv4Addr.size(), "Information not accessible or not supported");
	}

	std::array<char, 256> chlSupport;
	const char* chlHeadline;
	if (is_client_chl) {
		if ((ci->msg_length_c2d + ci->msg_length_d2c) > 0)
			chlHeadline = "TAS channels supported";
		else
			chlHeadline = "No TAS channels supported (or device in HAR state?)";
		snprintf(chlSupport.data(), chlSupport.size(), 
			"%s\nmax message length c2d %u, d2c %u\n"
			"max number in on-chip buffer c2d %u, d2c %u",
			chlHeadline, ci->msg_length_c2d, ci->msg_length_d2c, 
			ci->msg_num_c2d, ci->msg_num_d2c);
	}
	else {
		snprintf(chlSupport.data(), chlSupport.size(), "TAS channel support was not checked by this client\n\n\n");
	}

	int strLen = snprintf(str_buf, TASUTIL_CON_INFO_STR_BUF_SIZE,
		"device_type    0x%8.8X (%s)\n"
		"device_id      0x%8.8X 0x%8.8X 0x%8.8X 0x%8.8X\n"
		"device_id_hash 0x%8.8X (%s)\n"
		"dev_con_phys   %s\n"
		"identifier     %s\n"
		"ipv4_addr      %s\n" 
		"max_pkt_size   rq %u, rsp %u\n"
		"max_num_rw     %u\n"
		"rw_mode_mask   0x%4.4X\n"
		"addr_map_mask  0x%4.4X\n"
		"dev_con_feat   %s\n"
		"\n%s\n",

		ci->device_type, deviceName,
		ci->device_id[0], ci->device_id[1], ci->device_id[2], ci->device_id[3],
		ci->device_id_hash, ci->device_id_hash_str,
		devConPhys,
		ci->identifier,
		ipv4Addr.data(),
		ci->max_pl2rq_pkt_size, ci->max_pl2rsp_pkt_size,
		ci->pl0_max_num_rw, 
		ci->pl0_rw_mode_mask, 
		ci->pl0_addr_map_mask,
		devConFeat.data(),
		chlSupport.data()
		);
	assert(strLen < TASUTIL_CON_INFO_STR_BUF_SIZE - 64);  // With some margin

	return strLen;
}

int tasutil_get_server_info_str(const char* ip_addr, uint16_t port_num, const tas_server_info_st* si, char* str_buf)
{
	time_t startTime = si->start_time_us / 1000000;  // Convert to seconds
	std::array<char, TASUTIL_TIME_STR_BUF_SIZE> timeStr;
	tasutil_get_time_str(startTime, timeStr.data());

	std::array<char, 128> protocVer;
	assert(si->supp_protoc_ver != 0);
	assert(!(si->supp_protoc_ver & 1));
	int32_t s = 0;
	for (int32_t i = 1; i < 32; i++) {
		if (si->supp_protoc_ver & (1 << i)) {
			s += snprintf(protocVer.data() + s, 32 - s, "%d, ", i);
		}
	}
	protocVer[s - 2] = 0;  // Remove last ", "

	// Currently only DMM channels and no trace supported
	assert(si->supp_chl_target == 2);

	const char* suppTrcTypeStr = "Supports on-chip trace but no continuous trace";
	if (si->supp_trc_type != 0) {
		assert(si->supp_trc_type == 1);
		suppTrcTypeStr = "Supports on-chip trace and continuous trace";
	}

	int strLen = snprintf(str_buf, TASUTIL_SERVER_INFO_STR_BUF_SIZE,
		"%s V%u.%u (%s)\n"
		"Started at %s\n"
		"ip_addr    %s\n"
		"port_num   %u\n"
		"Supports TAS protocol version %s\n"
		"Supports device memory mapped (DMM) channels\n"
		"%s\n",
		si->server_name, si->v_major, si->v_minor, si->date,
		timeStr.data(),
		ip_addr,
		port_num,
		protocVer.data(),
		suppTrcTypeStr
	);
	assert(strLen < TASUTIL_CON_INFO_STR_BUF_SIZE - 64);  // With some margin

	return strLen;
}

int tasutil_get_target_clients_str(
	const char* session_name, uint64_t session_start_time_us,
	const tas_target_client_info_st* tci, uint32_t num_client, 
	char* str_buf, int str_buf_size)
{
	if (num_client == 0) {
		assert(false);  // Situation should be handled properly on higher level
		str_buf[0] = 0;
		return 0;
	}

	int s = 0;
	if (session_name[0] == 0)
		s = snprintf(str_buf, str_buf_size, "Session was started without a session name\n");
	else
		s = snprintf(str_buf, str_buf_size, "Session %.32s was started\n", session_name);
	double sessionStartTime = ((double)session_start_time_us) / 1000000;
	s += snprintf(str_buf + s, str_buf_size - s, "%.3f s after server start\n", sessionStartTime);

	for (int c = 0; c < (int)num_client; c++) {
		const char* clientTypeStr = tasutil_get_client_type_str(tci[c].client_type);
		assert(tci[c].client_connect_time >= session_start_time_us);
		double clientRelativeStartTime = ((double)(tci[c].client_connect_time - session_start_time_us)) / 1000000;
		s += snprintf(str_buf + s, str_buf_size - s,
			"%s (%s, PID %u, user %s)\n"
			"    Client connected %.3f s after session start\n"
			"    Client sent %" PRIu64 " bytes and received %" PRIu64 " bytes\n",
			tci[c].client_name, clientTypeStr, tci[c].client_pid, tci[c].user_name,
			clientRelativeStartTime,
			tci[c].num_byte_c2s, tci[c].num_byte_s2c);
	}
	return s;
}

CTasutilDeviceFrequ::CTasutilDeviceFrequ()
{
	mClearRefValue();
	mClearValue();
}

void CTasutilDeviceFrequ::add_timer_sample(uint32_t timer_value, uint64_t timestamp_before_us, uint64_t timestamp_after_us)
{
	assert(timestamp_before_us > 0);
	assert(timestamp_after_us > timestamp_before_us);

	if (mRefTimestampAfterUs == 0) {
		mRefTimerValue = timer_value;
		mRefTimestampBeforeUs = timestamp_before_us;
		mRefTimestampAfterUs = timestamp_after_us;
		mClearValue();
	}
	else {
		assert(timestamp_before_us > mTimestampAfterUs);
		assert(timestamp_after_us > timestamp_before_us);
		const uint64_t maxSampleDistanceTimeUs = 4000000;  // 4s -> Only one 32 bit timer overrun can happen even at 1GHz
		if (timestamp_after_us > mRefTimestampBeforeUs + maxSampleDistanceTimeUs) {
			mRefTimerValue = timer_value;
			mRefTimestampBeforeUs = timestamp_before_us;
			mRefTimestampAfterUs = timestamp_after_us;
			mClearValue();
		}
		else {
			mTimerValue = timer_value;
			mTimestampBeforeUs = timestamp_before_us;
			mTimestampAfterUs = timestamp_after_us;
		}
	}
}

void CTasutilDeviceFrequ::get_timer_frequency(double* frequency_hz, double* max_deviation_hz) const
{
	if (mTimestampAfterUs == 0) {
		*frequency_hz = 0.0;
		*max_deviation_hz = 1.0E9;  // 1 GHz
	}
	else {
		assert(mRefTimestampBeforeUs > 0);
		uint32_t timerDiff = mTimerValue - mRefTimerValue;  // Result also correct for 32 bit overrun!
		uint64_t timeDiffMinUs = mTimestampBeforeUs - mRefTimestampAfterUs;
		uint64_t timeDiffMaxUs = mTimestampAfterUs - mRefTimestampBeforeUs;
		double frequMaxMHz = ((double)timerDiff) / ((double)timeDiffMinUs);
		double frequMinMHz = ((double)timerDiff) / ((double)timeDiffMaxUs);
		auto frequMaxMHzRoundedDown = (int)(frequMaxMHz * 1.001);   // Compensate if crystal frequency is slightly below
		auto frequMinMHzRoundedDown = (int)frequMinMHz;
		if (frequMaxMHzRoundedDown == frequMinMHzRoundedDown + 1) {
			*frequency_hz = frequMaxMHzRoundedDown * 1.0E6;
			double frequDiffUpper = (frequMaxMHz * 1.0E6) - *frequency_hz;
			double frequDiffLower = *frequency_hz - (frequMinMHz * 1.0E6);
			*max_deviation_hz = (frequDiffUpper > frequDiffLower) ? frequDiffUpper : frequDiffLower;
		}
		else {
			*frequency_hz     = (frequMaxMHz + frequMinMHz) * 1.0E6 / 2;
			*max_deviation_hz = (frequMaxMHz - frequMinMHz) * 1.0E6 / 2;
		}
	}
}

void CTasutilDeviceFrequ::mClearRefValue()
{
	mRefTimerValue = 0;
	mRefTimestampBeforeUs = 0;
	mRefTimestampAfterUs = 0;
}

void CTasutilDeviceFrequ::mClearValue()
{
	mTimerValue = 0;
	mTimestampBeforeUs = 0;
	mTimestampAfterUs = 0;
}
