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
#include "tas_utils_jtag.h"
#include "tas_device_family.h"
#include "tas_am15_am14.h"

// Standard includes
#include <cassert>

tas_return_et tasutil_jtag_scan(CTasClientRw* tcrw, const tasutil_jtag_scan_st* scan, uint8_t num_scan)
{
	tas_return_et ret = TAS_ERR_NONE;

	uint32_t numTransMax = 16 + 6 * num_scan;  // Very generous
	auto rwTrans = new tas_rw_trans_st[numTransMax];
	uint64_t* zeroDataBlock = nullptr;

	uint32_t resDat;
	uint32_t t = 1;
	rwTrans[0] = { TAS_AM15_RW_JTAG_SCAN_MODE_ENABLE, 4, 0, TAS_AM15, TAS_RW_TT_WR, &resDat };
	for (uint32_t s = 0; s < num_scan; s++) {
		assert((scan[s].tjs == TJS_NONE) || (scan[s].tjs == TJS_TAP_RESET));
		if (scan[s].tjs == TJS_TAP_RESET) {
			rwTrans[t] = { TAS_AM15_W_JTAG_RESET, 4, 0, TAS_AM15, TAS_RW_TT_WR, &resDat };
			t++;
		}
		if (scan[s].ir.width > 0) {
			rwTrans[t] = { TAS_AM15_W_JTAG_SET_IR, 8, 0, TAS_AM15, TAS_RW_TT_WR, &scan[s].ir };
			t++;
		}
		rwTrans[t] = { TAS_AM15_W_JTAG_CAPTURE, 4, 0, TAS_AM15, TAS_RW_TT_WR, &scan[s].num_bits };
		t++;

		const uint32_t maxBitsTrans = TAS_AM15_JTAG_MAX_NUM_SCAN_BITS_PER_TRANSACTION;
		uint64_t* dataOut = scan[s].data_out;
		const uint64_t* dataIn = scan[s].data_in;
		if (scan[s].data_in == nullptr) {
			if (!zeroDataBlock)
				zeroDataBlock = new uint64_t[maxBitsTrans / 64];
			dataIn = zeroDataBlock;
		}

		uint32_t bitsNow;
		uint32_t bytesNow;
		uint32_t bitsRemaining = scan[s].num_bits;
		do {
			bitsNow = (bitsRemaining > maxBitsTrans) ? maxBitsTrans : bitsRemaining;
			bytesNow = ((bitsNow + 63) / 64) * 8;
			rwTrans[t] = { TAS_AM15_RW_JTAG_DATA_SHIFT, bytesNow, 0, TAS_AM15, TAS_RW_TT_WR, dataIn };
			t++;
			if (dataOut != nullptr) {
				rwTrans[t] = { TAS_AM15_RW_JTAG_DATA_SHIFT, bytesNow, 0, TAS_AM15, TAS_RW_TT_RD, dataOut };
				t++;
			}
			if (bitsNow == maxBitsTrans) {
				if (dataIn != zeroDataBlock)
					dataIn += (maxBitsTrans / 64);
				if (dataOut != nullptr)
					dataOut += (maxBitsTrans / 64);
			}
			bitsRemaining -= bitsNow;
		} while ((bitsRemaining > 0) && (t < numTransMax - 6));

		if (bitsRemaining > 0) {
			assert(false);  // May only happen for extremely long scan chains (higher level SW bug)
			ret = TAS_ERR_FN_PARAM;
			break;  // for (s)
		}
	}
	rwTrans[t] = { TAS_AM15_W_JTAG_SCAN_MODE_DISABLE, 4, 0, TAS_AM15, TAS_RW_TT_WR, &resDat };
	t++;
	
	if (ret == TAS_ERR_NONE) {
		ret = tcrw->execute_trans(rwTrans, t);
	}

	delete[] rwTrans;
	delete[] zeroDataBlock;

	return ret;
}

