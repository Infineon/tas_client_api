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

//! \addtogroup TAS_utils
//! \{

// TAS includes
#include "tas_client_rw.h"

// Standard includes
#include <cstring>

//! \brief Set a set of user pins to logic high.
//! \param tcrw pointer to a RW client object, which will execute the operation
//! \param pins set of pins to be set to high
//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
inline tas_return_et tasutil_userpins_set_high(CTasClientRw* tcrw, uint32_t pins)
{
	tas_userpins_mask_st up = { pins, pins };
	return tcrw->write64(TAS_AM15_RW_USERPINS, *(uint64_t*)&up, TAS_AM15);
}

//! \brief Set a set of user pins to logic low.
//! \param tcrw pointer to a RW client object, which will execute the operation
//! \param pins set of pins to be set to low
//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
inline tas_return_et tasutil_userpins_set_low(CTasClientRw* tcrw, uint32_t pins)
{
	tas_userpins_mask_st up = { 0, pins };
	return tcrw->write64(TAS_AM15_RW_USERPINS, *(uint64_t*)&up, TAS_AM15);
}

//! \brief Turn on the run led of access hardware.
//! \param tcrw pointer to a RW client object, which will execute the operation
//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
inline tas_return_et tasutil_acc_hw_runled_on(CTasClientRw* tcrw)
{
	return tasutil_userpins_set_low(tcrw, TAS_UP_RUNLED);
}

//! \brief Turn off the run led of access hardware.
//! \param tcrw pointer to a RW client object, which will execute the operation
//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
inline tas_return_et tasutil_acc_hw_runled_off(CTasClientRw* tcrw)
{
	return tasutil_userpins_set_high(tcrw, TAS_UP_RUNLED);
}

//! \brief Check if client is connected to an emulator.
//! \returns \c true if yes, otherwise \c false
inline bool tasutil_emulator_connected(CTasClientServerCon* client)
{
	return (strstr(client->get_con_info()->identifier, "Emulator") != NULL);;
}

//! \} // end of group TAS_utils