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
#include "tas_client_rw_base.h"
#include "tas_utils.h"

// Standard includes
#include <cassert>
#include <cstdio>
#include <cinttypes>
#include <memory>
#include <array>

CTasClientRwBase::CTasClientRwBase(uint32_t max_rsp_size)
	: mRspBuf(max_rsp_size / 4) {}

CTasClientRwBase::~CTasClientRwBase()
{
	delete mTphRw;
}

CTasClientRwBase::CTasClientRwBase(CTasPktMailboxIf* mb_if, uint32_t max_rq_size, uint32_t max_rsp_size, uint32_t max_num_rw)
	: mRspBuf(max_rsp_size / 4),
	  mMbIfRw(mb_if)
{
	mMbIfRw->config(mTimeoutMs, max_rsp_size);
	mTphRw = new CTasPktHandlerRw(&mEi, max_rq_size, max_rsp_size, max_num_rw);
}

tas_return_et CTasClientRwBase::target_ping(tas_con_info_st* con_info)
{
	*con_info = {};

	if (!mTphRw) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Session not yet started");
		return TAS_ERR_FN_USAGE;
	}

	if (const uint32_t* pktRq = mTphRw->get_pkt_rq_ping(TAS_PL1_CMD_PING); !mMbIfRw->execute(pktRq, mRspBuf.data()))
		return tas_client_handle_error_server_con(&mEi);

	if (mTphRw->set_pkt_rsp_ping(TAS_PL1_CMD_PING, TAS_CLIENT_TYPE_RW, mRspBuf.data())) {
		return mEi.tas_err;
	}

	memcpy(con_info, mTphRw->get_con_info(), sizeof(tas_con_info_st));

	return tas_clear_error_info(&mEi);
}

tas_return_et CTasClientRwBase::read8(uint64_t addr, uint8_t* value, uint8_t addr_map)
{
	tas_rw_trans_st trans = { addr, 1, TAS_RW_TT_RD, addr_map, TAS_RW_TT_RD, value };
	*value = 0;  // Default in case of errors
	return mExecuteSingleTrans(&trans);
}

tas_return_et CTasClientRwBase::write8(uint64_t addr, uint8_t value, uint8_t addr_map)
{
	tas_rw_trans_st trans = { addr, 1, 0, addr_map, TAS_RW_TT_WR, &value };
	return mExecuteSingleTrans(&trans);
}

tas_return_et CTasClientRwBase::read16(uint64_t addr, uint16_t* value, uint8_t addr_map)
{
	tas_rw_trans_st trans = { addr, 2, 0, addr_map, TAS_RW_TT_RD, value };
	*value = 0;  // Default in case of errors
	return mExecuteSingleTrans(&trans);
}

tas_return_et CTasClientRwBase::write16(uint64_t addr, uint16_t value, uint8_t addr_map)
{
	tas_rw_trans_st trans = { addr, 2, 0, addr_map, TAS_RW_TT_WR, &value };
	return mExecuteSingleTrans(&trans);
}

tas_return_et CTasClientRwBase::read32(uint64_t addr, uint32_t* value, uint8_t addr_map)
{
	tas_rw_trans_st trans = { addr, 4, 0, addr_map, TAS_RW_TT_RD, value };
	*value = 0;  // Default in case of errors
	return mExecuteSingleTrans(&trans);
}

tas_return_et CTasClientRwBase::write32(uint64_t addr, uint32_t value, uint8_t addr_map)
{
	tas_rw_trans_st trans = { addr, 4, 0, addr_map, TAS_RW_TT_WR, &value };
	return mExecuteSingleTrans(&trans);
}

tas_return_et CTasClientRwBase::read64(uint64_t addr, uint64_t* value, uint8_t addr_map)
{
	tas_rw_trans_st trans = { addr, 8, 0, addr_map, TAS_RW_TT_RD, value };
	*value = 0;  // Default in case of errors
	return mExecuteSingleTrans(&trans);
}

tas_return_et CTasClientRwBase::write64(uint64_t addr, uint64_t value, uint8_t addr_map)
{
	tas_rw_trans_st trans = { addr, 8, 0, addr_map, TAS_RW_TT_WR, &value };
	return mExecuteSingleTrans(&trans);
}

tas_return_et CTasClientRwBase::read(uint64_t addr, void* data, uint32_t num_bytes, uint32_t* num_bytes_ok, uint8_t addr_map)
{
	tas_rw_trans_st trans = { addr, num_bytes, 0, addr_map, TAS_RW_TT_RD, data };
	return mExecuteSingleTrans(&trans, num_bytes_ok);
}

tas_return_et CTasClientRwBase::write(uint64_t addr, const void* data, uint32_t num_bytes, uint32_t* num_bytes_ok, uint8_t addr_map)
{
	tas_rw_trans_st trans = { addr, num_bytes, 0, addr_map, TAS_RW_TT_WR, data };
	return mExecuteSingleTrans(&trans, num_bytes_ok);
}

tas_return_et CTasClientRwBase::fill32(uint64_t addr, uint32_t value, uint32_t num_bytes, uint8_t addr_map)
{
	if (addr % 4) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: addr has to be 32 bit aligned for fill32()");
		return TAS_ERR_FN_PARAM;
	}
	if ((num_bytes == 0) || (num_bytes % 4)) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: num_bytes has to be a multiple of 4 for fill32()");
		return TAS_ERR_FN_PARAM;
	}

	uint64_t value64;	
	std::array<tas_rw_trans_st, 3> trans;
	uint32_t t = 0;
	uint64_t addrTr = addr;
	uint32_t numBytesTr = num_bytes;
	if (addr % 8) {
		trans[t] = tas_rw_trans_st{ addrTr, 4, 0, addr_map, TAS_RW_TT_WR, &value };
		addrTr += 4;
		numBytesTr -= 4;
		t++;
	}
	if (numBytesTr >= 8) {
		uint32_t numBytesTr64 = (numBytesTr / 8) * 8;
		value64 = (((uint64_t)value) << 32) | value;
		trans[t] = tas_rw_trans_st{ addrTr, numBytesTr64, 0, addr_map, TAS_RW_TT_FILL, &value64 };
		addrTr += numBytesTr64;
		numBytesTr -= numBytesTr64;
		t++;
	}
	if (numBytesTr > 0) {
		assert(numBytesTr == 4);
		trans[t] = tas_rw_trans_st{ addrTr, 4, 0, addr_map, TAS_RW_TT_WR, &value };
		t++;
	}
	assert((t >= 1) && (t <= 3));

	tas_return_et ret = execute_trans(trans.data(), t);

	return ret;
}

tas_return_et CTasClientRwBase::fill64(uint64_t addr, uint64_t value, uint32_t num_bytes, uint8_t addr_map)
{
	if (addr % 8) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: addr has to be 64 bit aligned for fill64()");
		return TAS_ERR_FN_PARAM;
	}
	if ((num_bytes == 0) || (num_bytes % 8)) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: num_bytes has to be a multiple of 8 for fill64()");
		return TAS_ERR_FN_PARAM;
	}

	tas_rw_trans_st trans = { addr, num_bytes, 0, addr_map, TAS_RW_TT_FILL, &value };
	uint32_t numBytesOk;
	tas_return_et ret = mExecuteSingleTrans(&trans, &numBytesOk);

	return ret;
}

tas_return_et CTasClientRwBase::mExecuteSingleTrans(const tas_rw_trans_st* trans, uint32_t* num_bytes_ok)
{
	if (tas_return_et ret = execute_trans(trans, 1); ret != TAS_ERR_NONE)
	{
		if (num_bytes_ok)
			*num_bytes_ok = 0;
		return ret;
	}

	const tas_rw_trans_rsp_st* transRsp;
	uint32_t numTrans = rw_get_trans_rsp(&transRsp);
	assert(numTrans == 1);
	_unused(numTrans);

	if (num_bytes_ok)
		*num_bytes_ok = transRsp[0].num_bytes_ok;

	return tas_clear_error_info(&mEi);
}

tas_return_et CTasClientRwBase::execute_trans(const tas_rw_trans_st* trans, uint32_t num_trans)
{
	if (!mTphRw) {
		snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Session not yet started");
		return TAS_ERR_FN_USAGE;
	}

	if (!mTphRw->rw_set_trans(trans, num_trans)) 
	{
		std::array<char, TAS_INFO_STR_LEN/2> transStr;
		const char* typeStr = (trans->type == TAS_RW_TT_RD) ? "RD" : "WR";
		snprintf(transStr.data(), TAS_INFO_STR_LEN/2, "%s addr=0x%" PRIX64 ", num_bytes=%" PRIu32 ", acc_mode=0x%4.4" PRIX16 ", addr_map=%" PRIu8,
			typeStr, trans->addr, trans->num_bytes, trans->acc_mode, trans->addr_map);
		if (num_trans == 1)
			snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Failed to add %s", transStr.data());
		else
			snprintf(mEi.info, TAS_INFO_STR_LEN, "ERROR: Failed to add %" PRIu32 " trans (first %s)", num_trans, transStr.data());
		return TAS_ERR_FN_PARAM;
	}

	const uint32_t* rq;
	uint32_t rqNumBytes;
	uint32_t rspNumBytes;
	uint32_t numPl2Pkt;
	mTphRw->rw_get_rq(&rq, &rqNumBytes, &rspNumBytes, &numPl2Pkt);

	uint32_t rspNumBytesReceived = 0;
	if (!mMbIfRw->execute(rq, mRspBuf.data(), numPl2Pkt, &rspNumBytesReceived))
		return tas_client_handle_error_server_con(&mEi);
	assert(rspNumBytesReceived > 0);
	assert(rspNumBytesReceived % 4 == 0);
	assert(rspNumBytesReceived <= rspNumBytes);

	if (mTphRw->rw_set_rsp(mRspBuf.data(), rspNumBytesReceived) != TAS_ERR_NONE)
		return mEi.tas_err;

	return tas_clear_error_info(&mEi);
}

uint32_t CTasClientRwBase::rw_get_trans_rsp(const tas_rw_trans_rsp_st** trans_rsp)
{
	if (!mTphRw) {
		assert(false);  // Wrong usage
		return 0;
	}

	return mTphRw->rw_get_trans_rsp(trans_rsp);
}

void CTasClientRwBase::rw_set_timeout(uint32_t timeout_ms)
{
	assert(timeout_ms >= 100);     // 100ms
	assert(timeout_ms <= 100000);  // 100s
	mTimeoutMs = timeout_ms;
}

uint32_t CTasClientRwBase::rw_get_timeout() 
{
	return mTimeoutMs;
}

