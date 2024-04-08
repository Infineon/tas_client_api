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
#include "tas_pkt.h"

// Standard includes
#include <cstdint>

//! \brief Get a device name.
//! \details It allocates TAS_NAME_LEN32 long buffer for the device name.
//! \param device_type device type identifier
//! \param device_id0 device ID (JTAG ID)
//! \param device_name pointer to a c-string containing device name
//! \returns \c true if device is known, otherwise \c false. 
bool tasutil_ifx_get_device_name(uint32_t device_type, uint32_t device_id0, char* device_name);

//! \} // end of group TAS_utils