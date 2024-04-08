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

//! \defgroup TAS_utils Utility functions
//! \brief Description of TAS utility functions

//! \addtogroup TAS_utils
//! \{

// TAS includes
#include "tas_pkt.h"
#include "tas_client.h"

// Standard includes
#include <chrono>
#include <thread>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <array>

#define _unused(x)	((void)(x))	//!< \brief Specifies an unused varable or paramter

//! \brief Acquire time since epoch in microseconds.
//! \returns time since epoch in us
inline uint64_t tasutil_time_high_resolution_clock_us() 
{
	using namespace std::chrono;
	return duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

//! \brief Acquire time since epoch in milliseconds.
//! \return time since epoch in ms
inline uint64_t tasutil_time_ms()
{
	return tasutil_time_high_resolution_clock_us() / 1000;
}

//! \brief Acquire time since epoch.
//! \returns time since epoch 
inline time_t tasutil_time_since_epoch()
{
	using namespace std::chrono;
	return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

//! \brief Delay function.
//! \param x delay in milliseconds
inline void tasutil_sleep_ms(long long x) 
{
	std::this_thread::sleep_for(std::chrono::milliseconds(x));
}

//! \brief Check if device is connected with a standard debug interface.
//! \param dev_con_phys physical device connection identifier
//! \returns \c true if yes, otherwise \c false
inline bool tasutil_dev_con_phys_is_acc_hw(uint8_t dev_con_phys)
{
	switch (dev_con_phys) {
	case TAS_DEV_CON_PHYS_JTAG:
	case TAS_DEV_CON_PHYS_DAP:
	case TAS_DEV_CON_PHYS_DAP_DAP:
	case TAS_DEV_CON_PHYS_DAP_DXCPL:
	case TAS_DEV_CON_PHYS_DAP_DXCM:
	case TAS_DEV_CON_PHYS_SWD:
		return true;
	case TAS_DEV_CON_PHYS_ETH:
		return false;
	default: assert(false);
	}
	return false;
}

//! \brief Check if the physical device connection supports JTAG scan.
//! \param dev_con_phys physical device connection identifier
//! \returns \c true if yes, otherwise \c false
inline bool tasutil_dev_con_phys_supports_jtag_scans(uint8_t dev_con_phys)
{
	switch (dev_con_phys) {
	case TAS_DEV_CON_PHYS_JTAG:
	case TAS_DEV_CON_PHYS_DAP:
	case TAS_DEV_CON_PHYS_DAP_DAP:
	case TAS_DEV_CON_PHYS_DAP_DXCPL:
		return true;
	case TAS_DEV_CON_PHYS_DAP_DXCM:
	case TAS_DEV_CON_PHYS_SWD:
	case TAS_DEV_CON_PHYS_ETH:
		return false;
	default: assert(false);
	}
	return false;
}

enum { TASUTIL_TIME_STR_BUF_SIZE = 20 };

//! \brief Get a time string in YY-MM-DD hh:mm:ss format given the time value.
//! \param time time value to be transformed 
//! \param str_buf pointer to a string buffer
inline void tasutil_get_time_str(time_t time, char* str_buf)
{
#ifdef _MSC_VER
	struct tm timeTm;
	localtime_s(&timeTm, &time);
	strftime(str_buf, 20, "%Y-%m-%d %H:%M:%S", &timeTm);
#else
	struct tm timeTm;
	localtime_r(&time, &timeTm);
	strftime(str_buf, 20, "%Y-%m-%d %H:%M:%S", &timeTm);
#endif
	assert(strlen(str_buf) < TASUTIL_TIME_STR_BUF_SIZE);
}

//! \brief Get a string representation of a client type.
//! \param client_type client type identifier
//! \returns pointer to a c-string 
inline const char* tasutil_get_client_type_str(uint8_t client_type)
{
	switch (client_type) {
	case TAS_CLIENT_TYPE_RW:  return "ClientRw";
	case TAS_CLIENT_TYPE_CHL: return "ClientChl";
	case TAS_CLIENT_TYPE_TRC: return "ClientTrc";
	default: assert(false); return "UNKNOWN";
	}
}

//! \brief Get a string representation of the physical device connection. 
//! \param dev_con_phys physical device connection identifier
//! \returns pointer to a c-string 
inline const char* tasutil_get_dev_con_phys_str(uint8_t dev_con_phys)
{
	switch (dev_con_phys) {
	case TAS_DEV_CON_PHYS_UNKNOWN:   return "UNKNOWN";
	case TAS_DEV_CON_PHYS_JTAG:      return "JTAG";
	case TAS_DEV_CON_PHYS_DAP:       return "DAPx";
	case TAS_DEV_CON_PHYS_DAP_DAP:   return "DAP";
	case TAS_DEV_CON_PHYS_DAP_DXCPL: return "DXCPL";
	case TAS_DEV_CON_PHYS_DAP_DXCM:  return "DXCM";
	case TAS_DEV_CON_PHYS_SWD:       return "SWD";
	case TAS_DEV_CON_PHYS_ETH:       return "ETH";
	default: assert(false);          return "UNKNOWN";
	}
}

enum { TASUTIL_DEV_CON_FEAT_STR_BUF_SIZE = 128 };

//! \brief Get a string representation of the device connection feature.
//! \param dev_con_feat connection feature identifier
//! \param str_buf pointer to a string buffer
void tasutil_get_dev_con_feat_str(uint16_t dev_con_feat, char* str_buf);

enum { TASUTIL_CON_INFO_STR_BUF_SIZE = 1024 };

//! \brief Get a string representation of the connection information.
//! \param con_info pointer to the connection information
//! \param str_buf pointer to a string buffer
//! \param is_client_chl is the client type CHL, optional, default: \c false
//! \returns length of the str_buf in bytes
int tasutil_get_con_info_str(const tas_con_info_st* con_info, char* str_buf, bool is_client_chl = false);

enum { TASUTIL_SERVER_INFO_STR_BUF_SIZE = 256 };

//!	\brief Get a string representation of the server information.
//! \param ip_addr pointer to the server's hostname
//! \param port_num server's port number
//! \param si pointer to the server information
//! \param str_buf pointer to a string buffer
//! \returns length of the str_buf in bytes
int tasutil_get_server_info_str(const char* ip_addr, uint16_t port_num, const tas_server_info_st* si, char* str_buf);

//! \brief Get a string representation of the list of clients.
//! \param session_name pointer to a c-string containing the session name
//! \param session_start_time_us start time of the session in us
//! \param tci pointer to a list of clients 
//! \param num_client number of clients in the list
//! \param str_buf pointer to a string buffer
//! \param str_buf_size size of the string buffer
//! \returns length of the str_buf in bytes
int tasutil_get_target_clients_str(
	const char* session_name, uint64_t session_start_time_us,
	const tas_target_client_info_st* tci, uint32_t num_client, 
	char* str_buf, int str_buf_size);

//! \brief Check if the device is connected with a variant of DAP.
//! \param dev_con_phys physical device connection identifier
//! \returns \c true if yes, otherwise \c false
inline bool tasutil_dev_con_phys_is_dap_variant(tas_dev_con_phys_et dev_con_phys)
{
	switch (dev_con_phys) {
	case TAS_DEV_CON_PHYS_DAP:
	case TAS_DEV_CON_PHYS_DAP_DAP:
	case TAS_DEV_CON_PHYS_DAP_DXCPL:
	case TAS_DEV_CON_PHYS_DAP_DXCM:
		return true;
	default:
		return false;
	}
}

//! \brief Set an assertion
//! \param ret TAS return value
//! \returns the same TAS return value (in release mode)
inline tas_return_et tasutil_assert(tas_return_et ret)
{
	if (ret != TAS_ERR_NONE)
		assert(ret == TAS_ERR_NONE);  // Set breakpoint here
	return ret;
}

//! \brief Calculate a 32-bit CRC
//! \param data32 pointer to data (single entry should be 32-bit)
//! \param n_bytes number of bytes
//! \returns 32-bit CRC value
inline uint32_t tasutil_crc32(const uint32_t* data32, uint32_t n_bytes)
{
	// Ethernet CRC. In case of mismatch consider
	// 1. Binary inversion
	// 2. MSB first vs LSB first 

	static const std::array<uint32_t, 16> crcTable =
	{
	  0x4DBDF21C, 0x500AE278, 0x76D3D2D4, 0x6B64C2B0,
	  0x3B61B38C, 0x26D6A3E8, 0x000F9344, 0x1DB88320,
	  0xA005713C, 0xBDB26158, 0x9B6B51F4, 0x86DC4190,
	  0xD6D930AC, 0xCB6E20C8, 0xEDB71064, 0xF0000000
	};

	auto data8 = (const uint8_t*)data32;
	uint32_t crc = 0;
	for (uint32_t n = 0; n < n_bytes; n++)
	{
		crc = (crc >> 4) ^ crcTable[(crc ^ (data8[n] >> 0)) & 0x0F];
		crc = (crc >> 4) ^ crcTable[(crc ^ (data8[n] >> 4)) & 0x0F];
	}
	return crc;
}

enum { TASUTIL_HASH32_STRBUF_SIZE = 8 };  // 6 + zero char would suffice

//! \brief Convert 32-bit hash into a string representation. 
//! \details Creates a string out of capital letters and numbers. 
//! The string starts with a letter and has 6 characters. 
//! The 3 MSB of the 32 bit hash are not used for the string generation.
//! \param hash32 hash value
//! \param hash32_str pointer to a c-string representing the hash32
void tasutil_hash32_to_str(uint32_t hash32, char* hash32_str);

//! \brief Check if the device identifier corresponds to TCP/IP identifier, which should be an IP address.
//! \param identifier device identifier
//! \returns \c true if yes, otherwise \c false
inline bool tasutil_check_if_tcpip_identifier(const char* identifier)
{
	assert(strlen(identifier) < TAS_NAME_LEN64);
	int nItems;
	int a;
	int b;
	int c;
	int d;

#ifdef _MSC_VER
	nItems = sscanf_s(identifier, "%d.%d.%d.%d", &a, &b, &c, &d);
#else
	nItems = sscanf(identifier, "%d.%d.%d.%d", &a, &b, &c, &d);
#endif
	return (nItems == 4);
}

//! \brief Extract an unsigned 64-bit value from a string.
//! \param str pointer to a c-string to be checked
//! \param format pointer to c-string specifying the str format
//! \param value pointer to the extracted 64-bit value, 0 if more than one found
//! \returns on success, the function returns the number of items in the argument list successfully filled
inline int tasutil_sscanf_u64(const char* str, const char* format, uint64_t* value)
{
#ifdef _MSC_VER
	int nItems = sscanf_s(str, format, value);
#else
	int nItems = sscanf(str, format, value);
#endif
	if (nItems != 1)
		*value = 0;
	return nItems;
}

//! \brief Extract an unsigned 32-bit value from a string.
//! \param str pointer to a c-string to be checked
//! \param format pointer to c-string specifying the str format
//! \param value pointer to the extracted 32-bit value, 0 if more than one found
//! \returns on success, the function returns the number of items in the argument list successfully filled
inline int tasutil_sscanf_u32(const char* str, const char* format, uint32_t* value)
{
#ifdef _MSC_VER
	int nItems = sscanf_s(str, format, value);
#else
	int nItems = sscanf(str, format, value);
#endif
	if (nItems != 1)
		*value = 0;
	return nItems;
}

//! \brief Retrieve the user name of the host system.
//! \details Allocates \ref TAS_NAME_LEN16 long buffer for the user name string.
//! \param user_name pointer to a c-string containing the user name
void tasutil_get_user_name(char* user_name);

//! \brief Retrieve a process id of the application
//! \returns the process id
uint32_t tasutil_get_pid();

//! \brief Check if local TAS server is already running. Windows only.
//! \returns \c true if yes, otherwise \c false
bool tasutil_check_local_tas_server_is_running();

//! \brief Start a local TAS server. Windwos only.
void tasutil_start_local_tas_server();

//! \brief Class not used anywhere
class CTasutilDeviceFrequ
{
public:

	CTasutilDeviceFrequ();

	void add_timer_sample(uint32_t timer_value, uint64_t timestamp_before_us, uint64_t timestamp_after_us);

	void get_timer_frequency(double* frequency_hz, double* max_deviation_hz) const;

	void clear() {
		mClearRefValue();
		mClearValue();
	}

private:

	void mClearRefValue();
	void mClearValue();

	uint32_t mRefTimerValue;
	uint32_t mTimerValue;
	uint64_t mRefTimestampBeforeUs;
	uint64_t mRefTimestampAfterUs;
	uint64_t mTimestampBeforeUs;
	uint64_t mTimestampAfterUs;

};

//! \} // end of group TAS_utils