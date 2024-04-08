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

//! \addtogroup Client_API
//! \{

#undef TAS_DBG		
#ifdef TAS_DEBUG	// Check if TAS debugging is enabled
#define TAS_DBG 1	//!< \brief debugging prints enabled
#else
#define TAS_DBG 0	//!< \brief debugging prints disabled
#endif

// Inspired by StackOverflow answer: https://stackoverflow.com/a/1644898

//! \brief Helper marco to print debug info 
#define tas_debug_print(fmt, ...) \
	{ if (TAS_DBG) fprintf(stderr, fmt, ##__VA_ARGS__); }

//! \brief Helper macro to print debug info including location
#define tas_debug_printloc(fmt, ...) \
	{ if (TAS_DBG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); }
	
//! \} // end of group Client_API