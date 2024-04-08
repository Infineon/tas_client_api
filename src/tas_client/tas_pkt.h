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

//! All values are little endian
//! Three levels of packet wrapping:
//! PL0 RW access steps, wrapped by PL1
//! PL1 RW, CHL, trace CHL
//! PL1 guarantees atomicity for the sequence of RW transactions
//! PL2 Just an initial 32 bit number which gives the overall PL1 length in bytes + 4

//! \defgroup Protocol_Definition Definition of TAS protocol and packet layers

//! \defgroup Packet_Level_2 Definition of packet layer 2 
//! \ingroup Protocol_Definition

//! \defgroup Packet_Level_1 Definition of packet layer 1
//! \ingroup Protocol_Definition

//! \defgroup Packet_Level_0 Definition of packet layer 0
//! \ingroup Protocol_Definition

#ifndef __tas_pkt_h
#define __tas_pkt_h

// Standard includes
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Definition of TAS protocol version.
//! \details Used in \ref tas_server_info_st .supp_protoc_ver -> limited to 31 versions.
//! \ingroup Protocol_Definition
typedef enum {
	TAS_PKT_PROTOC_VER_1 = 1,  //!< \brief Initial protocol version (value 0 is unknown/undefined/unused etc.)
} tas_protoc_ver_et;

//! \brief Collection of used port numbers.
//! \details Non-standard port numbers are used. Current list can be found at https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers.
//! \ingroup Protocol_Definition
enum {
	TAS_PORT_NUM_SERVER_DEFAULT = 24817,  //!< \brief Local network TasServer
	TAS_PORT_NUM_PROXY_DEFAULT  = 24818,  //!< \brief Remote test farm proxy server
	TAS_PORT_NUM_DEVICE_DEFAULT = 24823,  //!< \brief TasDeviceServer locally connected to TasServer
}; 

//! \brief String length helper constants.
//! \ingroup Protocol_Definition
enum {
	TAS_NAME_LEN64 = 64, 			//!< \brief Used for specifying a 64 Byte long string
	TAS_NAME_LEN32 = 32,			//!< \brief Used for specifying a 32 Byte long string
	TAS_NAME_LEN16 = 16,			//!< \brief Used for specifying a 16 Byte long string
	TAS_NAME_LEN8  =  8,			//!< \brief Used for specifying an 8 Byte long string

	TAS_HOST_ADDR_LEN = 128, 		//!< \brief Max host address length in Bytes

	TAS_MAX_PKT_SIZE_1KB = 1024,  	//!< \brief Maximum size limit of certain packet types
};

//! \brief Security related constants.
//! \ingroup Protocol_Definition
enum {
	TAS_UNLOCK_MAX_KEY_LEN       = 512,  //!< \brief Size in Bytes
	TAS_UNLOCK_MAX_CHALLENGE_LEN = 512,  //!< \brief Size in Bytes
};

//! \brief Supported client types.
//! \ingroup Protocol_Definition
typedef enum { 
	TAS_CLIENT_TYPE_UNKNOWN = 0, //!< \brief Unknown client type
	TAS_CLIENT_TYPE_RW      = 1, //!< \brief Read/write client identifier
	TAS_CLIENT_TYPE_CHL     = 2, //!< \brief Channel client identifier
	TAS_CLIENT_TYPE_TRC     = 3, //!< \brief Trace client identifier
} tas_client_type_et;

//! \brief Default properties of a packet.
//! \details default packet size, max block transfer, and channel massage size.
//! \ingroup Protocol_Definition
enum {
	TAS_PL2_MAX_PKT_SIZE     = 0x10008,  //!< \brief Max. 64KB payload
	TAS_PL2_MAX_PKT_SIZE_MIN = 128,      //!< \brief Minimum size an agent must support
	TAS_PL0_DATA_BLK_SIZE    = 1024,     //!< \brief Limited by protocol constraints
	TAS_PL1_CHL_MAX_MSG_SIZE = 1024,     //!< \brief Limited by protocol constraints
};

//! \brief Packet error.
//! \ingroup Protocol_Definition
typedef uint8_t tas_pl_err_et8;

//! \brief TAS packet error codes.
//! \ingroup Protocol_Definition
typedef enum {
	TAS_PL_ERR_NO_ERROR       = 0x08,	//!< \brief no error
	TAS_PL_ERR_PARAM          = 0x10,  	//!< \brief Wrong parameter value
	TAS_PL_ERR_NOT_SUPPORTED  = 0x12,  	//!< \brief Command or parameter not supported by this implementation or setup
	TAS_PL_ERR_USAGE          = 0x14,  	//!< \brief Not correctly used (e.g. wrong sequence of commands)
	TAS_PL_ERR_PROTOCOL       = 0x1E,  	//!< \brief Packet layer protocol error

	TAS_PL1_ERR_CMD_FAILED    = 0x20,	//!< \brief command requested by PL1 failed

	TAS_PL1_ERR_SERVER_LOCKED = 0x32,	//!< \brief server is locked

	TAS_PL1_ERR_SESSION       = 0x38,  	//!< \brief Session name or password mismatch

	TAS_PL1_ERR_DEV_RESET     = 0x44,  	//!< \brief Device reset was detected. Sent to all clients as an unsolicited PL2 packet
	TAS_PL1_ERR_DEV_LOCKED    = 0x46, 	//!< \brief Device is locked (no access possible)
	TAS_PL1_ERR_DEV_ACCESS    = 0x48,  	//!< \brief Device access (e.g. DAP connection)

	                                                    // Access
	TAS_PL0_ERR_NO_ERROR      = 0x80,                   //!< \brief yes
	TAS_PL0_ERR_DEV_LOCKED    = TAS_PL1_ERR_DEV_LOCKED, //!< \brief no     Device is locked (no access possible)
	TAS_PL0_ERR_DEV_ACCESS    = TAS_PL1_ERR_DEV_ACCESS, //!< \brief no     Device access (e.g. DAP connection)
	TAS_PL0_ERR_ACC_MODE      = 0x89,                   //!< \brief no     Set acc_mode is not supported
	TAS_PL0_ERR_ADDR_MAP      = 0x8A,                   //!< \brief no     Set addr_map is not supported
	TAS_PL0_ERR_ADDR_BLOCKED  = 0x8B,                   //!< \brief no     TasAgent protection prevents access to this address
	TAS_PL0_ERR_DATA          = 0x8D,                   //!< \brief tried  RW access was not (completely - blk) successful (e.g. device internal bus error)
	TAS_PL0_ERR_CONSEQUENTIAL = 0x98,                   //!< \brief impl   Consequential error since a previous PL0 command failed. Whether the access took place is implementation specific. Recommendation is no.

	TAS_PL0_ERR_HIGHEST       = 0x9F,                   //!< \brief no     Delimiter for PL0 error codes

} tas_pl_err_et;

//! \brief TAS packet command description
//! \details Commands are partitioned into two levels. PL1 and PL0.
//! PL1 packets are considered higher level commands most of which are used between a client and a server.
//! PL0 packets are used for read/write access.
//! \ingroup Protocol_Definition
typedef enum {

    // PL0 RW commands
	TAS_PL0_CMD_ACCESS_MODE     = 0x10,	 	//!< \brief No response pkt. Sets the access mode. 
	TAS_PL0_CMD_ADDR_MAP        = 0x11,  	//!< \brief No response pkt. Sets the address map to be used.
	TAS_PL0_CMD_BASE_ADDR32     = 0x15,  	//!< \brief No response pkt. Sets the a 32-bit base address for r/w commands that follow
	TAS_PL0_CMD_BASE_ADDR64     = 0x16,  	//!< \brief No response pkt. Sets the a 64-bit base address for r/w commands that follow
	TAS_PL0_CMD_WR8             = 0x20,  	//!< \brief TAS_PL0_ERR_. Issues a request for an 8-bit write operation.
	TAS_PL0_CMD_RD8             = 0x21,  	//!< \brief TAS_PL0_ERR_. Issues a request for an 8-bit read operation.
	TAS_PL0_CMD_WR16            = 0x22,  	//!< \brief TAS_PL0_ERR_. Issues a request for a 16-bit write operation.
	TAS_PL0_CMD_RD16            = 0x23,  	//!< \brief TAS_PL0_ERR_. Issues a request for a 16-bit read operation.
	TAS_PL0_CMD_WR32            = 0x24,  	//!< \brief TAS_PL0_ERR_. Issues a request for a 32-bit write operation.
	TAS_PL0_CMD_RD32            = 0x25,  	//!< \brief TAS_PL0_ERR_. Issues a request for a 32-bit read operation. 
	TAS_PL0_CMD_WR64            = 0x26,  	//!< \brief TAS_PL0_ERR_. Issues a request for a 64-bit write operation.
	TAS_PL0_CMD_RD64            = 0x27,  	//!< \brief TAS_PL0_ERR_. Issues a request for a 64-bit read operation.
	TAS_PL0_CMD_WRBLK           = 0x2A,  	//!< \brief TAS_PL0_ERR_. Issues a request for a block write operation.
	TAS_PL0_CMD_FILL            = 0x2B,  	//!< \brief TAS_PL0_ERR_. Issues a request for a fill operation.
	TAS_PL0_CMD_RDBLK           = 0x2C,  	//!< \brief TAS_PL0_ERR_. Issues a request for a block read operation.
	TAS_PL0_CMD_RDBLK1KB        = 0x2D,  	//!< \brief TAS_PL0_ERR_NO_ERROR (used for response if 1kb data read without error). 
											//! Issues a request or 1KB block read operation.

	// PL1 commands
	TAS_PL1_CMD_SERVER_CONNECT  = 0x80,	 	//!< \brief Used for establishing a connection with a server.

	TAS_PL1_CMD_GET_TARGETS     = 0x86, 	//!< \brief Used for requesting a list of targets connected to a server.
	TAS_PL1_CMD_GET_CLIENTS     = 0x87, 	//!< \brief Used for requesting a list of clients connected to a server.

	TAS_PL1_CMD_SESSION_START   = 0xA1, 	//!< \brief Used for opening a connection session.
	TAS_PL1_CMD_PING            = 0xA2, 	//!< \brief Used for checking the connection which was established with session start.
	TAS_PL1_CMD_DEVICE_CONNECT  = 0xA8, 	//!< \brief Used for connecting to a target (hot attach),or  with optional reset or reset and halt.
	TAS_PL1_CMD_DEVICE_RESET_COUNT = 0xAA, 	//!< \brief Used for checking the number of times a target was reset.

	TAS_PL1_CMD_GET_CHALLENGE   = 0xAC,		//!< \brief TBD.
	TAS_PL1_CMD_SET_DEVICE_KEY  = 0xAD,		//!< \brief Set a device key for device unlocking.

	TAS_PL1_CMD_PL0_START       = 0xB0,		//!< \brief Used for indicating a start of PL0 sequence.
	TAS_PL1_CMD_PL0_END         = 0xB1,		//!< \brief Used for indicating an end of PL0 sequence.

	TAS_PL1_CMD_CHL_SUBSCRIBE   = 0xC0,		//!< \brief Used for subscribing to a channel number.
	TAS_PL1_CMD_CHL_UNSUBSCRIBE = 0xC2,		//!< \brief Used for unsubscribing from a subscribed channel number.
	TAS_PL1_CMD_CHL_MSG_C2D     = 0xCC,		//!< \brief Send a message from a client to a device.
	TAS_PL1_CMD_CHL_MSG_D2C     = 0xCD,		//!< \brief Receive a message from a device.

	TAS_PL1_CMD_TRC_SUBSCRIBE   = 0xD0,		//!< \brief TBD.
	TAS_PL1_CMD_TRC_UNSUBSCRIBE = 0xD2,		//!< \brief TBD.
	TAS_PL1_CMD_TRC_DATA        = 0xD4,		//!< \brief TBD.

	TAS_PL1_CMD_ERROR           = 0xEE,		//!< \brief Error packet.
} tas_pl_cmd_et;

//! \brief Device connection features.
//! \details Values can be bitwise or-ed.
//! \ingroup Protocol_Definition
typedef enum { 
	TAS_DEV_CON_FEAT_NONE           =      0, 	//!< \brief None of the features used.
	TAS_DEV_CON_FEAT_RESET          = 0x0001, 	//!< \brief Triggers a device reset. 
	TAS_DEV_CON_FEAT_RESET_AND_HALT = 0x0003,	//!< \brief Triggers a reset and halt. 
	TAS_DEV_CON_FEAT_UNLOCK         = 0x0010, 	//!< \brief Unlock a device.
	TAS_DEV_CON_FEAT_UNLOCK_CNR     = 0x0030,  	//!< \brief Unlocking with challenge and response protocol
	TAS_DEV_CON_FEAT_UNKNOWN        = 0x4000,	//!< \brief Connection to an unknown device. In respect to JTAG ID.
	TAS_DEV_CON_FEAT_UNKNOWN_RESET  = 0x4001,	//!< \brief Triggers a reset of unknown device. In respect to JTAG ID.
} tas_dev_con_feat_et;

//! \brief Definitions of possible physical connections to a device.
//! \ingroup Protocol_Definition
typedef enum {
	// Connections where the identifier is provided by a unique tool access HW identifier
	TAS_DEV_CON_PHYS_UNKNOWN    = 0,
	TAS_DEV_CON_PHYS_JTAG       = 0x01,
	TAS_DEV_CON_PHYS_DAP        = 0x10,	//!< \brief DAP, DXCPL or DXCM
	TAS_DEV_CON_PHYS_DAP_DAP    = 0x11,
	TAS_DEV_CON_PHYS_DAP_SPD    = 0x12,
	TAS_DEV_CON_PHYS_DAP_DXCPL  = 0x13,
	TAS_DEV_CON_PHYS_DAP_DXCM   = 0x14,
	TAS_DEV_CON_PHYS_SWD        = 0x20,
	TAS_DEV_CON_PHYS_ACC_HW_MAX = 0x7F,

	// Connections which have an intrinsic unique identifier
	TAS_DEV_CON_PHYS_ETH        = 0x80,
}  tas_dev_con_phys_et;

//! \brief Options for how to obtain a challenge for unlocking.
//! \ingroup Protocol_Definition
typedef enum {
	TAS_DEV_ULCRO_UDID     = 0x01,	//!< \brief Challenge is the unique device ID which can be read from the locked device
	TAS_DEV_ULCRO_UDID_RST = 0x81,	//!< \brief Unique device ID can only be read after a reset
	TAS_DEV_ULCRO_CR       = 0x04,	//!< \brief Challenge is read from the locked device
	TAS_DEV_ULCRO_CR_RST   = 0x84,	//!< \brief Challenge can only be read after a reset
	TAS_DEV_ULCRO_CUSTOM0  = 0xC0,	//!< \brief Option 0 for custom protocol for retrieving the challenge
	TAS_DEV_ULCRO_CUSTOM1  = 0xC1,	//!< \brief Option 1
	TAS_DEV_ULCRO_CUSTOM2  = 0xC2,	//!< \brief Option 2
	TAS_DEV_ULCRO_CUSTOM3  = 0xC3,	//!< \brief Option 3
} tas_dev_unlock_cr_option_et;

//! \brief Device unlocking options.
//! \ingroup Protocol_Definition
typedef enum {
	TAS_DEV_ULO_HOT_ATTACH         = 0x01,  //!< \brief Unlocking is possible any time
	TAS_DEV_ULO_AFTER_RST_BY_FW    = 0x08,  //!< \brief Unlocking by FW before application SW is started
	TAS_DEV_ULO_AFTER_RST_BY_APPSW = 0x09,  //!< \brief Unlocking by application SW in a time window after reset
} tas_dev_unlock_option_et;

//! \brief Target's access mode.
//! \ingroup Protocol_Definition
typedef enum {
	TAS_PL0_ACC_MODE_DEFAULT = 0x0000,  //!< \brief Default RW access mode for the target device
} tas_pl0_acc_mode_et;

// ____________________________________________________________________________________________________________________
// Packet Level 2

//! \brief Packet level 2.
//! \ingroup Packet_Level_2
typedef struct {
	union {
		struct { 
			uint16_t length; 	//!< \brief overall packet length
			uint8_t  type;		//!< \brief additional type encoding 
			uint8_t  param;		//!< \brief additional parameters
		};
		uint32_t u32;			//!< \brief 32-bit representation 
	};
} tas_pl2_header_st;

// ____________________________________________________________________________________________________________________
// Packet level 1 which is wrapping packet level 0

//! \brief PL1 request header.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;         //!< \brief Word Length of a packet excluding this first word
	uint8_t  cmd;        //!< \brief TAS_PL1_CMD_xxx
	uint8_t  con_id;     //!< \brief Connection identifier of TasDispatcher
} tas_pl1rq_header_st;

//! \brief PL1 response header
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;         //!< \brief Word Length of packet excluding this first word
	uint8_t  cmd;        //!< \brief TAS_PL1_CMD_xxx
	uint8_t  con_id;     //!< \brief Connection identifier of TasDispatcher
	uint8_t  err;        //!< \brief tas_pl_err_et
} tas_pl1rsp_header_st;

//! \brief Server information.
//! \ingroup Packet_Level_1
typedef struct {
	char      server_name[TAS_NAME_LEN64];	//!< \brief storage for c-string containing the server name max 64 bytes long
	uint16_t  v_minor;          //!< \brief Version minor
	uint16_t  v_major;          //!< \brief Version major
	uint32_t  supp_protoc_ver;  //!< \brief Supported protocol versions.  tas_protoc_ver_et defines the bit number
	uint32_t  supp_chl_target;  //!< \brief Supported channel targets.    tas_chl_target_et defines the bit number
	uint32_t  supp_trc_type;    //!< \brief Supported trace stream types. tas_trc_type_et   defines the bit number
	uint32_t  reserved[4];		//!< \brief Reserved field: 0
	char      date[16];         //!< \brief String from __DATE__ macro at compile time
	uint64_t  start_time_us;    //!< \brief Time the server was started, as microseconds elapsed since midnight, January 1, 1970
} tas_server_info_st;

//! \brief Request to connect to a server.
//! \details Response: \ref tas_pl1rsp_server_connect_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;          //!< \brief Request size: 10
	uint8_t  cmd;         //!< \brief Command identifier: TAS_PL1_CMD_SERVER_CONNECT
	uint16_t reserved;    //!< \brief Reserved field: 0

	char     client_name[TAS_NAME_LEN32];	//!< \brief Client name as a c-string 
	char     user_name[TAS_NAME_LEN16];		//!< \brief User name as a c-string
	uint32_t client_pid;  					//!< \brief Process ID of a client

} tas_pl1rq_server_connect_st;

//! \brief Response to the server connect request \ref tas_pl1rq_server_connect_st.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;         //!< \brief Response size (excluding header): 25
	uint8_t  cmd;        //!< \brief Command identifier: TAS_PL1_CMD_SERVER_CONNECT
	uint8_t  reserved;   //!< \brief Reserved field: 0
	uint8_t  err;        //!< \brief Error code: TAS_PL_ERR_NO_ERROR, TAS_PL1_ERR_CMD_FAILED or TAS_PL1_ERR_SERVER_LOCKED
	uint32_t reserved1;  //!< \brief Reserved field: Ensures no padding by compiler for 64 bit alignment

	tas_server_info_st server_info; //!< \brief Server information: \ref tas_server_info_st

	uint64_t challenge;	//!< \brief Challenge value

} tas_pl1rsp_server_connect_st;

//! \brief Response to server unlock request.
//! \details Work in progress...
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;         //!< \brief Response size (excluding header): ?
	uint8_t  cmd;        //!< \brief Command identifier: TAS_PL1_CMD_SERVER_UNLOCK
	uint8_t  reserved;   //!< \brief Reserved field: 0
	uint8_t  err;        //!< \brief Error code: TAS_PL_ERR_NO_ERROR, TAS_PL1_ERR_CMD_FAILED or TAS_PL1_ERR_SERVER_LOCKED

	// Add missing stuff here

} tas_pl1rsp_server_unlock_st;

//! \brief Target information.
//! \ingroup Packet_Level_1
typedef struct {
	char     identifier[TAS_NAME_LEN64];  //!< \brief Unique access HW name or IP address of device
	uint32_t device_type;   //!< \brief Device type identifier (IEEE 1149.1 device ID). 0 if no device connected
	uint32_t device_id[4];  //!< \brief 128 bit unique device ID
	uint8_t  dev_con_phys;  //!< \brief \ref tas_dev_con_phys_et
	uint8_t  num_client;    //!< \brief Number of attached clients
	uint16_t reserved;		//!< \brief Reserved field: 0
} tas_target_info_st;  // 76 bytes

//! \brief Request to get a list of targets.
//! \details Response: \ref tas_pl1rsp_get_targets_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;          	//!< \brief Request size (excluding header): 0
	uint8_t  cmd;      		//!< \brief Command identifier: TAS_PL1_CMD_GET_TARGETS
	uint8_t  start_index; 	//!< \brief Start index of targets' list. If 0 the list of all targets is sampled by the TasServer
	uint8_t  reserved;    	//!< \brief Reserved field: 0
} tas_pl1rq_get_targets_st;

//! \brief Response to \ref tas_pl1rq_get_targets_st.
//! \details The response description is followed by \ref tas_target_info_st[num_now]
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;			//!< \brief Response size (excluding header): 1...255 -> TAS_MAX_PKT_SIZE_1KB
	uint8_t  cmd;        	//!< \brief Command identifier: TAS_PL1_CMD_GET_TARGETS
	uint8_t  reserved;   	//!< \brief Reserved field: 0
	uint8_t  err;        	//!< \brief Error response: TAS_PL_ERR_NO_ERROR or TAS_PL1_ERR_SERVER_LOCKED

	uint8_t  num_target;	//!< \brief Number of targets
	uint8_t  start_index;	//!< \brief Start index of the target list
	uint8_t  num_now;		//!< \brief Current index of the target list
	uint8_t  reserved1;  	//!< \brief Reserved field: 0

	// Followed by tas_target_info_st[num_now] 

} tas_pl1rsp_get_targets_st;

//! \brief Client information.
//! \ingroup Packet_Level_1
typedef struct {
	char     client_name[TAS_NAME_LEN32];	//!< \brief Client name as a c-string 
	char     user_name[TAS_NAME_LEN16];		//!< \brief Username of a user using this client as a c-string
	uint32_t client_pid;           	//!< \brief Process ID of a client
	uint8_t  client_type;          	//!< \brief \ref tas_client_type_et
	uint8_t  reserved[3];          	//!< \brief Reserved field: 0
	uint64_t client_connect_time;  	//!< \brief Time in us since since the server start time \ref tas_server_info_st.start_time_us
	uint64_t num_byte_c2s;			//!< \brief Number of Bytes sent from a client to a server
	uint64_t num_byte_s2c;			//!< \brief Number of Bytes received by a client from a server
} tas_target_client_info_st;

//! \brief Request to get a list of connected clients.
//! \details Response: \ref tas_pl1rsp_get_clients_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;         	//!< \brief Request size (excluding header): 16 
	uint8_t  cmd;      		//!< \brief Command identifier: TAS_PL1_CMD_GET_CLIENTS
	uint8_t  start_index; 	//!< \brief If 0 the list of all clients is sampled by the TasServer
	uint8_t  reserved;    	//!< \brief Reserved field: 0

	char     identifier[TAS_NAME_LEN64];  //!< \brief Target identifier. TAS_PL_ERR_PARAM if not valid
} tas_pl1rq_get_clients_st;

//! \brief Response to \ref tas_pl1rq_get_clients_st.
//! \details The response description is followed by \ref tas_target_client_info_st[num_now]
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl; 		//!< \brief Response size (excluding header): 7...255 -> TAS_MAX_PKT_SIZE_1KB
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_GET_CLIENTS
	uint8_t  reserved;  //!< \brief Reserved field: 0
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR, TAS_PL_ERR_PARAM or TAS_PL1_ERR_SERVER_LOCKED

	uint8_t  num_client;	//!< \brief Client number
	uint8_t  start_index;	//!< \brief Start index of the client list
	uint8_t  num_now;		//!< \brief Current index of the client list
	uint8_t  reserved1;  	//!< \brief Reserved field: 0

	char     session_name[TAS_NAME_LEN16];	//!< \brief Session name and time only if start_index is 0
	uint64_t session_start_time_us;  		//!< \brief Time in microseconds since tas_server_info_st.start_time_us

	// Followed by tas_target_client_info_st[num_now] 

} tas_pl1rsp_get_clients_st;

//! \brief Request for a session start.
//! \details Response: \ref tas_pl1rsp_ping_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;       		//!< \brief Request size (excluding header): 51
	uint8_t  cmd;          	//!< \brief Command identifier: TAS_PL1_CMD_SESSION_START
	uint8_t  con_id;       	//!< \brief Connection identifier of TasDispatcher
	uint8_t  client_type;  	//!< \brief \ref tas_client_type_et
	uint8_t  param8[4];    	//!< \brief E.g. [0] for chl_target
	uint64_t param64;      	//!< \brief E.g. for chl_param
	char   identifier[TAS_NAME_LEN64];   	//!< \brief Unique access HW name or IP address of device as a c-string
	char   session_name[TAS_NAME_LEN16];	//!< \brief Session name as a c-string
	char   session_pw[TAS_NAME_LEN16];   	//!< \brief Session password as a c-string
} tas_pl1rq_session_start_st;

//! \brief Connection information provided by a device.
//! \details Only this part of connection information is provided by a TasAgent running on a device.
//! \ingroup Packet_Level_1
typedef struct {

	uint32_t max_pl2rq_pkt_size;	//!< \brief Maximum size of PL2 request packet
	uint32_t max_pl2rsp_pkt_size;	//!< \brief Maximum size if PL2 response packet

	uint32_t device_type;       	//!< \brief Device type identifier (IEEE 1149.1 device ID). 0 if no device connected
	uint32_t device_id[4];      	//!< \brief 128 bit unique device ID

	uint32_t ipv4_addr;         	//!< \brief IPV4 address of the device. 
									//! Same as identifier for TAS_DEV_CON_PHYS_ETH or copied from tas_chl_osw_config_st.

	uint16_t dev_con_feat;      	//!< \brief \ref tas_dev_con_feat_et 
	uint8_t  dev_con_phys;      	//!< \brief \ref tas_dev_con_phys_et 
	uint8_t  pl0_max_num_rw;    	//!< \brief Maximum number of RW transactions in a PL1 packet

	uint16_t pl0_rw_mode_mask;  	//!< \brief Mask for supported RW access mode bits
	uint16_t pl0_addr_map_mask; 	//!< \brief Mask for supported address maps. Bit 15 represents TAS_AM15.

	uint16_t msg_length_c2d;    	//!< \brief Maximum length of messages sent from client to device. 0 if not ClientChl or channels not supported
	uint16_t msg_length_d2c;    	//!< \brief Maximum length of messages sent from device to client. 0 if not ClientChl or channels not supported

	uint8_t  msg_num_c2d;       	//!< \brief Maximum number of messages sent from client to device
	uint8_t  msg_num_d2c;       	//!< \brief Maximum number of messages sent from device to client
	uint16_t reserved;				//!< \brief Reserved field: 0

} tas_con_info_d2s_st;

//! \brief Connection information.
//! \details Includes an extension to \ref tas_con_info_d2s_st which are only transmitted between a TasServer and a TasClient
//! \ingroup Packet_Level_1
typedef struct {
	uint32_t max_pl2rq_pkt_size; 	//!< \brief Maximum size of PL2 request packet
	uint32_t max_pl2rsp_pkt_size; 	//!< \brief Maximum size if PL2 response packet

	uint32_t device_type;       	//!< \brief Device type identifier (IEEE 1149.1 device ID). 0 if no device connected
	uint32_t device_id[4];      	//!< \brief 128 bit unique device ID

	uint32_t ipv4_addr;         	//!< \brief IPV4 address of the device. 
									//! Same as identifier for TAS_DEV_CON_PHYS_ETH or copied from tas_chl_osw_config_st.

	uint16_t dev_con_feat;      	//!< \brief tas_dev_con_feat_et 
	uint8_t  dev_con_phys;      	//!< \brief tas_dev_con_phys_et 
	uint8_t  pl0_max_num_rw;    	//!< \brief Maximum number of RW transactions in a PL1 packet

	uint16_t pl0_rw_mode_mask;  	//!< \brief Mask for supported RW access mode bits
	uint16_t pl0_addr_map_mask; 	//!< \brief Mask for supported address maps. Bit 15 represents TAS_AM15.

	uint16_t msg_length_c2d;    	//!< \brief Maximum length of messages sent from client to device. 0 if not ClientChl or channels not supported
	uint16_t msg_length_d2c;    	//!< \brief Maximum length of messages sent from device to client. 0 if not ClientChl or channels not supported

	uint8_t  msg_num_c2d;       	//!< \brief Maximum number of messages sent from client to device that will be buffered in the device
	uint8_t  msg_num_d2c;       	//!< \brief Maximum number of messages sent from device to client that will be buffered in the device
	uint16_t reserved;				//!< \brief Reserved field: 0

	// Extensions to tas_con_info_d2s_st which are only transmitted between TasServer and TasClient

	char     identifier[TAS_NAME_LEN64];  		//!< \brief Unique access HW name or IP address of device. Empty if not connected.
	uint32_t device_id_hash;    				//!< \brief 32 bit hash of device_id, calculated by TasClient
	char     device_id_hash_str[TAS_NAME_LEN8]; //!< \brief ASCII string generated with tasutil_hash32_to_str()

} tas_con_info_st;

//! \brief Request to execute a ping.
//! \details Response: \ref tas_pl1rsp_ping_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;     	//!< \brief Request size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_PING
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  reserved;  //!< \brief Reserved field: 0
} tas_pl1rq_ping_st;

//! \brief Response to \ref tas_pl1rq_ping_st.
//! \details Valid for device - server connection. Includes the connection information \ref tas_con_info_d2s_st.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;       	//!< \brief Response size (excluding header): 8
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_PING
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  err;		//!< \brief TAS_PL_ERR_NO_ERROR (check acc_hw and device type in con_info)

	uint8_t  protoc_ver_min;	//!< \brief Lowest supported protocol version
	uint8_t  protoc_ver_max;  	//!< \brief Highest supported protocol version
	uint8_t  num_instances;   	//!< \brief Number of matches for the identifier from \ref tas_pl1rq_device_connect_st
	uint8_t  reserved;        	//!< \brief Reserved field: 0

	tas_con_info_d2s_st con_info_d2s;	//!< \brief Connection information on the device - server level

} tas_pl1rsp_ping_d2s_st;

//! \brief Response to \ref tas_pl1rq_ping_st or \ref tas_pl1rq_session_start_st.
//! \details Valid for client - server connection. Includes the full connection information \ref tas_con_info_st.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;      	//!< \brief Response size (excluding header): 31
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_SESSION_START or TAS_PL1_CMD_PING
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  err;       //!< \brief TAS_PL_ERR_NO_ERROR (check acc_hw and device type in con_info)

	uint8_t  protoc_ver_min;	//!< \brief Lowest supported protocol version
	uint8_t  protoc_ver_max;  	//!< \brief Highest supported protocol version
	uint8_t  num_instances;   	//!< \brief Number of matches for the identifier from \ref tas_pl1rq_device_connect_st
	uint8_t  reserved;        	//!< \brief Reserved field: 0

	tas_con_info_st con_info;	//!< \brief Connection information

} tas_pl1rsp_ping_st;  

//! \brief Representation of \ref tas_pl1rsp_ping_st, as a response to session start.
//! \ingroup Packet_Level_1
typedef tas_pl1rsp_ping_st tas_pl1rsp_session_start_st;

//! \brief Request to get a challenge.
//! \details Response: \ref tas_pl1rsp_get_challenge_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;		//!< \brief Request size (excluding header): 1
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_GET_CHALLENGE
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  reserved;  //!< \brief Reserved field: 0

	uint16_t ulcro;     //!< \brief From where to get a challenge: \ref tas_dev_unlock_cr_option_et
	uint16_t reserved1; //!< \brief Reserved field: 0

} tas_pl1rq_get_challenge_st;

//! \brief Response to \ref tas_pl1rq_get_challenge_st.
//! \details Response description is followed by N data words for the challenge.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;        //!< \brief Response size (excluding header): 1 ... (1 + N) depending on challenge_length
	uint8_t  cmd;      	//!< \brief Command identifier: TAS_PL1_CMD_GET_CHALLENGE
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR, TAS_PL1_ERR_CMD_FAILED or TAS_PL1_ERR_DEV_ACCESS

	uint16_t challenge_length; 	//!< \brief Challenge length in bytes.
	uint16_t reserved;   		//!< \brief Reserved field: 0

	// Followed by N data words for the challenge 

} tas_pl1rsp_get_challenge_st;  

//! \brief TBD
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;      	//!< \brief Response size (excluding header): ?
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_DEVICE_UNLOCK
	uint8_t  reserved;  //!< \brief Reserved field: 0
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR, TAS_PL1_ERR_CMD_FAILED

	// Add missing stuff here

} tas_pl1rsp_device_unlock_get_challenge_st;

//! \brief Request to set a device key.
//! \details Response: \ref tas_pl1rsp_set_device_key_st. Request description is followed by N data words of a key. The key will be used if needed.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;       		//!< \brief Request size (excluding header): 1 ... (1 + N) depending on key_length
	uint8_t  cmd;       	//!< \brief Command identifier: TAS_PL1_CMD_SET_DEVICE_KEY
	uint8_t  con_id;    	//!< \brief Connection identifier of TasDispatcher
	uint8_t  reserved;  	//!< \brief Reserved field: 0

	uint16_t ulo;        	//!< \brief Unlock option \ref tas_dev_unlock_option_et
	uint16_t key_length; 	//!< \brief Key length in bytes

	// Followed by N data words for the key. The key will be used if needed 
} tas_pl1rq_set_device_key_st;

//! \brief Response to \ref tas_pl1rq_set_device_key_st.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;		//!< \brief Response size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_SET_DEVICE_KEY
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR (key will be used by following device_connect)
} tas_pl1rsp_set_device_key_st; 

//! \brief Request to establish connection to a device.
//! \details Response: \ref tas_pl1rsp_device_connect_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;		//!< \brief Request size (excluding header): 1
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_DEVICE_CONNECT
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  reserved;  //!< \brief Reserved field: 0

	uint16_t option;    //!< \brief Connection option \ref tas_dev_con_feat_et but only _RESET, _RESET_AND_HALT, _UNLOCK
	uint16_t reserved1; //!< \brief Reserved field: 0
} tas_pl1rq_device_connect_st;

//! \brief Response to \ref tas_pl1rq_device_connect_st.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;        //!< \brief Response size (excluding header): 2
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_DEVICE_CONNECT
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR, TAS_PL1_ERR_DEV_ACCESS, TAS_PL1_ERR_DEV_LOCKED or TAS_PL1_ERR_CMD_FAILED (associated feat_used bit not set)

	uint16_t feat_used; //!< \brief Connection option used \ref tas_dev_con_feat_et used (_RESET, _RESET_AND_HALT, _UNLOCK)
	uint16_t reserved;  //!< \brief Reserved field: 0

	uint32_t device_type;  //!< \brief Device type identifier (IEEE 1149.1 device ID). 0 if no device connected

} tas_pl1rsp_device_connect_st;  

//! \brief Reset counters.
//! \details The detection possibilities depend on the access path and the device type
//! The TasServer increments the value each time when a PORST or reset is detected. 
//! Please note:
//! - Not all device connections support this feature
//! - If there is no interaction with the device, only the last PORST of a 
//!   sequence of PORSTs will increment the value.
//! - The initial value can be different from 0, if another TasClient has
//!   already established a device connection.
//! \ingroup Packet_Level_1
typedef struct {
	uint32_t porst;	//!< \brief Detected power-on resets (PORST)
	uint32_t reset; //!< \brief Detected resets
} tas_reset_count_st;

//! \brief Request to read out the reset counters. 
//! \details Response: \ref tas_pl1rsp_device_reset_count_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;     	//!< \brief Request size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_DEVICE_RESET_COUNT
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  reserved;  //!< \brief Reserved field: 0
} tas_pl1rq_device_reset_count_st;

//! \brief Response to \ref tas_pl1rq_device_reset_count_st.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;        //!< \brief Request size (excluding header): 2
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_DEVICE_RESET_COUNT
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR, or TAS_PL1_ERR_CMD_FAILED

	tas_reset_count_st reset_count;	//!< \brief Reset counter value

} tas_pl1rsp_device_reset_count_st; 

// ____________________________________________________________________________________________________________________
// Channels

//! \brief Channel target
//! \details Used in \ref tas_server_info_st.supp_chl_target -> limited to 31 different targets
//! \ingroup Packet_Level_1
typedef enum {  
	TAS_CHL_TGT_UNKNOWN = 0,	//!< \brief Unknown target
	TAS_CHL_TGT_DMM     = 1,  	//!< \brief Device memory mapped protocol
} tas_chl_target_et;

//! \brief Channel type
//! \ingroup Packet_Level_1
typedef enum { 
	TAS_CHT_NONE = 0,		//!< \brief Unspecified
	TAS_CHT_SEND = 0x01,  	//!< \brief Message is sent from client to device
	TAS_CHT_RCV  = 0x02,  	//!< \brief Message is received by client from device
	TAS_CHT_BIDI = 0x03,  	//!< \brief Only use if the client always knows when to send and when to receive 
} tas_cht_et;

//! \brief channel subscribe option
//! \ingroup Packet_Level_1
typedef enum { 
	TAS_CHSO_DEFAULT = 0,		//!< \brief Default is an open channel
	TAS_CHSO_EXCLUSIVE = 0x01,	//!< \brief Exclusive channel
} tas_chso_et;

//! \brief Channel limits.
//! \ingroup Packet_Level_1
enum {
	TAS_CHL_NUM_MAX = 32,			//!< \brief Maximum number of channels
	TAS_CHL_LOWEST_PRIORITY = 31,	//!< \brief Channel number with lowest priority
};

//! \brief Request to subscribe to a channel.
//! \details Response: \ref tas_pl1rsp_chl_subscribe_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;      	//!< \brief Request size (excluding header): 1
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_CHL_SUBSCRIBE
	uint16_t reserved;  //!< \brief Reserved field: 0

	uint8_t  chl;       //!< \brief Channel number: 0 <= chl < MCD_CHL_NUM_MAX
	uint8_t  cht;       //!< \brief Channel type: \ref tas_cht_et
	uint8_t  chso;      //!< \brief Channel subscribe option: \ref tas_chso_et
	uint8_t  prio;      //!< \brief Channel priority 0 <= chl <= MCD_CHL_LOWEST_PRIORITY

} tas_pl1rq_chl_subscribe_st;

//! \brief Response to \ref tas_pl1rq_chl_subscribe_st.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;        //!< \brief Response size (excluding header): 1
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_CHL_SUBSCRIBE
	uint8_t  reserved;  //!< \brief Reserved field: 0
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR or TAS_PL1_ERR_CMD_FAILED

	uint8_t  chl;       //!< \brief Channel number: 0 <= chl < MCD_CHL_NUM_MAX
	uint8_t  cht;       //!< \brief Channel type: \ref tas_cht_et
	uint8_t  chso;      //!< \brief Channel subscribe option: \ref tas_chso_et
	uint8_t  prio; 		//!< \brief Channel priority 0 <= chl <= MCD_CHL_LOWEST_PRIORITY

} tas_pl1rsp_chl_subscribe_st;

//! \brief Request to unsubscribe from a channel.
//! \details Response: \ref tas_pl1rsp_chl_unsubscribe_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;     	//!< \brief Request size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_CHL_UNSUBSCRIBE
	uint8_t  chl;       //!< \brief Channel number: 0 <= chl < MCD_CHL_NUM_MAX
	uint8_t  reserved;  //!< \brief Reserved field: 0
} tas_pl1rq_chl_unsubscribe_st;

//! \brief Response to \ref tas_pl1rq_chl_unsubscribe_st.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;        //!< \brief Response size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_CHL_UNSUBSCRIBE
	uint8_t  chl;       //!< \brief Channel number: 0 <= chl < MCD_CHL_NUM_MAX
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR (unsubscribing an unsubscribed channel is no error)
} tas_pl1rsp_chl_unsubscribe_st;

//! \brief channel option
//! \ingroup Packet_Level_1
typedef enum {  
	TAS_CHO_NONE = 0,		//!< \brief No option specified
	TAS_CHO_INIT = 0x01,  	//!< \brief First word is the channel init value
} tas_cho_et;

//! \brief Request to send a message from a client to a device.
//! \details Client to device channel message.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;   			//!< \brief Request size (excluding header): 1
	uint8_t  cmd;        	//!< \brief Command identifier: TAS_PL1_CMD_CHL_MSG_C2D
	uint16_t reserved;   	//!< \brief Reserved field: 0

	uint8_t  chl;        	//!< \brief Channel number: 0 <= chl < MCD_CHL_NUM_MAX
	uint8_t  cho;        	//!< \brief Channel option: \ref tas_cho_et
	uint16_t msg_length; 	//!< \brief Includes the optional 4 bytes for init
} tas_pl1rq_chl_msg_c2d_st;  

//! \brief Response to \ref tas_pl1rq_chl_msg_c2d_st
//! \details Or in case of unidirectional communication also just a device to a client channel message
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;         //!< \brief Response size (excluding header): 1
	uint8_t  cmd;        //!< \brief Command identifier: TAS_PL1_CMD_CHL_MSG_D2C
	uint8_t  reserved;   //!< \brief Reserved field: 0
	uint8_t  err;        //!< \brief Erro code: TAS_PL_ERR_NO_ERROR, TAS_PL1_ERR_DEV_RESET


	uint8_t  chl;        //!< \brief Channel number: 0 <= chl < MCD_CHL_NUM_MAX
	uint8_t  cho;        //!< \brief Channel option: \ref tas_cho_et
	uint16_t msg_length; //!< \brief Includes the optional 4 bytes for init
} tas_pl1rsp_chl_msg_d2c_st;  


// ____________________________________________________________________________________________________________________
// Trace

//! \brief Trace type.
//! \details Used in \ref tas_server_info_st.supp_trc_type -> limited to 31 different trace types
//! \ingroup Packet_Level_1
typedef enum {  
	TAS_TRC_T_UNKNOWN = 0, 	//!< \brief Unknown trace type
	TAS_TRC_T_MTSC    = 1,	//!< \brief MTSC container
} tas_trc_type_et;

//! \brief Request to subscribe to a trace channel
//! \details Response: \ref tas_pl1rsp_trc_subscribe_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;		//!< \brief Request size (excluding header): 1
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_TRC_SUBSCRIBE
	uint16_t reserved;  //!< \brief Reserved field: 0
	
	uint8_t  chso;      //!< \brief Channel subscribe option: \ref tas_chso_et
	uint8_t  stream;	//!< \brief Trace stream identifier
	uint16_t reserved1; //!< \brief Reserved field: 0

} tas_pl1rq_trc_subscribe_st;

//! \brief Response to \ref tas_pl1rsp_trc_subscribe_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;        //!< \brief Response size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_TRC_SUBSCRIBE
	uint8_t  reserved;  //!< \brief Reserved field: 0
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR, TAS_PL1_ERR_CMD_FAILED or TAS_PL_ERR_NOT_SUPPORTED

	uint8_t  chso;      //!< \brief Channel subscribe option: \ref tas_chso_et
	uint8_t  stream;	//!< \brief Trace stream identifier
	uint8_t  trct;      //!< \brief Trace type: \ref tas_trc_type_et
	uint8_t  reserved1; //!< \brief Reserved field: 0

} tas_pl1rsp_trc_subscribe_st;

//! \brief Request to unsubscribe from a subscribed trace channel
//! \details Response: \ref tas_pl1rsp_trc_unsubscribe_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;        //!< \brief Request size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_TRC_UNSUBSCRIBE
	uint8_t  stream;	//!< \brief Trace stream identifier
	uint8_t  reserved;  //!< \brief Reserved field: 0
} tas_pl1rq_trc_unsubscribe_st;

//! \brief Response to \ref tas_pl1rq_trc_unsubscribe_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;        //!< \brief Response size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_TRC_UNSUBSCRIBE
	uint8_t  stream;	//!< \brief Trace stream identifier
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR (unsubscribing an unsubscribed stream is no error)
} tas_pl1rsp_trc_unsubscribe_st;

//! \brief Trace stream state
//! \ingroup Packet_Level_1
typedef enum {  
	TAS_TRCS_CONT     = 0,	//!< \brief Trace stream continued
	TAS_TRCS_SYNC     = 1,  //!< \brief Trace stream sync point for trace decoding (e.g. MTSC container)
	TAS_TRCS_START_AC = 2,  //!< \brief Trace stream start after trace hardware (re)configuration
	TAS_TRCS_START_AR = 3,  //!< \brief Trace stream start after an interruption by a reset
	TAS_TRCS_START_AI = 4,  //!< \brief Trace stream start after an interruption by a trace system specific reason
} tas_trcs_et;

//! \brief Response to TBD
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl; 		//!< \brief Response size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_TRC_DATA
	uint8_t  stream;	//!< \brief Trace stream identifier
	uint8_t  trcs;      //!< \brief Trace stream state: \ref tas_trcs_et
} tas_pl1rsp_trc_data_st;

// ____________________________________________________________________________________________________________________
// Read / Write

//! \brief Request to execute a read/write sequence. It indicates a start of PL0 packets.
//! \details Response: \ref tas_pl1rsp_pl0_start_st.
//! With \ref tas_pl1rq_pl0_end_st forms a PL1 level frame around PL0 read/write access packets.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;       			//!< \brief Request size (excluding header): 1
	uint8_t  cmd;        		//!< \brief Command identifier: TAS_PL1_CMD_PL0_START
	uint8_t  con_id;     		//!< \brief Connection identifier of TasDispatcher
	uint8_t  protoc_ver; 		//!< \brief tas_pkt.h protocol version

	uint16_t pl1_cnt;    		//!< \brief Packet level 1 count returned by \ref tas_pl1rsp_pl0_end_st
	uint16_t pl0_addr_map_mask; //!< \brief Address maps used by this PL1 packet. Bit 15 represents TAS_AM15.

} tas_pl1rq_pl0_start_st;

//! \brief Response to \ref tas_pl1rq_pl0_start_st.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;      	//!< \brief Response size (excluding header): 0
	uint8_t  cmd;    	//!< \brief Command identifier: TAS_PL1_CMD_PL0_START
	uint8_t  con_id;    //!< \brief Connection identifier of TasDispatcher
	uint8_t  err;       //!< \brief Error code: TAS_PL_ERR_NO_ERROR, TAS_PL1_ERR_DEV_ACCESS, TAS_PL1_ERR_DEV_RESET
} tas_pl1rsp_pl0_start_st; 

//! \brief Request to end a read/write sequence. It indicates the end of PL0 packets.
//! \details  Response: \ref tas_pl1rsp_pl0_end_st.
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;       		//!< \brief Request size (excluding header): 0
	uint8_t  cmd;        	//!< \brief Command identifier: TAS_PL1_CMD_PL0_END
	uint16_t num_pl0_rw; 	//!< \brief Number of PL0 RW transactions
} tas_pl1rq_pl0_end_st;

//! \brief Response to \ref tas_pl1rq_pl0_end_st
//! \ingroup Packet_Level_1
typedef struct {
	uint8_t  wl;        //!< \brief Response size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL1_CMD_PL0_END
	uint16_t pl1_cnt;  	//!< \brief Packet level 1 count from \ref tas_pl1rq_pl0_start_st
} tas_pl1rsp_pl0_end_st;


// ____________________________________________________________________________________________________________________
// PL0 packets

//! \brief PL0 request header.
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;		//!< \brief Word Length of packet excluding this first word
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL0_CMD_xxx
} tas_pl0rq_header_st;


//! \brief Request to set access mode.
//! \details Response: none (can cause a TAS_PL0_ERR_ACC_MODE for following transaction)
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;        //!< \brief Request size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL0_CMD_ACCESS_MODE
	uint16_t acc_mode;  //!< \brief Access mode: \ref tas_pl0_acc_mode_et
} tas_pl0rq_acc_mode_st;

//! \brief Request to set address map.
//! \details Response: none (can cause a TAS_PL0_ERR_ADDR_MAP for following transaction)
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;        //!< \brief Request size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL0_CMD_ADDR_MAP
	uint8_t  addr_map;  //!< \brief Address map: 0 is default for device access. Refer also to TAS_AM15 \ref tas_am15_am14.h
	uint8_t  reserved;  //!< \brief Reserved field 0
} tas_pl0rq_addr_map_st;

//! \brief Request to set 32-bit base address
//! \details Response: none
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;        //!< \brief Request size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL0_CMD_BASE_ADDR32
	uint16_t ba31to16;	//!< \brief Base address value
} tas_pl0rq_base_addr32_st;

//! \brief Request to set 64-bit base address
//! \details Response: none
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;        //!< \brief Request size (excluding header): 1
	uint8_t  cmd;       //!< \brief Command identifier TAS_PL0_CMD_BASE_ADDR64
	uint16_t ba31to16;	//!< \brief 32-bit LSB base address value

	uint32_t ba63to32;	//!< \brief 32-bit MSB base address value
} tas_pl0rq_base_addr64_st;

//! \brief Request to write data.
//! \details Response: \ref tas_pl0rsp_wr_st
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;        //!< \brief Request size (excluding header): 1
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL0_CMD_WR8, TAS_PL0_CMD_WR16, TAS_PL0_CMD_WR32
	uint16_t a15to0;	//!< \brief Address offset value which forms a full address with a base address from \ref tas_pl0rq_base_addr32_st or \ref tas_pl0rq_base_addr64_st

	uint32_t data;		//!< \brief Data to be written, up to 4 Bytes, based on the cmd
} tas_pl0rq_wr_st;

//! \brief Request to read data.
//! \details Response: \ref tas_pl0rsp_rd_st
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;        //!< \brief Request size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL0_CMD_RD8, TAS_PL0_CMD_RD16, TAS_PL0_CMD_RD32, TAS_PL0_CMD_RD64
	uint16_t a15to0;	//!< \brief Address offset value which forms a full address with a base address from \ref tas_pl0rq_base_addr32_st or \ref tas_pl0rq_base_addr64_st
} tas_pl0rq_rd_st;

//! \brief Request to write 64-bit data
//! \details Response: \ref tas_pl0rsp_wr_st
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;        //!< \brief Request size (excluding header): 2
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL0_CMD_WR64
	uint16_t a15to0;	//!< \brief Address offset value which forms a full address with a base address from \ref tas_pl0rq_base_addr32_st or \ref tas_pl0rq_base_addr64_st

	uint32_t data[2];	//!< \brief Data to be written
} tas_pl0rq_wr64_st;

//! \brief Request to write block of data
//! \details Response: \ref tas_pl0rsp_wr_st
//! Request description is followed by 1 to 256 (1KB) data words.
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;        //!< \brief Request size (excluding header): N (0 means 256)
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL0_CMD_WRBLK
	uint16_t a15to0;	//!< \brief Address offset value which forms a full address with a base address from \ref tas_pl0rq_base_addr32_st or \ref tas_pl0rq_base_addr64_st

	// Followed by 1 to 256 (1KB) data words
} tas_pl0rq_wrblk_st;

//! \brief Request to fill a memory area with provided value
//! \details Response: \ref tas_pl0rsp_wr_st
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;         	//!< \brief Request size (excluding header): 3
	uint8_t  cmd;        	//!< \brief Command identifier: TAS_PL0_CMD_FILL
	uint16_t a15to0;		//!< \brief Address offset value which forms a full address with a base address from \ref tas_pl0rq_base_addr32_st or \ref tas_pl0rq_base_addr64_st

	uint8_t  wlwr;        	//!< \brief N (0 means 256) data words to be written
	uint8_t  reserved[3];	//!< \brief Reserved field: 0

	uint64_t value;			//!< \brief Value for the fill function
} tas_pl0rq_fill_st;

//! \brief Rquest to read a block of data
//! \details Response: \ref tas_pl0rsp_rd_st or \ref tas_pl0rsp_rdblk1kb_st
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;         	//!< \brief Request size (excluding header): 1
	uint8_t  cmd;        	//!< \brief Command identifier: TAS_PL0_CMD_RDBLK
	uint16_t a15to0;		//!< \brief Address offset value which forms a full address with a base address from \ref tas_pl0rq_base_addr32_st or \ref tas_pl0rq_base_addr64_st

	uint8_t  wlrd;    		//!< \brief N (0 means 256) data words to be read
	uint8_t  reserved[3]; 	//!< \brief Reserved field: 0
} tas_pl0rq_rdblk_st;


//! \brief Packet level 0 response header
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;     	//!< \brief Response size (excluding header): 0
	uint8_t  cmd;       //!< \brief Command identifier: TAS_PL0_CMD_xxx
	uint8_t  reserved;  //!< \brief Reserved field: 0   
	uint8_t  err;       //!< \brief Error code: \ref tas_pl_err_et
} tas_pl0rsp_st;

//! \brief Response to \ref tas_pl0rq_wr_st, \ref tas_pl0rq_wr64_st, \ref tas_pl0rq_wrblk_st, and \ref tas_pl0rq_fill_st.
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;         //!< \brief Request size (excluding header): 0
	uint8_t  cmd;        //!< \brief Command identifier: TAS_PL0_CMD_WRxx
	uint8_t  wlwr;       //!< \brief 1 for wr, 2 for wr64, N for wrblk (0 means 256 if no error) data words written. 
	uint8_t  err;        //!< \brief Error code: \ref tas_pl_err_et
} tas_pl0rsp_wr_st;

//! \brief Response to \ref tas_pl0rq_rd_st, and \ref tas_pl0rq_rdblk_st.
//! \details Respons definition is followed by 0 to 255 data words. \ref tas_pl0rsp_rdblk1kb_st is used for 256 data words.
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;         //!< \brief 0 or 1|2 or N depending on err or rd8/16/32/64 or rdblk
	uint8_t  cmd;        //!< \brief Command identifier: TAS_PL0_CMD_RDxx
	uint8_t  wlrd;       //!< \brief 0 or 1|2 or N depending on err or rd8/16/32/64 or rdblk
	uint8_t  err;        //!< \brief Error code: \ref tas_pl_err_et

	// Followed by 0 to 255 data words. tas_pl0rsp_rdblk1kb_st is used for 256 data words
} tas_pl0rsp_rd_st;

//! \brief Response to \ref tas_pl0rq_rdblk_st in case of 1KB block transfer
//! \details Respons definition is followed by 256 data words (1KB)
//! \ingroup Packet_Level_0
typedef struct {
	uint8_t  wl;         //!< \brief EXCEPTION: 0 means 256 words for TAS_PL0_CMD_RDBLK1KB
	uint8_t  cmd;        //!< \brief Command identifier: TAS_PL0_CMD_RDBLK1KB
	uint8_t  wlrd;       //!< \brief 0 means 256 words
	uint8_t  err;        //!< \brief Error code: TAS_PL0_ERR_NO_ERROR

	// Followed by 256 data words for 1KB
} tas_pl0rsp_rdblk1kb_st;

#ifdef __cplusplus
}
#endif

#endif   // __tas_pkt_h
