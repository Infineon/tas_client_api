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

#include "tas_python_client_server_con.hpp"

const char* TasClientServerCon::get_error_info(){ 
    return clientServerCon->get_error_info();
}

tas_return_et TasClientServerCon::server_connect(const char* ip_addr, uint16_t port_num){
    return clientServerCon->server_connect(ip_addr, port_num);
}

const char* TasClientServerCon::get_server_ip_addr() { 
    return clientServerCon->get_server_ip_addr(); 
}

uint16_t TasClientServerCon::get_server_port_num() { 
    return clientServerCon->get_server_port_num(); 
}

const tas_server_info_st TasClientServerCon::get_server_info() { 
    return *(clientServerCon->get_server_info()); 
}

uint64_t TasClientServerCon::get_server_challenge() { 
    return clientServerCon->get_server_challenge(); 
}

tas_return_et TasClientServerCon::server_unlock(py::bytes key){			
    char *key_c = nullptr;
    Py_ssize_t key_length = 0;

    PyBytes_AsStringAndSize(key.ptr(), &key_c, &key_length);

    return clientServerCon->server_unlock((const void*)key_c, (uint16_t)key_length);
}

py::tuple TasClientServerCon::get_targets(){
    tas_target_info_st* target_info = nullptr; 
    uint32_t num_target = 0;

    tas_return_et ret = clientServerCon->get_targets((const tas_target_info_st**)&target_info, &num_target);

    py::list targets;
    for(uint32_t i = 0; i < num_target; i++){
        targets.append(target_info[i]);
    }

    return py::make_tuple(targets, ret);
}

py::tuple TasClientServerCon::get_target_clients(const char* identifier){
    tas_target_client_info_st* target_client_info = nullptr;
    char* session_name = nullptr;
    uint64_t session_start_time_us = 0;
    uint32_t num_client = 0;

    tas_return_et ret = clientServerCon->get_target_clients(identifier, (const char**)&session_name, &session_start_time_us, (const tas_target_client_info_st**)&target_client_info, &num_client);

    py::list target_clients;

    for(uint32_t i = 0; i < num_client; i++){
        target_clients.append(target_client_info[i]);
    }

    py::str s_name(session_name);

    return py::make_tuple(target_clients, s_name, session_start_time_us, ret);
}

const tas_con_info_st TasClientServerCon::get_con_info() { 
    const tas_con_info_st *info = clientServerCon->get_con_info();

    return  *info;
}

tas_device_family_t TasClientServerCon::get_device_family(uint32_t device_type){
    return  tas_get_device_family(device_type);
}

py::tuple TasClientServerCon::device_unlock_get_challenge(tas_dev_unlock_cr_option_et ulcro){
    char* challenge = nullptr;
    uint16_t challenge_length = 0;

    tas_return_et ret = clientServerCon->device_unlock_get_challenge(ulcro, (const void**)&challenge, &challenge_length);

    py::bytes py_challenge(challenge, challenge_length);

    return py::make_tuple(py_challenge, ret);
}

tas_return_et TasClientServerCon::device_unlock_set_key(tas_dev_unlock_option_et ulo, py::bytes key){
    char *key_c = nullptr;
    Py_ssize_t key_length = 0;
    
    PyBytes_AsStringAndSize(key.ptr(), &key_c, &key_length);

    return clientServerCon->device_unlock_set_key(ulo, (const void*)key_c, (uint16_t)key_length);
}

tas_return_et TasClientServerCon::device_connect(tas_clnt_dco_et dco){
    return clientServerCon->device_connect(dco);
}

bool TasClientServerCon::device_reset_occurred(){
    return clientServerCon->device_reset_occurred();
}

TasClientServerCon::TasClientServerCon(CTasClientServerCon& c)
{
    clientServerCon = &c;
};