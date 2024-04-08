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

#include "tas_python_client_chl.hpp"

tas_return_et TasClientChl::session_start(const char* identifier, const char* session_id, const char* session_pw, tas_chl_target_et chl_target, uint64_t chl_param)
{
    return client.session_start(identifier, session_id, session_pw, chl_target, chl_param);
}

tas_return_et TasClientChl::subscribe(uint8_t chl, tas_cht_et cht, tas_chso_et chso, uint8_t* prio){
    return client.subscribe(chl, cht, chso, prio);
}

tas_return_et TasClientChl::unsubscribe(){
    return client.unsubscribe();
}

tas_return_et TasClientChl::send_msg(pybind11::bytes message, uint32_t init){
    char* msg;
    Py_ssize_t msg_length;

    PyBytes_AsStringAndSize(message.ptr(), &msg, &msg_length);

    return client.send_msg((const void*)msg, (uint16_t)msg_length, init);
}

tas_return_et TasClientChl::send_string(std::string message, uint32_t init){
    pybind11::bytes msg(message.c_str(), message.length()+1);
    
    return send_msg(msg, init);
}

pybind11::tuple TasClientChl::rcv_msg(uint32_t timeout_ms){
    char* message = nullptr;
    uint16_t message_length;
    uint32_t init;

    tas_return_et ret = client.rcv_msg(timeout_ms, (const void**)&message, &message_length, &init);

    pybind11::bytes msg(message, message_length);

    return pybind11::make_tuple(msg, init, ret);
}

pybind11::tuple TasClientChl::rcv_string(uint32_t timeout_ms){
    char* message = nullptr;
    uint16_t message_length;
    uint32_t init;

    tas_return_et ret = client.rcv_msg(timeout_ms, (const void**)&message, &message_length, &init);

    py::str pMessage(message, message_length);

    return pybind11::make_tuple(pMessage, init, ret);
}