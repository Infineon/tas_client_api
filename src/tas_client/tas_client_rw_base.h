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
#include "tas_client_impl.h"
#include "tas_pkt_mailbox_if.h"
#include "tas_pkt_handler_rw.h"
#include "tas_am15_am14.h"

// Standard includes
#include <vector>

//! \brief Base class for read/write operations. 
//! \details This API assumes that the timeout and the size and number of transactions are configured
//! in a way that no timeout will occur. If a timeout occurs the server connection has to be setup again.
//!
//! \note The read/write class is split into two classes as functionality of the base class is needed also within 
//! TAS server. 
class CTasClientRwBase
{

public:
	CTasClientRwBase(const CTasClientRwBase&) = delete; //!< \brief delete the copy constructor
	CTasClientRwBase operator= (const CTasClientRwBase&) = delete; //!< \brief delete copy-assignment operator

	//! \brief Base class object constructor.
	//! \param max_rsp_size Defines maximum response packet size
	explicit CTasClientRwBase(uint32_t max_rsp_size);

	//! \brief Base class object destructor. Used for cleanup.
	~CTasClientRwBase();

	//! \brief Execute an 8-bit read operation
	//! \param addr 64-bit address at which the operation is performed
	//! \param value Pointer to a data buffer to which the read data is stored
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et read8(uint64_t addr, uint8_t* value, uint8_t addr_map = TAS_AM0);

	//! \brief Execute an 8-bit write operation
	//! \param addr 64-bit address at which the operation is performed
	//! \param value Data to be written
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et write8(uint64_t addr, uint8_t value, uint8_t addr_map = TAS_AM0);

	//! \brief Execute a 16-bit read operation
	//! \param addr 64-bit address at which the operation is performed
	//! \param value Pointer to a data buffer to which the read data is stored
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et read16(uint64_t addr, uint16_t* value, uint8_t addr_map = TAS_AM0);

	//! \brief Execute a 16-bit write operation
	//! \param addr 64-bit address at which the operation is performed
	//! \param value Data to be written
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et write16(uint64_t addr, uint16_t value, uint8_t addr_map = TAS_AM0);

	//! \brief Execute a 32-bit read operation
	//! \param addr 64-bit address at which the operation is performed
	//! \param value Pointer to a data buffer to which the read data is stored
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et read32(uint64_t addr, uint32_t* value, uint8_t addr_map = TAS_AM0);

	//! \brief Execute a 32-bit write operation
	//! \param addr 64-bit address at which the operation is performed
	//! \param value Data to be written
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et write32(uint64_t addr, uint32_t value, uint8_t addr_map = TAS_AM0);

	//! \brief Execute a 64-bit read operation
	//! \param addr 64-bit address at which the operation is performed
	//! \param value Pointer to a data buffer to which the read data is stored
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et read64(uint64_t addr, uint64_t* value, uint8_t addr_map = TAS_AM0);

	//! \brief Execute a 64-bit write operation
	//! \param addr 64-bit address at which the operation is performed
	//! \param value Data to be written
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et write64(uint64_t addr, uint64_t value, uint8_t addr_map = TAS_AM0);

	//! \brief Execute a read operation
	//! \param addr 64-bit address at which the operation is performed
	//! \param data Pointer to a data buffer to which the read data is stored
	//! \param num_bytes Number of bytes to be read
	//! \param num_bytes_ok Pointer to a variable holding the number of bytes read successfully
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et read(uint64_t addr, void* data, uint32_t num_bytes, uint32_t* num_bytes_ok, uint8_t addr_map = TAS_AM0);

	//! \brief Execute a write operation
	//! \param addr 64-bit address at which the operation is performed
	//! \param data Pointer to a data buffer from which the data is written
	//! \param num_bytes Number of bytes to be written
	//! \param num_bytes_ok Pointer to a variable holding the number of bytes written successfully
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et write(uint64_t addr, const void* data, uint32_t num_bytes, uint32_t* num_bytes_ok, uint8_t addr_map = TAS_AM0);
	
	//! \brief Execute a fill operation with a 32-bit value
	//! \details addr and num_bytes need to be 32 bit aligned. addr_map has to be lower than TAS_AM12.
	//! \param addr 64-bit address at which the operation is performed
	//! \param value 32-bit data value used as a pattern
	//! \param num_bytes Number of bytes to be written
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et fill32(uint64_t addr, uint32_t value, uint32_t num_bytes, uint8_t addr_map = TAS_AM0);

	//! \brief Execute a fill operation with a 64-bit value
	//! \details addr and num_bytes need to be 64 bit aligned. addr_map has to be lower than TAS_AM12.
	//! \param addr 64-bit address at which the operation is performed
	//! \param value 64-bit data value used as a pattern
	//! \param num_bytes Number of bytes to be written
	//! \param addr_map Address map to be used, default: \ref TAS_AM0
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et fill64(uint64_t addr, uint64_t value, uint32_t num_bytes, uint8_t addr_map = TAS_AM0);

	//! \brief Execute a series of read and write operations based on provided transaction list.
	//! \details Use transaction arrays for higher efficiency and for ensuring atomicity of the execution \n
	//! If trans contains a mixture of different address maps, the following rules apply: \n
	//! 1. If all address maps are in \ref TAS_AM_MASK_SHARED, the first transaction with an error
	//!    will return this error. Following transactions will return \ref TAS_PL0_ERR_CONSEQUENTIAL \n
	//! 2. For \ref TAS_AM_MASK_EXCLUSIVE the same applies for the single exclusive address map. \n
	//! 3. If trans consists of sequences of transactions which follow rule 1 or 2, these \n
	//!    rules are applied independently to these sequences of transactions
	//! A \ref TAS_RW_TT_FILL transaction needs to be 64 bit aligned (addr, num_bytes) \n
	//! \param trans Pointer to a list of transactions
	//! \param num_trans Number of transaction in the list
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et execute_trans(const tas_rw_trans_st* trans, uint32_t num_trans);
	

	// The following methods are only needed for special use cases and debugging

	//! \brief Send a ping to a target and obtain connection info
	//! \details Check the connection which was established with CTasClientServerCon::session_start() before.\n
	//! Only needed for special use cases and debugging.
	//! \param con_info Pointer to a variable to which connection info should be stored
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et target_ping(tas_con_info_st* con_info);

	//! \brief Get a transaction response
	//! \details This method is for individual error handling or debugging
	//! \param trans_rsp Response to trans or the single transaction from the other methods
	//! \returns the number of RW transactions
	uint32_t rw_get_trans_rsp(const tas_rw_trans_rsp_st** trans_rsp);

	//! \brief Set a timeout for read/write operations
	//! \details Default timeout is 10s. Recommended for TAS clients. No need to call in this case.
	//! Please note that a timeout is fatal so the server connection has to be setup again.
	//! \param timeout_ms Timeout value in milliseconds
	void rw_set_timeout(uint32_t timeout_ms);	

	//! \brief Get the timeout value set for read/write operations
	//! \returns 32-bit timeout value
	uint32_t rw_get_timeout();

	//! \brief Base class object constructor. !!Only used within the server and for special test setups!!
	//! \param mb_if Mailbox interface
	//! \param max_rq_size Defines maximum size of request packets
	//! \param max_rsp_size Defines maximum size of response packets
	//! \param max_num_rw Defines maximum number of read/write transactions
	CTasClientRwBase(CTasPktMailboxIf* mb_if, uint32_t max_rq_size, uint32_t max_rsp_size, uint32_t max_num_rw);

protected:

	tas_error_info_st mEi; //!< \brief Contains current error information. 	

	CTasPktMailboxIf* mMbIfRw;  //!< \brief Mailbox interface. Needed in addition to mMbIf due to usage in TasServer

	CTasPktHandlerRw* mTphRw = nullptr; //!< \brief Packet handler object. Used for constructing and parsing packets.

private:
	uint32_t mTimeoutMs = TAS_DEFAULT_TIMEOUT_MS;	//!< \brief Current timeout setting.

	std::vector<uint32_t> mRspBuf; //!< \brief Response packet buffer. For one or more PL2 packets.

	//! \brief Transforms simple read/write operations into single transaction execution
	//! \param trans Pointer to a transaction definition
	//! \param num_bytes_ok Pointer to a variable holding the number of successfully read or written Bytes
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code
	tas_return_et mExecuteSingleTrans(const tas_rw_trans_st* trans, uint32_t* num_bytes_ok = nullptr);

};

//! \} // end of group Read_Write_API