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
#include "tas_client_chl.h"
#include "tas_utils.h"
#include "tas_device_family.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <array>

std::unique_ptr<CTasClientChl> createAndConnectChlClient(const char* client_name);

//********************************************************************************************************************
//----------------------------------------------------- Main ---------------------------------------------------------
//********************************************************************************************************************
int main(int argc, char** argv)
{
    printf("TAS CHL API demo\n");

    auto clientChlBi = createAndConnectChlClient("DemoBidirectionalClient");

    if (!clientChlBi)
        return -1;

    // Basic channel operations
    //      Bidirectional client
    uint8_t chl_num = 1;


    tasutil_assert(clientChlBi->subscribe(chl_num, tas_cht_et::TAS_CHT_BIDI));

    tasutil_assert(clientChlBi->send_msg("HELLO WORLD!", 13));

    uint32_t timeout = 100;
    const char* msg_rec;
    uint16_t msg_len;
    uint32_t init;
    tasutil_assert(clientChlBi->rcv_msg(timeout, (const void**)&msg_rec, &msg_len, &init));

    printf("Received message: %s\n", msg_rec);

    tasutil_assert(clientChlBi->unsubscribe());

    //      Single direction clients
    auto clientSnd = createAndConnectChlClient("DemoSendClient");
    auto clientRcv = createAndConnectChlClient("DemoReceiveClient");

    if (!clientRcv || !clientSnd)
        return -1;

    uint8_t prioS;
    uint8_t prioR;
    const uint32_t data_2_send = 0xBEBECAFE;
    const uint32_t* data_received;

    prioS = 1;
    prioR = 2;

    tasutil_assert(clientSnd->subscribe(2, tas_cht_et::TAS_CHT_SEND, tas_chso_et::TAS_CHSO_DEFAULT, &prioS));
    tasutil_assert(clientRcv->subscribe(2, tas_cht_et::TAS_CHT_RCV, tas_chso_et::TAS_CHSO_DEFAULT, &prioR));

    tasutil_assert(clientSnd->send_msg(&data_2_send, sizeof(data_2_send)));
    tasutil_assert(clientRcv->rcv_msg(timeout, (const void**)&data_received, &msg_len, &init));

    printf("Received data: %X", *data_received);

    tasutil_assert(clientSnd->unsubscribe());
    tasutil_assert(clientRcv->unsubscribe());

    return 0;
}

std::unique_ptr<CTasClientChl> createAndConnectChlClient(const char* client_name){
    // Create an instance of TAS Client RW 
    auto clientChl = std::make_unique<CTasClientChl>(client_name);

    
    // Connect to the server, provide IP address or localhost
    tas_return_et ret; // TAS return value 
    ret = clientChl->server_connect("localhost");
    if (ret != TAS_ERR_NONE)
    {
        printf("Failed to connect to the server, %s\n", clientChl->get_error_info());
        // other error processing code
        return nullptr;  // Fatal
    }    


    // Print the server info
    printf("\n");
    const tas_server_info_st* serverInfo = clientChl->get_server_info();
    printf("Server Info: \n");

    time_t startTime = serverInfo->start_time_us / 1000000;  // Convert to seconds
    std::array<char, 20> timeStr;
	tasutil_get_time_str(startTime, timeStr.data());
    printf("Started at %s\n", timeStr.data());    

    printf("%s V%d.%d (%s)\n", serverInfo->server_name, serverInfo->v_major, serverInfo->v_minor, serverInfo->date);

    // Print the server IP address and its port number
    printf("Server IP:port: %s:%d\n", clientChl->get_server_ip_addr(), clientChl->get_server_port_num());    
    

    // Print the list of targets connected to the server
    printf("\n");
    const tas_target_info_st* targets; // list of targets
    uint32_t numTargets; // number of connected targets, updated by the API call
    ret = clientChl->get_targets(&targets, &numTargets);
    if (ret != TAS_ERR_NONE)
    {
        printf("Failed to get the list of targets %s\n", clientChl->get_error_info());
        // other error processing code
        return nullptr;  // Fatal
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
    ret = clientChl->session_start(targets[0].identifier, "DemoSession");
    if (ret != TAS_ERR_NONE)
    {
        printf("Failed to start a session, %s\n", clientChl->get_error_info());
        // other error processing code
        return nullptr;  // Fatal
    }


    // Print con_info
    const tas_con_info_st* conInfo = clientChl->get_con_info();
    constexpr int infoStrBufSize = 4096;
    std::array<char, infoStrBufSize> infoStr;
    int s = snprintf(infoStr.data(), 16, "TARGET:\n");
    tasutil_get_con_info_str(conInfo, infoStr.data() + s);
    printf("%s", infoStr.data());
    
    return clientChl;
}