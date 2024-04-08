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

//! \addtogroup Packet_Handler_RW
//! \{

// TAS includes
#include "tas_pkt_handler_base.h"

// Standard includes

//! \brief Derived packet handler class for handling read/write packets
class CTasPktHandlerRw : public CTasPktHandlerBase
{

public:
	CTasPktHandlerRw(const CTasPktHandlerRw&) = delete; //!< \brief delete the copy constructor
	CTasPktHandlerRw operator= (const CTasPktHandlerRw&) = delete; //!< \brief delete copy-assignment operator

	//! \brief Read/write packet handler object constructor.
	//! \details TasServer connection was established with CTasClientServerCon before.
	//! Uses default settings with 64KB for rq and rsp and 256 for max_num_rw.
	//! Please note that the effective data sizes are lower since these limits include the
	//! packet overhead which depends on the transaction data sizes and address alignment
	//! and the TAS frontend properties.
	//! \param ei pointer to the TAS error info 
	//! \param con_info pointer to the connection information
	CTasPktHandlerRw(tas_error_info_st* ei, const tas_con_info_st* con_info);
	
	//! \brief Read/write packet handler object constructor. Used for testing and inside of TasServer.
	//! \details Controls the allocated memory for packets and transaction descriptions.
	//! \param ei pointer to the TAS error info 
	//! \param max_rq_size defines maximum size of request packets
	//! \param max_rsp_size defines maximum size of response packets
	//! \param max_num_rw defines maximum number of read/write transactions
	CTasPktHandlerRw(tas_error_info_st* ei, uint32_t max_rq_size, uint32_t max_rsp_size, uint32_t max_num_rw);

	//! \brief Packet handler object destructor for cleanup.
	~CTasPktHandlerRw();

	// This API assumes a usage where:
	// Normally all transactions are successful
	// Errors are not handled individually for the transactions

	// Create packets option 1:
	//! \brief Initialization function for a new set of PL0 packets.
	//! \details The call to this method is followed by \ref rw_add_rd, \ref rw_add_wr, and/or \ref rw_add_fill. These
	//! functions return false and do not add, if the limits (default or set by constructor) are violated.
	//! However created packets up to this point stay valid. This supports a simple loop for splitting larger data accesses.
	void rw_start();

	//! \brief Add a read transaction. It can result in multiple PL0 packets in case of unaligned address.
	//! \param addr target address
	//! \param num_bytes number of bytes to be read
	//! \param data pointer to a read data buffer
	//! \param acc_mode access mode to be used, default: 0
	//! \param addr_map address map to be used, default: 0
	//! \returns \c true on success, otherwise \c false and does not add the transaction if the limits (default or set by constructor) are violated
	bool rw_add_rd(uint64_t addr, uint32_t num_bytes,       void* data, uint16_t acc_mode = 0, uint8_t addr_map = 0);

	//! \brief Add a write transaction. It can result in multiple PL0 packets in case of unaligned address.
	//! \param addr target address
	//! \param num_bytes number of bytes to be written
	//! \param data pointer to a data buffer
	//! \param acc_mode access mode to be used, default: 0
	//! \param addr_map address map to be used, default: 0
	//! \returns \c true on success, otherwise \c false and does not add the transaction if the limits (default or set by constructor) are violated
	bool rw_add_wr(uint64_t addr, uint32_t num_bytes, const void* data, uint16_t acc_mode = 0, uint8_t addr_map = 0);

	//! \brief Add a fill transaction. It can result in multiple PL0 packets in case of unaligned address.
	//! \param addr target address
	//! \param num_bytes number of bytes to be filled
	//! \param value fill value or pattern
	//! \param acc_mode access mode to be used, default: 0
	//! \param addr_map address map to be used, default: 0
	//! \returns \c true on success, otherwise \c false and does not add the transaction if the limits (default or set by constructor) are violated
	bool rw_add_fill(uint64_t addr, uint32_t num_bytes, uint64_t value, uint16_t acc_mode = 0, uint8_t addr_map = 0);


	// Create packets option 2:
	//! \brief Add a list of transactions. 
	//! \param trans pointer to a list of transactions
	//! \param num_trans number of transactions in the list, default: 1
	//! \returns \c true on success, otherwise \c false and if limits (default or set by constructor) are violated. No packets are created in this case.
	bool rw_set_trans(const tas_rw_trans_st* trans, uint32_t num_trans = 1);

	//! \brief Get a request size.
	//! \returns the size of all PL2 request packets
	uint32_t rw_get_rq_size() const;  

	//! \brief Get a response size.
	//! \details Returns the predicted size of all PL2 response packets (can be smaller in case of errors). If both are <= tas_con_info_st.max_pl2rq_pkt_size, max_pl1rqsp_pkt_size respectively only one PL2 packet is generated
	//! This means execution atomicity is warranted. This can be checked by num_pl2_pkt with rw_get_rq().
	//! \returns the predicted size of all PL2 response packets (can be smaller in case of errors)
	uint32_t rw_get_rsp_size() const; 

	//! \brief Finalize and get request packet(s). In case of read errors the rsp size can be smaller.
	//! \param rq pointer to the request
	//! \param rq_num_bytes pointer to the length of the request in bytes
	//! \param rsp_num_bytes_max pointer to the maximum response length in bytes
	//! \param num_pl2_pkt pointer to the number of pl2 packets
	void rw_get_rq(const uint32_t** rq, uint32_t* rq_num_bytes, uint32_t* rsp_num_bytes_max, uint32_t* num_pl2_pkt);
	
	//! \brief Get a number of PL2 packets in a response.
	//! \details Check if received response contains already all PL2 packets.
	//! Note that the response packet(s) can be smaller than predicted by rsp_num_bytes if there are read errors
	//! \param rsp pointer to a response buffer
	//! \param num_bytes length of a response in bytes
	//! \returns number of PL2 packets
	uint32_t rw_get_num_pl2_pkt(const uint32_t* rsp, uint32_t num_bytes) const;

	//! \brief Set received response packet(s).
	//! \details num_bytes will be smaller than rsp_num_bytes from rw_get_rq() in case of errors.
	//! \param rsp pointer to a response buffer
	//! \param num_bytes length of a response in bytes
	//! \returns \ref TAS_ERR_NONE on success, otherwise any other relevant TAS error code 
	tas_return_et rw_set_rsp(const uint32_t* rsp, uint32_t num_bytes);
	
	//! \brief Get a response form transactions.
	//! \details This method is for individual error handling or debugging.
	//! \param trans_rsp pointer to a list of transaction responses
	//! \returns the number of RW transactions
	uint32_t rw_get_trans_rsp(const tas_rw_trans_rsp_st** trans_rsp);
	
	//! \details This method is only for debugging.
	//! \param pl0_trans pointer to a list of pl0 transactions, is the same as trans_rsp if no transaction was split into different PL2 packets
	//! \param pl0_trans_rsp pointer to a list of pl0 transaction responses
	//! \returns the number of transactions on PL0 level
	uint32_t rw_get_pl0_trans(const tas_rw_trans_st** pl0_trans, const tas_rw_trans_rsp_st** pl0_trans_rsp) const;

	//! \brief Default limits.
	//! \details  Limits are for all generated PL2 packets together.
	enum {
		PKT_BUF_SIZE_DEFAULT = 0x10000,	//!< \brief default packet buffer size
		MAX_NUM_RW_DEFAULT = 256,		//!< \brief default maximum number of read/write transactions
		BUF_ALLOWANCE = 64,				//!< \brief buffer allowance for overhead
	}; 

private:

	//! \brief Read/write packet handler object initialization.
	//! \param max_rq_size maximum size of a request packet in bytes
	//! \param max_rsp_size maximum size of a response packet in bytes
	//! \param max_num_rw maximum number of read/write transactions
	void mInit(uint32_t max_rq_size, uint32_t max_rsp_size, uint32_t max_num_rw);

	void mEnforceDerivedClass() { ; }

	//! \brief Initialize a new PL2 packet. Called whenever a request needs to be split to multiple PL2 packets
	//! due to limitation of maximum allowed request size.
	void mPl2PktInit();

	//! \brief Set the address map, access mode, and base address for the next chain of PL0 packets. 
	//! \param addr_map selected address map
	//! \param acc_mode chosen access mode
	//! \param addr target address
	void mPktAdd_SetAddrMapAccModeBaseAddr(uint8_t addr_map, uint16_t acc_mode, uint64_t addr);

	//! \brief Add a PL0 read packet to the request.
	//! \param addr target address
	//! \param num_bytes number of bytes to be read
	//! \param data pointer to read data buffer
	//! \param acc_mode chosen access mode
	//! \param add_map selected address map
	void mPktAdd_Rd(uint64_t addr, uint32_t num_bytes, void* data, uint16_t acc_mode, uint8_t addr_map);

	//! \brief Add a PL0 write packet to the request.
	//! \param addr target address
	//! \param num_bytes number of bytes to be written
	//! \param data pointer to a data buffer
	//! \param acc_mode chosen access mode
	//! \param add_map selected address map
	void mPktAdd_Wr(uint64_t addr, uint32_t num_bytes, const void* data, uint16_t acc_mode, uint8_t addr_map);

	//! \brief Add a PL0 fill packet to the request.
	//! \param addr target address
	//! \param num_bytes number of bytes to be filled
	//! \param value fill value or pattern
	//! \param acc_mode chosen access mode
	//! \param add_map selected address map
	void mPktAdd_Fill(uint64_t addr, uint32_t num_bytes, uint64_t value, uint16_t acc_mode, uint8_t addr_map);

	//! \brief Check if a read/write transaction can be added to the request without overflowing the response buffer.
	//! \param num_bytes_needed_pktrq number of bytes needed in the request buffer for the transaction in question
	//! \param num_bytes_needed_pktrsp number of bytes needed in the response buffer for the transaction in question
	//! \returns \c true if the transaction fits in both, otherwise \c false
	bool mCheckRemainingPktSizeSufficient(uint32_t num_bytes_needed_pktrq, uint32_t num_bytes_needed_pktrsp) const
	{
		if (num_bytes_needed_pktrq > mGetRemainingSizeInPktRq())
			return false;
		else if (num_bytes_needed_pktrsp > mGetRemainingSizeInPktRsp())
			return false;
		return true;
	}

	//! \brief Wrap up current PL2 packet if there is no space to fit in the current transaction.
	//! \param num_bytes_needed_pktrq number of bytes needed in the request buffer for the transaction in question
	//! \param num_bytes_needed_pktrsp number of bytes needed in the response buffer for the transaction in question
	void mPktFinalizeIfNeeded(uint32_t num_bytes_needed_pktrq, uint32_t num_bytes_needed_pktrsp) {
		if (!mCheckRemainingPktSizeSufficient(num_bytes_needed_pktrq, num_bytes_needed_pktrsp))
			mPktFinalize();
	}

	//! \brief Wrap up current PL2 packet.
	//! \param init_next_pl2_pkt initialize the next PL2 packet, default: true
	void mPktFinalize(bool init_next_pl2_pkt = true);

	//! \brief Get the number of bytes required for address map, access mode, and base address PL0 packets.
	//! \param addr_map selected address map
	//! \param acc_mode chosen access mode
	//! \param addr target address
	//! \returns the combined size in bytes
	uint32_t mGetNumBytesAddrMapAccModeBaseAddr(uint8_t addr_map, uint16_t acc_mode, uint64_t addr) const;

	//! \brief Check buffer limits for a transaction in question. 
	//! \param num_bytes_rd number of bytes needed in a response, relevant for read transaction
	//! \param num_bytes_wr number of bytes needed in a request, relevant for write and fill transaction
	//! \returns \c true if the transaction fits, otherwise \c false.
	bool mCheckLimits(uint32_t num_bytes_rd, uint32_t num_bytes_wr) const;

	//! \brief Check address map rules. 
	//! \details A PL1 packet may contain a mix of PL0 address maps if all are less than 12.
    //! Rules for address maps 12, 13, TAS_AM14 and TAS_AM15: \n
    //! 1. May only be used exclusively in a Pl1 packet. \n
	//! \param addr_map chosen address map
	//! \returns \c true if chosen address map adheres to the rules, otherwise \c false
	bool mCheckAddrMapRulesInPkt(uint8_t addr_map) const;

	//! \brief Check if a read request generates a manageable amount of transactions without exceeding the maximum number 
	//! of read/write transactions allowed.
	//! \param addr target address
	//! \param num_bytes number of bytes to be read
	//! \returns \c true if adding the request would not exceed the maximum number of read/write transactions, otherwise \c false
	bool mNumTransManageableRd(uint64_t addr, uint32_t num_bytes) const;

	//! \brief Check if a write request generates a manageable amount of transactions without exceeding the maximum number 
	//! of read/write transactions allowed.
	//! \param addr target address
	//! \param num_bytes number of bytes to be written
	//! \returns \c true if adding the request would not exceed the maximum number of read/write transactions, otherwise \c false
	bool mNumTransManageableWr(uint64_t addr, uint32_t num_bytes) const;

	//! \brief Get the remaining available space in a request packet.
	//! \returns the size of remaining space in [bytes]
	uint32_t mGetRemainingSizeInPktRq() const;

	//! \brief Get the remaining available space in a response packet.
	//! \returns the size of remaining space in [bytes]
	uint32_t mGetRemainingSizeInPktRsp() const;

	//! \brief Get the size of a write block transfer that would still fit in the request packet.
	//! \param num_bytes number of bytes to be written
	//! \param addr target address
	//! \returns the size of a block in [bytes]
	uint32_t mGetWrDataBlkSizeInPktRq(uint32_t num_bytes, uint64_t addr) const;

	//! \brief Get the size of a read block transfer that would still fit in the response packet.
	//! \param num_bytes number of bytes to be read
	//! \returns the size of a block in [bytes]
	uint32_t mGetRdDataBlkSizeInPktRsp(uint32_t num_bytes) const;

	//! \brief Set server connection error in case of a PL1 count mismatch.
	//! \returns \ref TAS_ERR_SERVER_CON
	tas_return_et mSetPktRspErrPl1Cnt();

	//! \brief Converts PL0 error code into TAS error code.
	//! \param pl_err packet level 0 error code
	//! \param tas_err TAS error code 
	//! \param addr target address
	//! \param addr_map chosen address map
	void mSetPktRspErrPl0Data(uint8_t pl_err, tas_return_et tas_err, uint64_t addr, uint8_t addr_map);

	enum { 
		PROTOC_VER = 0 //!< \brief TasPkt protocol version implemented in this class
	};  

	uint32_t mNumPl2Pkt;	//!< \brief number of PL2 packets in a request

	uint32_t  mPl2HdrWi;	//!< \brief Word index of start of the current PL2 packet
	tas_pl1rq_pl0_start_st* mPl0Start;  //!< \brief PL0 start of the current PL2 packet
	uint32_t mPl2NumTrans;  //!< \brief Number of RW transactions in the current PL2 packet

	uint32_t mMaxWrDataBlkSizeInPktRq; //!< \brief Maximum size of a write block transfer. Depends on mConInfo
	uint32_t mMaxRdDataBlkSizeInPktRsp;  //!< \brief Maximum size of a read block transfer. Depends on mConInfo

	uint32_t mRspSize;         	//!< \brief Overall size of the PL2 packets [bytes]
	uint32_t mPl2RspPktStart;  	//!< \brief Start of the current PL2 packet [bytes|

	uint16_t mPl0AccMode;		//!< \brief Selected access mode for the current set of PL0 packets
	uint8_t  mPl0AddrMap;		//!< \brief Chosen address map for the current set of PL0 packets
	uint16_t mPl0AddrMapMask;  	//!< \brief Address maps used by this PL1 packet. Bit 15 represents TAS_AM15.
	uint64_t mPl0BaseAddr;		//!< \brief Target base address for the current set of PL0 packets

	// Transactions on API level, determined by caller
	tas_rw_trans_st* mRwTrans;	//!< \brief Pointer to an internal list of transactions, used for packet building
	tas_rw_trans_rsp_st* mRwTransRsp;	//!< \brief Pointer to an internal list of transaction responses
	uint32_t mRwNumTrans;   //!< \brief Used as index for mRwTrans[] and mRwTransRsp[]

	// Resulting transactions on PL0 level
	tas_rw_trans_st* mPl0Trans;	//!< \brief Pointer to an internal list of PL0 transactions
	tas_rw_trans_rsp_st* mPl0TransRsp;	//! \brief Pointer to an internal list of PL0 transaction responses
	uint32_t mPl0NumTrans;   //!< \brief Number of PL0 transactions. Used also as index for mPl0Trans[] and mPl0TransRsp[]

	uint32_t mNumTransMax;  //!< \brief mRwNumTrans <= mPl0NumTrans

	bool mGetPktRqWasCalled; //!< \brief Flag to indicated whether get a request method was called or not 
};

//! \} // end of group Packet_Handlers_RW