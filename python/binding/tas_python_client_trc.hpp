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

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <tas_client_trc.h>


class TasClientTrc
{
public:

	pybind11::tuple subscribe(uint8_t stream = 0, tas_chso_et chso = TAS_CHSO_DEFAULT);

	tas_return_et unsubscribe(uint8_t stream = 0);

	pybind11::tuple rcv_trace(uint32_t timeout_ms, uint8_t* stream = nullptr);

	TasClientTrc(){};

private:
    CTasClientTrc client;

};