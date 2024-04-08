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
#include "tas_device_family.h"
#include "tas_utils_ifx.h"

// Standard includes
#include <cstdio>
#include <cassert>


bool tasutil_ifx_get_device_name(uint32_t device_type, uint32_t device_id0, char* device_name)
{
	tas_device_family_t deviceFamily = tas_get_device_family(device_type);

	if (   (deviceFamily == TAS_DF_TC2X)
		|| (deviceFamily == TAS_DF_TC3X)
		|| (deviceFamily == TAS_DF_TC4X)) {
		if (device_id0 == 0) {
			snprintf(device_name, TAS_NAME_LEN32, "UNKNOWN");
		}
		else {
			snprintf(device_name, TAS_NAME_LEN32, "%s", tas_get_device_name_str(device_type));

		}
		return true;
	}
	else {
		snprintf(device_name, TAS_NAME_LEN32, "device_type 0x%8.8X", device_type);
		return false;
	}
}

