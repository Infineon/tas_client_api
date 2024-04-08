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

#include "tas_python_client_trc.hpp"

pybind11::tuple TasClientTrc::subscribe(uint8_t stream, tas_chso_et chso){
    tas_trc_type_et trct;

    tas_return_et ret = client.subscribe(&trct, stream, chso);

    return pybind11::make_tuple(trct, ret);
}

tas_return_et TasClientTrc::unsubscribe(uint8_t stream){
    return client.unsubscribe(stream);
}

pybind11::tuple TasClientTrc::rcv_trace(uint32_t timeout_ms, uint8_t* stream){
    char* trace_data = nullptr;
    uint32_t length;
    tas_trcs_et trcs;

    tas_return_et ret = client.rcv_trace(timeout_ms, (const void**)&trace_data, &length, &trcs, stream);

    pybind11::bytes trc((char*)(*trace_data), length);

    return pybind11::make_tuple(trc, trcs, *stream, ret);
}