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

#include "tas_python_binding.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)


namespace py = pybind11;
using namespace pybind11::literals;


PYBIND11_MODULE(PyTAS, m) {

    m.doc() = R"pbdoc(
        TAS python interface
        -----------------------

        .. currentmodule:: PyTAS

        .. autosummary::
           :toctree: _generate

           CTasClientServerCon
           CTasClientBase
           CTasClientRwBase
           CTasClientRw
    )pbdoc";

    py::class_<tas_server_info_st>(m, "TasServerInfo")
        .def(py::init<>())
        .def_readwrite("v_minor", &tas_server_info_st::v_minor)
        .def_readwrite("v_major", &tas_server_info_st::v_major)
        .def_property("server_name",
            [](tas_server_info_st &p) -> pybind11::str {
                std::string s(p.server_name);
                py::str value = s;
                return value;
            },
            [](tas_server_info_st& p){}
        )
        .def_property("date",
            [](tas_server_info_st &p) -> pybind11::str {
                std::string s(p.date);
                py::str value = s;
                return value;
            },
            [](tas_server_info_st& p){}
        )
        .def_readwrite("supp_protoc_ver", &tas_server_info_st::supp_protoc_ver)
        .def_readwrite("supp_chl_target", &tas_server_info_st::supp_chl_target)
        .def_readwrite("supp_trc_type", &tas_server_info_st::supp_trc_type)
        .def_readwrite("start_time_us", &tas_server_info_st::start_time_us);

    py::class_<TasRwTransaction>(m, "TasRwTransaction")
        .def(py::init<>())
        .def_readwrite("addr", &TasRwTransaction::addr)
        .def_readwrite("num_bytes", &TasRwTransaction::num_bytes)
        .def_readwrite("type", &TasRwTransaction::type)
        .def_readwrite("data", &TasRwTransaction::data, py::return_value_policy::reference);

    py::class_<tas_reset_count_st>(m, "TasResetCount")
        .def(py::init<>())
        .def_readwrite("porst", &tas_reset_count_st::porst)
        .def_readwrite("reset", &tas_reset_count_st::reset);

    py::class_<tas_target_client_info_st>(m, "TasTargetClientInfo")
        .def(py::init<>())
        .def_property("client_name",
            [](tas_target_client_info_st &p) -> pybind11::str {
                std::string s(p.client_name);
                py::str value = s;
                return value;
            },
            [](tas_target_client_info_st& p){}
        )
        .def_property("user_name",
            [](tas_target_client_info_st &p) -> pybind11::str {
                std::string s(p.user_name);
                py::str value = s;
                return value;
            },
            [](tas_target_client_info_st& p){}
        )
        .def_readwrite("client_pid", &tas_target_client_info_st::client_pid)
        .def_readwrite("client_type", &tas_target_client_info_st::client_type)
        .def_property("reserved", [](tas_target_client_info_st &p) -> pybind11::array {
            auto dtype = pybind11::dtype(pybind11::format_descriptor<uint8_t>::format());
            auto base = pybind11::array(dtype, {3}, {sizeof(uint8_t)});
            return pybind11::array(
                dtype, {3}, {sizeof(uint8_t)}, p.reserved, base);
        }, [](tas_target_client_info_st& p) {})
        .def_readwrite("client_connect_time", &tas_target_client_info_st::client_connect_time)
        .def_readwrite("num_byte_c2s", &tas_target_client_info_st::num_byte_c2s)
        .def_readwrite("num_byte_s2c", &tas_target_client_info_st::num_byte_s2c);

    py::class_<tas_target_info_st>(m, "TasTargetInfo")
        .def(py::init<>())
        .def_property("identifier",
            [](tas_target_info_st &p) -> pybind11::str {
                std::string s(p.identifier);
                py::str value = s;
                return value;
            },
            [](tas_target_info_st& p){}
        )
        .def_readwrite("device_type", &tas_target_info_st::device_type)
        .def_property("device_id", [](tas_target_info_st &p) -> pybind11::array {
            auto dtype = pybind11::dtype(pybind11::format_descriptor<unsigned int>::format());
            auto base = pybind11::array(dtype, {4}, {sizeof(unsigned int)});
            return pybind11::array(
                dtype, {4}, {sizeof(unsigned int)}, p.device_id, base);
        }, [](tas_target_info_st& p) {})
        .def_readwrite("dev_con_phys", &tas_target_info_st::dev_con_phys)
        .def_readwrite("num_client", &tas_target_info_st::num_client)
        .def_readwrite("reserved", &tas_target_info_st::reserved);

    py::class_<tas_con_info_st>(m, "TasConInfo")
        .def(py::init<>())
        .def_readwrite("max_pl2rq_pkt_size", &tas_con_info_st::max_pl2rq_pkt_size)
        .def_readwrite("max_pl2rsp_pkt_size", &tas_con_info_st::max_pl2rsp_pkt_size)
        .def_readwrite("device_type", &tas_con_info_st::device_type)
        .def_property("device_id", [](tas_con_info_st &p) -> pybind11::array {
            auto dtype = pybind11::dtype(pybind11::format_descriptor<unsigned int>::format());
            auto base = pybind11::array(dtype, {4}, {sizeof(unsigned int)});
            return pybind11::array(
                dtype, {4}, {sizeof(unsigned int)}, p.device_id, base);
        }, [](tas_con_info_st& p) {})
        .def_readwrite("ipv4_addr", &tas_con_info_st::ipv4_addr)
        .def_readwrite("dev_con_feat", &tas_con_info_st::dev_con_feat)
        .def_readwrite("dev_con_phys", &tas_con_info_st::dev_con_phys)
        .def_readwrite("pl0_max_num_rw", &tas_con_info_st::pl0_max_num_rw)
        .def_readwrite("pl0_rw_mode_mask", &tas_con_info_st::pl0_rw_mode_mask)
        .def_readwrite("pl0_addr_map_mask", &tas_con_info_st::pl0_addr_map_mask)
        .def_readwrite("msg_length_c2d", &tas_con_info_st::msg_length_c2d)
        .def_readwrite("msg_length_d2c", &tas_con_info_st::msg_length_d2c)
        .def_readwrite("msg_num_c2d", &tas_con_info_st::msg_num_c2d)
        .def_readwrite("msg_num_d2c", &tas_con_info_st::msg_num_d2c)
        .def_readwrite("reserved", &tas_con_info_st::reserved)
        .def_property("identifier",
            [](tas_con_info_st &p) -> pybind11::str {
                std::string s(p.identifier);
                py::str value = s;
                return value;
            },
            [](tas_con_info_st& p){}
        )
        .def_readwrite("device_id_hash", &tas_con_info_st::device_id_hash)
        .def_property("device_id_hash_str",    
            [](tas_con_info_st &p) -> pybind11::str {
                std::string s(p.device_id_hash_str);
                py::str value = s;
                return value;
            },
            [](tas_con_info_st& p){}
        );

    py::enum_ <tas_return_et>(m, "TasReturn")
            .value("TAS_ERR_NONE", tas_return_et::TAS_ERR_NONE)
            .value("TAS_ERR_GENERAL", tas_return_et::TAS_ERR_GENERAL)
            .value("TAS_ERR_FN_USAGE", tas_return_et::TAS_ERR_FN_USAGE)
            .value("TAS_ERR_FN_PARAM", tas_return_et::TAS_ERR_FN_PARAM)
            .value("TAS_ERR_SERVER_CON", tas_return_et::TAS_ERR_SERVER_CON)
            .value("TAS_ERR_SERVER_LOCKED", tas_return_et::TAS_ERR_SERVER_LOCKED)
            .value("TAS_ERR_DEVICE_ACCESS", tas_return_et::TAS_ERR_DEVICE_ACCESS)
            .value("TAS_ERR_DEVICE_LOCKED", tas_return_et::TAS_ERR_DEVICE_LOCKED)
            .value("TAS_ERR_RW_READ", tas_return_et::TAS_ERR_RW_READ)
            .value("TAS_ERR_RW_WRITE", tas_return_et::TAS_ERR_RW_WRITE)
            .value("TAS_ERR_CHL_SETUP", tas_return_et::TAS_ERR_CHL_SETUP)
            .value("TAS_ERR_CHL_SEND", tas_return_et::TAS_ERR_CHL_SEND)
            .value("TAS_ERR_CHL_RCV", tas_return_et::TAS_ERR_CHL_RCV)
            .value("TAS_ERR_TRC_RCV", tas_return_et::TAS_ERR_TRC_RCV)
            .export_values();

    py::enum_ <TasAddrMap>(m, "TasAddrMap")
            .value("TAS_AM0", TasAddrMap::TAS_AM_0)
            .value("TAS_AM1", TasAddrMap::TAS_AM_1)
            .value("TAS_AM12", TasAddrMap::TAS_AM_12)
            .value("TAS_AM14", TasAddrMap::TAS_AM_14)
            .value("TAS_AM15", TasAddrMap::TAS_AM_15)
            .value("TAS_AM132", TasAddrMap::TAS_AM_132)
            .export_values();

    py::enum_ <tas_chl_target_et>(m, "TasChlTarget")
            .value("TAS_CHL_TGT_UNKNOWN", tas_chl_target_et::TAS_CHL_TGT_UNKNOWN)
            .value("TAS_CHL_TGT_DMM", tas_chl_target_et::TAS_CHL_TGT_DMM)
            .export_values();

    py::enum_ <tas_dev_unlock_cr_option_et>(m, "TasDevUnlockCrOption")
            .value("TAS_DEV_ULCRO_UDID", tas_dev_unlock_cr_option_et::TAS_DEV_ULCRO_UDID)
            .value("TAS_DEV_ULCRO_UDID_RST", tas_dev_unlock_cr_option_et::TAS_DEV_ULCRO_UDID_RST)
            .value("TAS_DEV_ULCRO_CR", tas_dev_unlock_cr_option_et::TAS_DEV_ULCRO_CR)
            .value("TAS_DEV_ULCRO_CR_RST", tas_dev_unlock_cr_option_et::TAS_DEV_ULCRO_CR_RST)
            .value("TAS_DEV_ULCRO_CUSTOM0", tas_dev_unlock_cr_option_et::TAS_DEV_ULCRO_CUSTOM0)
            .value("TAS_DEV_ULCRO_CUSTOM1", tas_dev_unlock_cr_option_et::TAS_DEV_ULCRO_CUSTOM1)
            .value("TAS_DEV_ULCRO_CUSTOM2", tas_dev_unlock_cr_option_et::TAS_DEV_ULCRO_CUSTOM2)
            .value("TAS_DEV_ULCRO_CUSTOM3", tas_dev_unlock_cr_option_et::TAS_DEV_ULCRO_CUSTOM3)
            .export_values();

    py::enum_ <tas_dev_unlock_option_et>(m, "TasDevUnlockOption")
            .value("TAS_DEV_ULO_HOT_ATTACH", tas_dev_unlock_option_et::TAS_DEV_ULO_HOT_ATTACH)
            .value("TAS_DEV_ULO_AFTER_RST_BY_FW", tas_dev_unlock_option_et::TAS_DEV_ULO_AFTER_RST_BY_FW)
            .value("TAS_DEV_ULO_AFTER_RST_BY_APPSW", tas_dev_unlock_option_et::TAS_DEV_ULO_AFTER_RST_BY_APPSW)
            .export_values();

    py::enum_ <tas_clnt_dco_et>(m, "TasClntDco")
            .value("TAS_CLNT_DCO_HOT_ATTACH", tas_clnt_dco_et::TAS_CLNT_DCO_HOT_ATTACH)
            .value("TAS_CLNT_DCO_RESET", tas_clnt_dco_et::TAS_CLNT_DCO_RESET)
            .value("TAS_CLNT_DCO_RESET_AND_HALT", tas_clnt_dco_et::TAS_CLNT_DCO_RESET_AND_HALT)
            .value("TAS_CLNT_DCO_UNKNOWN", tas_clnt_dco_et::TAS_CLNT_DCO_UNKNOWN, "Connect to 'unknown' device -> no device interaction")
            .value("TAS_CLNT_DCO_UNKNOWN_RESET", tas_clnt_dco_et::TAS_CLNT_DCO_UNKNOWN_RESET, "Reset 'unknown' device -> no device interaction")
            .export_values();

    py::enum_ <tas_cht_et>(m, "TasCht")
            .value("TAS_CHT_NONE", tas_cht_et::TAS_CHT_NONE)
            .value("TAS_CHT_SEND", tas_cht_et::TAS_CHT_SEND)
            .value("TAS_CHT_RCV", tas_cht_et::TAS_CHT_RCV)
            .value("TAS_CHT_BIDI", tas_cht_et::TAS_CHT_BIDI)
            .export_values();

    py::enum_ <tas_chso_et>(m, "TasChso")
            .value("TAS_CHSO_DEFAULT", tas_chso_et::TAS_CHSO_DEFAULT)
            .value("TAS_CHSO_EXCLUSIVE", tas_chso_et::TAS_CHSO_EXCLUSIVE)
            .export_values();

    /*py::enum_ <tas_trct_et>(m, "TasTrct")
            .value("TAS_TRCT_UNKNOWN", tas_trct_et::TAS_TRCT_UNKNOWN)
            .value("TAS_TRCT_MTSC", tas_trct_et::TAS_TRCT_MTSC)
            .export_values();*/

    py::enum_ <tas_rw_trans_type_et>(m, "TasRwTransType")
            .value("TAS_RW_TT_RD", tas_rw_trans_type_et::TAS_RW_TT_RD)
            .value("TAS_RW_TT_WR", tas_rw_trans_type_et::TAS_RW_TT_WR)
            .value("TAS_RW_TT_FILL", tas_rw_trans_type_et::TAS_RW_TT_FILL)
            .export_values();

    py::enum_ <tas_device_family_t>(m, "TasDeviceFamily")
        .value("TAS_DF_UNKNOWN",    tas_device_family_t::TAS_DF_UNKNOWN)
        .value("TAS_DF_TC2X",       tas_device_family_t::TAS_DF_TC2X)
        .value("TAS_DF_TC3X",       tas_device_family_t::TAS_DF_TC3X)
        .value("TAS_DF_TC4X",       tas_device_family_t::TAS_DF_TC4X)
        .value("TAS_DF_XMC1X",      tas_device_family_t::TAS_DF_XMC1X)
        .value("TAS_DF_XMC4X",      tas_device_family_t::TAS_DF_XMC4X)
        .export_values();

    py::enum_ <tas_am15_addr_et>(m, "TasAm15Addr")
        .value("TAS_AM15_R_RESET_COUNT_GET",            tas_am15_addr_et::TAS_AM15_R_RESET_COUNT_GET, "Read tas_reset_count_st of TasServer for the connected device")
        .value("TAS_AM15_RW_USERPINS",                  tas_am15_addr_et::TAS_AM15_RW_USERPINS, "Read or write user pins with a bit vector.\nIncludes input and output pins.\nread32 returns the value of input and output pins. Not existing pins are 0.\nread64 returns tas_userpins_mask_st. mask shows all available user pins.\nwrite32 updates all access HW output pins (tool to device t2d direction).\nwrite64 updates only the t2d pins, where the mask bit is set.\nThe usage of write32 is discouraged to avoid race conditions for multi-tool setups.\nCan use the tasutil_userpins_set_high()/tasutil_userpins_set_low() in simple cases.\nIf mask includes a pin which is not t2d, error TAS_ERR_RW_WRITE is returned.\nHowever other t2d pins are updated properly in this case.")
        .value("TAS_AM15_RW_USERPINS_DIR",              tas_am15_addr_et::TAS_AM15_RW_USERPINS_DIR, "Read or write the direction of the user pins with a bit vector.\nThe direction is encoded with 0 for tool to device t2d direction and 1 for d2t.\nread32 returns the current direction of pins. Not existing pins are 0.\nread64 returns tas_userpins_mask_st. mask shows the pins with direction control.\nwrite32 updates the direction of all pins with direction control.\nwrite64 updates only the pins, where the mask bit is set.\nThe usage of write32 is discouraged to avoid race conditions for multi-tool setups.\nIf mask includes a pin without direction control, error TAS_ERR_RW_WRITE is returned.\nHowever other pins with direction control are updated properly in this case.")
        .value("TAS_AM15_RW_USERPINS_CNTRL",            tas_am15_addr_et::TAS_AM15_RW_USERPINS_CNTRL, "Read or write the control settings tas_am15_userpins_cntrl_et")
        .value("TAS_AM15_RW_ACC_HW_FREQUENCY",          tas_am15_addr_et::TAS_AM15_RW_ACC_HW_FREQUENCY, "Read or write the frequency [Hz] of the device access HW clock (JTAG, DAP, SWD, etc.).\nWhen the value is written the TasServer chooses the closest possible lower value it can operate. \nIf too low the lowest supported clock frequency is set.")
        .value("TAS_AM15_RW_JTAG_SCAN_MODE_ENABLE",     tas_am15_addr_et::TAS_AM15_RW_JTAG_SCAN_MODE_ENABLE, "Enable native JTAG scan mode\nThe uint32_t value written is reserved for future use\nThe LSB of the read uint32_t value indicates that JTAG scan mode is enabled\nNote that during native JTAG scan mode regular R/W access to the device is not possible.")
        .value("TAS_AM15_W_JTAG_SCAN_MODE_DISABLE",     tas_am15_addr_et::TAS_AM15_W_JTAG_SCAN_MODE_DISABLE, "Disable native JTAG scan mode\nThe uint32_t value written is reserved for future use")
        .value("TAS_AM15_W_JTAG_SET_IR",                tas_am15_addr_et::TAS_AM15_W_JTAG_SET_IR, "Select the scan chain with the JTAG instruction register by writing tas_jtag_set_ir_st")
        .value("TAS_AM15_W_JTAG_CAPTURE",               tas_am15_addr_et::TAS_AM15_W_JTAG_CAPTURE, "Capture selected JTAG scan chain The uint32_t num_scan_bits value written is the number of scan chain bits shifted  with one or several following DAE_ARW_JTAG_SCAN_MODE_DATA_SHIFT transactions")
        .value("TAS_AM15_RW_JTAG_DATA_SHIFT",           tas_am15_addr_et::TAS_AM15_RW_JTAG_DATA_SHIFT, "The access HW will shift data in and out of the captured JTAG scan chain\nThe access HW will automatically call Update of the JTAG state machine, when\nnum_scan_bits set with DAE_AW_JTAG_SCAN_MODE_CAPTURE is reached.\nThe data has to be a multiple of 8 (num_scan_bits sets the effective bits).\nExcess bits are ignored for write and set to 0 for read.\nWriting TAS_AM15_RW_JTAG_DATA_SHIFT will start the shift operation.\nReading, to get the shifted out data is optional.\nThe read, following the write has to use the same data size.")
        .value("TAS_AM15_W_JTAG_GENERATE_CLOCK_CYCLES", tas_am15_addr_et::TAS_AM15_W_JTAG_GENERATE_CLOCK_CYCLES, "Generate additional clock cycles in RUN TEST IDLE state\nThe uint32_t value written is num_clock_cycles ")
        .value("TAS_AM15_W_JTAG_RESET",                 tas_am15_addr_et::TAS_AM15_W_JTAG_RESET, "Execute a JTAG reset with the JTAG TAP controller (not the JTAG reset pin)\nThe uint32_t value written is reserved for future use")
        .value("TAS_AM15_R_DEVICE_TYPE_ACC_HW",         tas_am15_addr_et::TAS_AM15_R_DEVICE_TYPE_ACC_HW, "Get the device_type (JTAG ID) which was read by the access HW")
        .value("TAS_AM15_RW_DEVICE_TYPE_SET",           tas_am15_addr_et::TAS_AM15_RW_DEVICE_TYPE_SET, "Set the device_type in the TasServer\nIf not 0:\n- The TasServer will operate the device according to the given device_type\n- Output drivers of the access HW will always stay enabled even if no device is present. \nIf 0, the device_type, which is automatically read by the access HW, is used.")
        .value("TAS_AM15_R_TIME_SERVER_START_US",       tas_am15_addr_et::TAS_AM15_R_TIME_SERVER_START_US, "Time of the TasServer start in micro seconds elapsed since midnight, January 1, 1970\nHas to be read as a 64 bit value")
        .value("TAS_AM15_R_TIME_SINCE_SERVER_START_US", tas_am15_addr_et::TAS_AM15_R_TIME_SINCE_SERVER_START_US, "Time in micro seconds elapsed since TasServer start\nHas to be read as a 64 bit value")
        .export_values();

    py::enum_ <tas_am15_userpins_cntrl_et>(m, "TasAm15UserpinsCntrl")
        .value("TAS_UPC_USERPINS",      tas_am15_userpins_cntrl_et::TAS_UPC_USERPINS,       "TAS_UP_ALL_UP (default)")
        .value("TAS_UPC_ADD_SFP_RESET", tas_am15_userpins_cntrl_et::TAS_UPC_ADD_SFP_RESET,  "TAS_UP_ALL_UP, TAS_UP_SFP_RESET, TAS_UP_SFP_TRSTN")
        .value("TAS_UPC_ADD_SFP_TIP",   tas_am15_userpins_cntrl_et::TAS_UPC_ADD_SFP_TIP,    "TAS_UP_ALL_UP, TAS_UP_SFP_RESET, TAS_UP_SFP_JTAG or TAS_UP_SFP_DAP")
        .export_values();

    py::enum_ <tas_am15_userpins_et>(m, "TasAm15Userpins")
        .value("TAS_UP_ALL_UP",         tas_am15_userpins_et::TAS_UP_ALL_UP,        "All potential userpins of an access HW")
        .value("TAS_UP_USR0",           tas_am15_userpins_et::TAS_UP_USR0,          "DAP connector: USR0 bidi; OCDS L1 connector: OCDSE t2d")
        .value("TAS_UP_USR1",           tas_am15_userpins_et::TAS_UP_USR1,          "DAP connector: USR1 bidi")
        .value("TAS_UP_USR8",           tas_am15_userpins_et::TAS_UP_USR8,          "On-board, FTDI access HW USR8 t2d")
        .value("TAS_UP_BRKIN",          tas_am15_userpins_et::TAS_UP_BRKIN,         "OCDS L1 connector: BRKIN  t2d ")
        .value("TAS_UP_BRKOUT",         tas_am15_userpins_et::TAS_UP_BRKOUT,        "OCDS L1 connector: BRKOUT d2t")
        .value("TAS_UP_RUNLED",         tas_am15_userpins_et::TAS_UP_RUNLED,        "Target is running LED on miniWiggler and on-board. Low active.")
        .value("TAS_UP_ALL_SFP",        tas_am15_userpins_et::TAS_UP_ALL_SFP,       "All potential special function pins of an access HW")
        .value("TAS_UP_SFP_RESET",      tas_am15_userpins_et::TAS_UP_SFP_RESET,     "Low active reset pin t2d")
        .value("TAS_UP_SFP_TRST",       tas_am15_userpins_et::TAS_UP_SFP_TRST,      "Low active JTAG TRST pin t2d")
        .value("TAS_UP_SFP_ALL_JTAG",   tas_am15_userpins_et::TAS_UP_SFP_ALL_JTAG,  "All controllable JTAG pins (TRST, TDI, TMS, TCK) t2d")
        .value("TAS_UP_SFP_TDI",        tas_am15_userpins_et::TAS_UP_SFP_TDI,       "Additional special function pin")
        .value("TAS_UP_SFP_TMS",        tas_am15_userpins_et::TAS_UP_SFP_TMS,       "Additional special function pin")
        .value("TAS_UP_SFP_TCK",        tas_am15_userpins_et::TAS_UP_SFP_TCK,       "Additional special function pin")
        .value("TAS_UP_SFP_ALL_DAP",    tas_am15_userpins_et::TAS_UP_SFP_ALL_DAP,   "All controllable DAP pins (DAP0, DAP1) t2d")
        .value("TAS_UP_SFP_DAP0",       tas_am15_userpins_et::TAS_UP_SFP_DAP0,      "DAP0 or SWDCLK")
        .value("TAS_UP_SFP_DAP1",       tas_am15_userpins_et::TAS_UP_SFP_DAP1,      "DAP1 or SWDIO")
        .export_values();

    py::class_<TasClientServerCon>(m, "TasClientServerCon")
            .def("server_connect",          &TasClientServerCon::server_connect, "Stablish connection with the server", "ip_addr"_a="localhost", "port_num"_a=24817)
            .def("get_error_info",          &TasClientServerCon::get_error_info, "Get last error message")
            .def("get_server_ip_addr",      &TasClientServerCon::get_server_ip_addr, "Gets the IP address of the server")
            .def("get_server_port_num",     &TasClientServerCon::get_server_port_num, "Gets the port number of the server")
            .def("get_server_info",         &TasClientServerCon::get_server_info, "Gets information about the server")
            //.def("get_server_challenge",  &TasClientServerCon::get_server_challenge)
            //.def("server_unlock",         &TasClientServerCon::server_unlock, "key"_a)
            .def("get_targets",             &TasClientServerCon::get_targets, "Gets the targets connected to the server")
            .def("get_target_clients",      &TasClientServerCon::get_target_clients, "Gets the clients connected to the target", "identifier"_a)
            .def("get_con_info",            &TasClientServerCon::get_con_info, "Get information about the connection with the target")
            .def("get_device_family",       &TasClientServerCon::get_device_family, "Returns the Infineon family device type from the device type number", "device_type"_a)
            //.def("device_unlock_get_challenge", &TasClientServerCon::device_unlock_get_challenge, "ulcro"_a, "challenge"_a, "challenge_length"_a)
            //.def("device_unlock_set_key", &TasClientServerCon::device_unlock_set_key, "ulo"_a, "key"_a)
            .def("device_connect",        &TasClientServerCon::device_connect, "dco"_a)
            .def("device_reset_occurred",   &TasClientServerCon::device_reset_occurred, "Checks if the device was reset");

    py::class_<TasClientRwBase>(m, "TasClientRwBase")
            .def(py::init<uint32_t>(), "max_rsp_size"_a=0x10000)
            .def("read8", &TasClientRwBase::read8, "This methods allows to read a byte from memory", "addr"_a, "addr_map"_a=0)
            .def("write8", &TasClientRwBase::write8, "This methods allows to write a byte in memory", "addr"_a, "value"_a, "addr_map"_a=0)
            .def("read16", &TasClientRwBase::read16, "This methods allows to read two bytes from memory", "addr"_a, "addr_map"_a=0)
            .def("write16", &TasClientRwBase::write16, "This methods allows to write two bytes in memory", "addr"_a, "value"_a, "addr_map"_a=0)
            .def("read32", &TasClientRwBase::read32, "This methods allows to read four bytes from memory", "addr"_a, "addr_map"_a=0)
            .def("write32", &TasClientRwBase::write32, "This methods allows to write 4 bytes in memory", "addr"_a, "value"_a, "addr_map"_a=0)
            .def("read64", &TasClientRwBase::read64, "This methods allows to read 8 bytes from memory", "addr"_a, "addr_map"_a=0)
            .def("write64", &TasClientRwBase::write64, "This methods allows to write 8 bytes in memory", "addr"_a, "value"_a, "addr_map"_a=0)
            .def("read", &TasClientRwBase::read, "This methods allows to read an specified amount of bytes from memory", "addr"_a, "num_bytes"_a, "addr_map"_a=0)
            .def("write", &TasClientRwBase::write, "This methods allows to write data in memory", "addr"_a, "data"_a, "addr_map"_a=0)
            .def("fill32", &TasClientRwBase::fill32, "This fills the specified amount of bytes with the given 32bit value", "addr"_a, "value"_a, "num_bytes"_a, "addr_map"_a=0)
            .def("fill64", &TasClientRwBase::fill64, "This fills the specified amount of bytes with the given 64bit value", "addr"_a, "value"_a, "num_bytes"_a, "addr_map"_a=0)
            .def("execute_trans", &TasClientRwBase::execute_trans, "Executes the list of read/write instructions", "trans"_a);

    py::class_<TasClientRw, TasClientRwBase, TasClientServerCon>(m, "TasClientRw")
            .def(py::init<const char *>())
            .def("session_start", &TasClientRw::session_start, "Connect to a target device", "identifier"_a, "session_id"_a="", "session_pw"_a="");

    py::class_<TasClientChl, TasClientServerCon>(m , "TasClientChl")
            .def(py::init<const char *>())
            .def("session_start", &TasClientChl::session_start, "A function to connect to a target device", "identifier"_a, "session_id"_a="", "session_pw"_a="", "chl_target"_a=(tas_chl_target_et)1, "chl_param"_a=0)
            .def("subscribe", &TasClientChl::subscribe, "Subscribe to a channel", "chl"_a, "cht"_a, "chso"_a=(tas_chso_et)0x1, "prio"_a=0)
            .def("unsubscribe", &TasClientChl::unsubscribe, "Unsubscribe from a channel")
            .def("send_msg", &TasClientChl::send_msg, "Send bytes as message", "msg"_a, "init"_a=0)
            .def("send_string", &TasClientChl::send_string, "Send a string as a message", "msg"_a, "init"_a=0)
            .def("rcv_msg", &TasClientChl::rcv_msg, "Receive message as bytes", "timeout_ms"_a)
            .def("rcv_string", &TasClientChl::rcv_string, "Receive a string as a message", "timeout_ms"_a);

    /*
        Not implemented on C++ yet
  
    py::class_<TasClientTrc>(m, "TasClientTrc")
            .def(py::init<>())
            .def("subscribe", &TasClientTrc::subscribe, "stream"_a=0, "chso"_a=(tas_chso_et)0)
            .def("unsubscribe", &TasClientTrc::unsubscribe, "stream"_a=0)
            .def("rcv_trace", &TasClientTrc::rcv_trace, "timeout_ms"_a, "stream"_a=0);
    */



#ifdef VERSION_INFO
    //m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif

}