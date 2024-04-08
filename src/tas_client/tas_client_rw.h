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

#pragma once

//! \addtogroup Read_Write_API
//! \{

// TAS includes
#include "tas_client_rw_base.h"
#include "tas_client_server_con.h"

// Standard includes
#include <cassert>

//! \brief This class facilitates read/write access to a target.
class CTasClientRw final : public CTasClientRwBase, public CTasClientServerCon
{
public:

	//! \brief Start a connection session
	//! \param identifier Unique access HW name or IP address of device as a c-string
	//! \param session_name Unique session name as a c-string
	//! \param session_pw Session password, specify to block other clients joining this session
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et session_start(const char* identifier, const char* session_name = "", const char* session_pw = "")
	{
		tas_return_et ret = mSessionStart(TAS_CLIENT_TYPE_RW, identifier, session_name, session_pw);

		assert(mTphRw == nullptr);
		if (ret == TAS_ERR_NONE) {
			mTphRw = new CTasPktHandlerRw(&mEi, get_con_info());
		}
		return ret;
	}

	//! \brief Performs a check if device reset has occurred.
	//! \returns \c true if reset occurred, otherwise \c false
	bool device_reset_occurred() override
	{
		if (!mSessionStarted) {
			return false;
		}

		uint32_t resetCount = mTphRw->get_device_reset_count();

		if (mDeviceResetCount > resetCount) {
			assert(mDeviceResetCount == ~0);  // device_connect() was not called
			assert(false);  // Wrong usage
			return false;
		}

		bool resetOccurred = (resetCount > mDeviceResetCount);

		mDeviceResetCount = resetCount;

		return resetOccurred;
	}

	//! \brief Connect to a target device
	//! \param dco Connection option, _RESET, _RESET_AND_HALT, _UNLOCK, default hot attach
	//! \return \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et device_connect(tas_clnt_dco_et dco) override
	{
		tas_return_et ret = mDeviceConnect(dco);
		if (mTphRw)
			mDeviceResetCount = mTphRw->get_device_reset_count();
		return ret;
	}

	//! \brief Read/Write client object constructor
	//! \param client_name Mandatory client name as a c-string
	explicit CTasClientRw(const char* client_name)
		: CTasClientRwBase(CTasPktHandlerRw::PKT_BUF_SIZE_DEFAULT)
		, CTasClientServerCon(client_name, &mEi)
	{
		mMbIfRw = mMbSocket;
		mMbIfRw->config(rw_get_timeout(), CTasPktHandlerRw::PKT_BUF_SIZE_DEFAULT);
	};

	//! \brief Read/Write client object constructor
	//! \details !!Only needed for testing purposes. Not for regular TAS clients!!
	//! \param mb_if Mailbox interface
	//! \param max_rq_size Defines maximum size of request packets
	//! \param max_rsp_size Defines maximum size of response packets
	//! \param max_num_rw Defines maximum number of read/write transactions
	CTasClientRw(CTasPktMailboxIf* mb_if, uint32_t max_rq_size, uint32_t max_rsp_size, uint32_t max_num_rw)
		: CTasClientRwBase(mb_if, max_rq_size, max_rsp_size, max_num_rw)
		, CTasClientServerCon("TestOnly", &mEi)
	{
		mMbIf = mb_if;
		mSessionStarted = true;
	};

};
//! \} // end of group Read_Write_API