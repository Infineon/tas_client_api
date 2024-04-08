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

#include "tas_python_client_rw_base.hpp"

pybind11::tuple TasClientRwBase::read8(uint64_t addr, uint8_t addr_map){
    uint8_t value;
    tas_return_et ret = clientRwBase->read8(addr, &value, addr_map);
    return pybind11::make_tuple(value, ret);
}

tas_return_et TasClientRwBase::write8(uint64_t addr, uint8_t value, uint8_t addr_map){
    return clientRwBase->write8(addr, value, addr_map);
}

pybind11::tuple TasClientRwBase::read16(uint64_t addr, uint8_t addr_map){
    uint16_t value;
    tas_return_et ret = clientRwBase->read16(addr, &value, addr_map);
    return pybind11::make_tuple(value, ret);
}

tas_return_et TasClientRwBase::write16(uint64_t addr, uint16_t value, uint8_t addr_map){
    return clientRwBase->write16(addr, value, addr_map);
}

pybind11::tuple TasClientRwBase::read32(uint64_t addr, uint8_t addr_map){
    uint32_t value;
    tas_return_et ret = clientRwBase->read32(addr, &value, addr_map);
    return pybind11::make_tuple(value, ret);
}

tas_return_et TasClientRwBase::write32(uint64_t addr, uint32_t value, uint8_t addr_map){
    return clientRwBase->write32(addr, value, addr_map);
}

pybind11::tuple TasClientRwBase::read64(uint64_t addr, uint8_t addr_map){
    uint64_t value;
    tas_return_et ret = clientRwBase->read64(addr, &value, addr_map);
    return pybind11::make_tuple(value, ret);
}

tas_return_et TasClientRwBase::write64(uint64_t addr, uint64_t value, uint8_t addr_map){
    return clientRwBase->write64(addr, value, addr_map);
}

pybind11::tuple TasClientRwBase::read(uint64_t addr, uint32_t num_bytes, uint8_t addr_map){
    uint8_t* values = new uint8_t[num_bytes];
    uint32_t num_bytes_ok;
    
    tas_return_et ret = clientRwBase->read(addr, values, num_bytes, &num_bytes_ok, addr_map);

    pybind11::bytes data((char*)values, num_bytes_ok);

    delete[] values;

    return pybind11::make_tuple(data, ret);
}

pybind11::tuple TasClientRwBase::write(uint64_t addr, pybind11::bytes data, uint8_t addr_map){
    uint32_t num_bytes_ok;

    char* c_data;
    Py_ssize_t data_length;

    PyBytes_AsStringAndSize(data.ptr(), &c_data, &data_length);

    tas_return_et ret = clientRwBase->write(addr, (const void*)c_data, (uint32_t)data_length, &num_bytes_ok, addr_map);
    return pybind11::make_tuple(num_bytes_ok, ret);
}

tas_return_et TasClientRwBase::fill32(uint64_t addr, uint32_t value, uint32_t num_bytes, uint8_t addr_map){
    return clientRwBase->fill32(addr, value, num_bytes, addr_map);
}

tas_return_et TasClientRwBase::fill64(uint64_t addr, uint64_t value, uint32_t num_bytes, uint8_t addr_map){
    return clientRwBase->fill64(addr, value, num_bytes, addr_map);
}

pybind11::tuple TasClientRwBase::execute_trans(pybind11::list &trans){
    tas_rw_trans_st* trans_a = new tas_rw_trans_st[trans.size()];

    for (int i = 0; i < trans.size(); i++)
    {
        TasRwTransaction tran = trans[i].cast<TasRwTransaction>();

        trans_a[i].addr = tran.addr;
        trans_a[i].num_bytes = tran.num_bytes;
        trans_a[i].acc_mode = 0;
        trans_a[i].addr_map = 0;
        trans_a[i].type = tran.type;

        if(tran.type == TAS_RW_TT_RD){
            trans_a[i].rdata = (void*)new char[tran.num_bytes];
        }else{    
            trans_a[i].wdata = (const void*)PyBytes_AsString(tran.data.ptr());
        }
    }

    tas_return_et ret = clientRwBase->execute_trans(trans_a, (uint32_t)trans.size());
    
    pybind11::list new_trans;

    for (int i = 0; i < trans.size(); i++)
    {
        TasRwTransaction tran = trans[i].cast<TasRwTransaction>();
        
        if(tran.type == TAS_RW_TT_RD){
            pybind11::bytes data((char*)trans_a[i].rdata, trans_a[i].num_bytes);
            tran.data = data;
            delete[] trans_a[i].rdata;
        }

        new_trans.append(tran);
    }

    delete[] trans_a;

    return pybind11::make_tuple(new_trans, ret);
}

TasClientRwBase::TasClientRwBase(CTasClientRwBase& c){
    clientRwBase = &c;
}

TasClientRwBase::TasClientRwBase(uint32_t max_rsp_size){
    clientRwBase = new CTasClientRwBase(max_rsp_size);
}