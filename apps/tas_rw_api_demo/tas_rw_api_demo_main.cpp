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

//********************************************************************************************************************
//------------------------------------------------------Includes------------------------------------------------------
//********************************************************************************************************************
#include "tas_client_rw.h"
#include "tas_utils.h"
#include "tas_device_family.h"

#include <cstdio>
#include <cstring>
#include <cstddef>
#include <array>

//********************************************************************************************************************
//----------------------------------------------------- Main ---------------------------------------------------------
//********************************************************************************************************************
int main(int argc, char** argv)
{
    printf("TAS API demo\n");

    // Create an instance of TAS Client RW 
    CTasClientRw clientRw("DemoClientRw");

    
    // Connect to the server, provide IP address or localhost
    tas_return_et ret; // TAS return value 
    ret = clientRw.server_connect("localhost");
    if (ret != TAS_ERR_NONE)
    {
        printf("Failed to connect to the server, %s\n", clientRw.get_error_info());
        // other error processing code
        return -1;  // Fatal
    }    


    // Print the server info
    printf("\n");
    const tas_server_info_st* serverInfo = clientRw.get_server_info();
    printf("Server Info: \n");

    time_t startTime = serverInfo->start_time_us / 1000000;  // Convert to seconds
    std::array<char, 20> timeStr;
	tasutil_get_time_str(startTime, timeStr.data());
    printf("Started at %s\n", timeStr.data());    

    printf("%s V%d.%d (%s)\n", serverInfo->server_name, serverInfo->v_major, serverInfo->v_minor, serverInfo->date);

    // Print the server IP address and its port number
    printf("Server IP:port: %s:%d\n", clientRw.get_server_ip_addr(), clientRw.get_server_port_num());    
    

    // Print the list of targets connected to the server
    printf("\n");
    const tas_target_info_st* targets; // list of targets
    uint32_t numTargets; // number of connected targets, updated by the API call
    ret = clientRw.get_targets(&targets, &numTargets);
    if (ret != TAS_ERR_NONE)
    {
        printf("Failed to get the list of targets %s\n", clientRw.get_error_info());
        // other error processing code
        return -1;  // Fatal
    }
    printf("Number of targets: %d\n", numTargets);    
    for (uint32_t i = 0; i < numTargets; i++)
    {
        const char* deviceName = tas_get_device_name_str(targets[i].device_type);
        uint32_t deviceIdHash = tasutil_crc32(targets[i].device_id, 16);
        std::array<char, TASUTIL_HASH32_STRBUF_SIZE> deviceIdHashStr;
        tasutil_hash32_to_str(deviceIdHash, deviceIdHashStr.data());
        printf("Target %d: %s %s (%s)\n", i, deviceName, deviceIdHashStr.data(), targets[i].identifier);
    }


    // Connect to the first target from the list
    printf("\n");
    printf("Connecting to the first target from the list...\n");
    ret = clientRw.session_start(targets[0].identifier, "DemoSession");
    if (ret != TAS_ERR_NONE)
    {
        printf("Failed to start a session, %s\n", clientRw.get_error_info());
        // other error processing code
        return -1;  // Fatal
    }


    // Print con_info
    const tas_con_info_st* conInfo = clientRw.get_con_info();
    constexpr int infoStrBufSize = 4096;
    std::array<char, infoStrBufSize> infoStr;
    int s = snprintf(infoStr.data(), 16, "TARGET:\n");
    tasutil_get_con_info_str(conInfo, infoStr.data() + s);
    printf("%s", infoStr.data());


    // Reset the target
    ret = clientRw.device_connect(TAS_CLNT_DCO_RESET_AND_HALT);
    if (ret != TAS_ERR_NONE)
    {
        printf("Failed to reset the device, %s\n", clientRw.get_error_info());
        // other error processing code
    }


    constexpr size_t BUF_SIZE = 64;
    std::array<uint32_t, BUF_SIZE> rdData;
    std::array<uint32_t, BUF_SIZE> wrData;
    uint32_t addr;
    uint8_t value8;
    uint8_t rdData8;

    const uint32_t baseAddr = 0x70000000;
    if (!tas_device_family_is_aurix(conInfo->device_type))
    {
        printf("The address 0x%8.8X is for AURIX devices. Please modify for other devices.\n", baseAddr);
        // other error processing code
        return -1;  // Fatal
    }

    // Basic read/write operations
    printf("\n");
    printf("Basic read/write operations\n");    
    addr = baseAddr;
    printf("\tRead - write - read 1 Byte at address 0x%08X\n", addr);    
    tasutil_assert(clientRw.read8(addr, &rdData8));
    printf("Read data: %02X\n", rdData8);    
    value8 = 0xAB;
    printf("Write 0x%02X\n", value8);
    tasutil_assert(clientRw.write8(addr, value8));
    tasutil_assert(clientRw.read8(addr, &rdData8));
    printf("Read back data: %02X\n", rdData8);

    printf("\n");
    addr = baseAddr + 1;
    printf("\tRead - write - read 1 Byte at address 0x%08X\n", addr);    
    tasutil_assert(clientRw.read8(addr, &rdData8));
    printf("Read data: %02X\n", rdData8);    
    value8 = 0xCD;
    printf("Write 0x%02X\n", value8);
    tasutil_assert(clientRw.write8(addr, value8));
    tasutil_assert(clientRw.read8(addr, &rdData8));
    printf("Read back data: %02X\n", rdData8);

    // Reset the target
    tasutil_assert(clientRw.device_connect(TAS_CLNT_DCO_RESET_AND_HALT));

    printf("\n");
    addr = baseAddr;
    printf("\tRead - write - read 4 Bytes at address 0x%08X\n", addr);
    tasutil_assert(clientRw.read32(addr, rdData.data()));
    printf("Read data: %08X\n", rdData[0]);    
    wrData[0] = 0xABCDEF09;
    printf("Write 0x%08X\n", wrData[0]);
    tasutil_assert(clientRw.write32(addr, wrData[0]));
    tasutil_assert(clientRw.read32(addr, rdData.data()));
    printf("Read back data: %08X\n", rdData[0]);

    printf("\n");
    addr = baseAddr + 1;
    printf("\tRead - write - read 4 Bytes at unaligned address 0x%08X\n", addr);
    tasutil_assert(clientRw.read32(addr, rdData.data()));
    printf("Read data: %08X\n", rdData[0]);    
    wrData[0] = 0x12345678;
    printf("Write 0x%08X\n", wrData[0]);
    tasutil_assert(clientRw.write32(addr, wrData[0]));
    tasutil_assert(clientRw.read32((addr & 0xFFFFFFFE), &rdData[0]));
    tasutil_assert(clientRw.read32((addr & 0xFFFFFFFE) + 4, &rdData[1]));
    printf("Read back data @0x%08X: %08X\n", (addr & 0xFFFFFFFE), rdData[0]);
    printf("Read back data @0x%08X: %08X\n", (addr & 0xFFFFFFFE) + 4, rdData[1]);

    // Reset the target
    tasutil_assert(clientRw.device_connect(TAS_CLNT_DCO_RESET_AND_HALT));

    // Example fill 
    printf("\n");
    printf("Fill data starting from target address\n");
    addr = baseAddr;
    uint32_t numBytesOk;
    // fill the memory with 0x123ABCD from 0x60000000 to 0x6000007C
    tasutil_assert(clientRw.fill32(addr, 0x1234ABCD, 128));
    tasutil_assert(clientRw.read(addr, rdData.data(), 128, &numBytesOk));
    for(uint32_t i = 0; i < (128 / 4); i++)
    {
        printf("Data @0x%08X: %08X\n", addr | (i*4), rdData[i]);
    }

    // Reset the target
    tasutil_assert(clientRw.device_connect(TAS_CLNT_DCO_RESET_AND_HALT));

    // Example with transaction list
    printf("\n");
    printf("Read - write - read with transaction list\n");
    memset(rdData.data(), 0, BUF_SIZE * sizeof(rdData[0]));
    uint16_t accMode = 0;
    uint8_t  addrMap = 0;

    addr = 0x60000000;
    wrData[0] = 0xFFABCDEF;
    const uint8_t numTrans = 3;
    // build a list of transactions
    tas_rw_trans_st trans[numTrans] = {
        {addr, 4, accMode, addrMap, TAS_RW_TT_RD, &rdData[0] },
        {addr, 4, accMode, addrMap, TAS_RW_TT_WR, &wrData[0] },
        {addr, 4, accMode, addrMap, TAS_RW_TT_RD, &rdData[1] },
    };

    // Execute list
    tasutil_assert(clientRw.execute_trans(trans, numTrans));
    printf("Read transaction:  %08X\n", rdData[0]);
    printf("Write transaction: %08X\n", wrData[0]);
    printf("Read transaction:  %08X\n", rdData[1]);

    // Destructor of clientRw will automatically end the session

    return 0;
}