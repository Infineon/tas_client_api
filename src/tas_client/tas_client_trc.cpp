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
#include "tas_client_trc.h"

// Standard includes
#include <cassert>

CTasClientTrc::CTasClientTrc(const char* client_name)
	:  mClientChl(client_name)
{
	
}

tas_return_et CTasClientTrc::subscribe(tas_chso_et chso, uint8_t prio, tas_trc_type_et trace_type, uint8_t stream)
{
	tas_return_et ret;
	ret = mClientChl.subscribe(TAS_CHL_NUM_TRC, TAS_CHT_BIDI, chso, &prio);

	if (ret == TAS_ERR_NONE) {
		mTrcSubscribe = { (uint8_t)trace_type, stream, 0, 0 };
		ret = mClientChl.send_msg(&mTrcSubscribe, sizeof(mTrcSubscribe), TAS_TRC_CMD_SUBSCRIBE);
	}
	else {
		mTrcSubscribe = {};
	}
	return ret;
}


tas_return_et CTasClientTrc::get_trace(uint32_t timeout_ms, const uint32_t** trace_data, uint32_t* length, tas_trc_state_st* trc_state)
{
	const uint32_t* msg;
	uint32_t msgLength, init;
	tas_return_et ret;
	ret = mClientChl.rcv_msg(timeout_ms, &msg, &msgLength, &init);

	if (ret == TAS_ERR_NONE) {
		tas_trc_state_st* trcState = (tas_trc_state_st*)&msg[0];
		assert(trcState->stream == mTrcSubscribe.stream);
		assert(trcState->stream_state > TAS_TRCS_UNKNOWN);

		assert(sizeof(tas_trc_state_st) == 4);
		assert(msgLength >= (4 + 1024));
		assert((msgLength - 4) % 1024 == 0);

		*trace_data = &msg[4/4];
		*length = msgLength - 4;
		memcpy(trc_state, &msg[0], 4);
		memcpy(&mTrcStateLast, trc_state, 4);
	}
	else {
		assert(ret == TAS_ERR_CHL_RCV);
		*trace_data = nullptr;
		*length = 0;
		*trc_state = {};
		mTrcStateLast = {};
	}

	return ret;
}

