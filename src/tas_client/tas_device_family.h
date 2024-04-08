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

//! \addtogroup Client_API
//! \{

// Standard includes
#include <cstdint>
#include <cassert>

//! \brief Infineon's MCU families
enum tas_device_family_t : uint16_t {
    TAS_DF_UNKNOWN = 0,     //!< \brief Unknown device family
    TAS_DF_TC1X  = 0x0110,  //!< \brief TC1x, RiderD, AudoNG, TC11, AudoMax
    TAS_DF_TC2X  = 0x0120,  //!< \brief TC2x
    TAS_DF_TC3X  = 0x0130,  //!< \brief TC3x
    TAS_DF_TC4X  = 0x0140,  //!< \brief TC4x
    TAS_DF_XMC1X = 0x0210,  //!< \brief XMC 1000 series
    TAS_DF_XMC4X = 0x0240,  //!< \brief XMC 4000 series
    TAS_DF_PSOC6 = 0x0860,  //!< \brief PSoC6
};

//! \brief List of device type identifiers (IEEE 1149.1 device ID) or JTAG IDs
enum tas_device_type_t: uint32_t {

    TAS_DT_VERSION_MASK_OUT = 0x0FFFFFFF,  //!< \brief JTAG ID version nibble masked out

    // Museum
    TAS_DT_RIDERD   = 0x00063083,   
    TAS_DT_RIDERD_A = 0x10063083,   
    TAS_DT_TC1766   = 0x000DB083,   
    TAS_DT_TC1766_B = 0x200DB083,   
    TAS_DT_TC1796   = 0x000B8083,   
    TAS_DT_TC1796_B = 0x200B8083,   

    // TC2x
    TAS_DT_TC21X = 0x00202083,      
    TAS_DT_TC21X_A = 0x10202083,    

    TAS_DT_TC22X = 0x00201083,
    TAS_DT_TC22X_A = 0x10201083,

    TAS_DT_TC23X = 0x00200083,
    TAS_DT_TC23X_A = 0x10200083,

    TAS_DT_TC26X = 0x001E8083,
    TAS_DT_TC26X_A = 0x101E8083,
    TAS_DT_TC26X_B = 0x201E8083,

    TAS_DT_TC27X = 0x001DA083,
    TAS_DT_TC2D5ED = 0x101DA083,
    TAS_DT_TC27X_A = 0x201DA083,
    TAS_DT_TC27X_B = 0x301DA083,
    TAS_DT_TC27X_C = 0x401DA083,
    TAS_DT_TC27X_D = 0x501DA083,

    TAS_DT_TC29X = 0x001E9083,
    TAS_DT_TC29X_A = 0x101E9083,
    TAS_DT_TC29X_B = 0x201E9083,

    // TC3x
    TAS_DT_TC33X   = 0x0020B083,
    TAS_DT_TC33X_A = 0x1020B083,

    TAS_DT_TC33XE   = 0x0020C083,
    TAS_DT_TC33XE_A = 0x1020C083,

    TAS_DT_TC35X   = 0x0020A083,
    TAS_DT_TC35X_A = 0x1020A083,

    TAS_DT_TC36X   = 0x00209083,
    TAS_DT_TC36X_A = 0x10209083,

    TAS_DT_TC37X   = 0x00207083,
    TAS_DT_TC37X_A = 0x10207083,

    TAS_DT_TC37XE   = 0x00208083,
    TAS_DT_TC37XE_A = 0x10208083,

    TAS_DT_TC38X   = 0x00206083,
    TAS_DT_TC38X_A = 0x10206083,

    TAS_DT_TC3EX   = 0x00215083,
    TAS_DT_TC3EX_A = 0x10215083,

    TAS_DT_TC39X   = 0x00205083,
    TAS_DT_TC39X_A = 0x10205083,
    TAS_DT_TC39X_B = 0x20205083,

    TAS_DT_DAPEA2G = 0x0000DA9E,  // A2G DAPE connection

    // TC4x
    TAS_DT_TC41X   = 0x0021B083,
    TAS_DT_TC41X_A = 0x1021B083,

    TAS_DT_TC42X   = 0x00219083,
    TAS_DT_TC42X_A = 0x10219083,

    TAS_DT_TC44X   = 0x0021A083,
    TAS_DT_TC44X_A = 0x1021A083,

    TAS_DT_TC45X   = 0x00226083,
    TAS_DT_TC45X_A = 0x10226083,

    TAS_DT_TC46X   = 0x0021C083,
    TAS_DT_TC46X_A = 0x1021C083,

    TAS_DT_TC48X   = 0x0021D083,
    TAS_DT_TC48X_A = 0x1021D083,

    TAS_DT_TC49AA  = 0x0021E083,  // TC49xA
    TAS_DT_TC49AAA = 0x1021E083,

    TAS_DT_TC49X   = 0x0022B083,  // TC49xN
    TAS_DT_TC49X_A = 0x1022B083,

    TAS_DT_TC4DX   = 0x00225083,
    TAS_DT_TC4DX_A = 0x10225083,

    TAS_DT_TC4RX   = 0x00223083,
    TAS_DT_TC4RX_A = 0x10223083,

    // XMC1000
    TAS_DT_XMC1000   = 0x001ED083,
    TAS_DT_XMC1000_A = 0x101ED083,

    TAS_DT_XMC1400   = 0x00204083,
    TAS_DT_XMC1400_A = 0x10204083,

    // XMC4000
    TAS_DT_XMC4100   = 0x001DE083,
    TAS_DT_XMC4100_A = 0x101DE083,

    TAS_DT_XMC4200   = 0x001DD083,
    TAS_DT_XMC4200_A = 0x101DD083,

    TAS_DT_XMC4400   = 0x001DC083,
    TAS_DT_XMC4400_A = 0x101DC083,

    TAS_DT_XMC4500     = 0x001DB083,
    TAS_DT_XMC4500_A11 = 0x001DB083,  // A11 JTAG
    TAS_DT_XMC4500_A13 = 0x101DB083,  // A13 JTAG/SWD + A11 SWD!
    TAS_DT_XMC4500_A14 = 0x301DB083,  // A14

    TAS_DT_XMC4800     = 0x001DF083,
    TAS_DT_XMC4800_A   = 0x101DF083,

    // PSOC
    TAS_DT_PSOC62   = 0x00102069,  // CY8C62x PSOC 6
    TAS_DT_PSOC62_A = 0x10102069,

};

//! \brief Conversion from device type (JTAG ID) to a string 
//! \param device_type device type or JTAG ID
//! \returns pointer to a c-string containing string representation of device type
inline const char* tas_get_device_name_str(uint32_t device_type)
{
    switch (device_type & TAS_DT_VERSION_MASK_OUT) {

    case 0:             return "no device";

    case TAS_DT_TC33X:  return "TC33x";
    case TAS_DT_TC33XE: return "TC33xE";
    case TAS_DT_TC35X:  return "TC35x";
    case TAS_DT_TC36X:  return "TC36x";
    case TAS_DT_TC37X:  return "TC37x";
    case TAS_DT_TC37XE: return "TC37xE";
    case TAS_DT_TC38X:  return "TC38x";
    case TAS_DT_TC39X:
        return (device_type == TAS_DT_TC39X_A) ? "TC39x_A" : "TC39x";
    case TAS_DT_TC3EX:  return "TC3Ex";
    case TAS_DT_DAPEA2G: return "TC3x DAPE";

    case TAS_DT_TC41X:  return "TC41x";
    case TAS_DT_TC42X:  return "TC42x";
    case TAS_DT_TC44X:  return "TC44x";
    case TAS_DT_TC45X:  return "TC45x";
    case TAS_DT_TC46X:  return "TC46x";
    case TAS_DT_TC48X:  return "TC48x";
    case TAS_DT_TC49AA: return "TC49xA";
    case TAS_DT_TC49X:  return "TC49x";
    case TAS_DT_TC4DX:  return "TC4Dx";
    case TAS_DT_TC4RX:  return "TC4RxA";

    case TAS_DT_TC21X:  return "TC21x";
    case TAS_DT_TC22X:  return "TC22x";
    case TAS_DT_TC23X:  return "TC23x";
    case TAS_DT_TC26X:  return "TC26x";
    case TAS_DT_TC27X:  return "TC27x";
    case TAS_DT_TC29X:  return "TC29x";

    case TAS_DT_RIDERD: return "RiderD";
    case TAS_DT_TC1766: return "TC1766";
    case TAS_DT_TC1796: return "TC1796";

    case TAS_DT_XMC1000:  return "XMC1000";
    case TAS_DT_XMC1400:  return "XMC1400";
    case TAS_DT_XMC4100:  return "XMC4100";
    case TAS_DT_XMC4200:  return "XMC4200";
    case TAS_DT_XMC4400:  return "XMC4400";
    case TAS_DT_XMC4500:  return "XMC4500";
    case TAS_DT_XMC4800:  return "XMC4800";

    case TAS_DT_PSOC62:  return "PSoC62";

    default: return "UNKNOWN";
    }
}

//! \brief Performs a check if device type falls into TC1x family
//! \returns \c true if yes, otherwise \c false
inline bool tas_df_check_if_tc1x(uint32_t device_type)
{
    switch (device_type & TAS_DT_VERSION_MASK_OUT) {
    case TAS_DT_RIDERD:
    case TAS_DT_TC1766:
    case TAS_DT_TC1796:
        return true;
    default:
        return false;
    }
}

//! \brief Performs a check if device type falls into TC2x family
//! \returns \c true if yes, otherwise \c false
inline bool tas_df_check_if_tc2x(uint32_t device_type)
{
    switch (device_type & TAS_DT_VERSION_MASK_OUT) {
    case TAS_DT_TC21X:
    case TAS_DT_TC22X:
    case TAS_DT_TC23X:
    case TAS_DT_TC26X:
    case TAS_DT_TC27X:
    case TAS_DT_TC29X:
        return true;
    default:
        return false;
    }
}

//! \brief Performs a check if device type falls into TC3x family
//! \returns \c true if yes, otherwise \c false
inline bool tas_df_check_if_tc3x(uint32_t device_type)
{
    switch (device_type & TAS_DT_VERSION_MASK_OUT) {
    case TAS_DT_TC33X:
    case TAS_DT_TC33XE:
    case TAS_DT_TC35X:
    case TAS_DT_TC36X:
    case TAS_DT_TC37X:
    case TAS_DT_TC37XE:
    case TAS_DT_TC38X:
    case TAS_DT_TC3EX:
    case TAS_DT_TC39X:
    case TAS_DT_DAPEA2G:
        return true;
    default:
        return false;
    }
}

//! \brief Performs a check if device type falls into TC4x family
//! \returns \c true if yes, otherwise \c false
inline bool tas_df_check_if_tc4x(uint32_t device_type)
{
    switch (device_type & TAS_DT_VERSION_MASK_OUT) {
    case TAS_DT_TC41X:
    case TAS_DT_TC42X:
    case TAS_DT_TC44X:
    case TAS_DT_TC45X:
    case TAS_DT_TC46X:
    case TAS_DT_TC48X:
    case TAS_DT_TC49AA:
    case TAS_DT_TC49X:
    case TAS_DT_TC4DX:
    case TAS_DT_TC4RX:
        return true;
    default:
        return false;
    }
}

//! \brief Performs a check if device type falls into XMC 1000 family
//! \returns \c true if yes, otherwise \c false
inline bool tas_df_check_if_xmc1x(uint32_t device_type)
{
    switch (device_type & TAS_DT_VERSION_MASK_OUT) {
    case TAS_DT_XMC1000:
    case TAS_DT_XMC1400:
        return true;
    default:
        return false;
    }
}

//! \brief Performs a check if device type falls into XMC 4000 family
//! \returns \c true if yes, otherwise \c false
inline bool tas_df_check_if_xmc4x(uint32_t device_type)
{
    switch (device_type & TAS_DT_VERSION_MASK_OUT) {
    case TAS_DT_XMC4100:
    case TAS_DT_XMC4200:
    case TAS_DT_XMC4400:
    case TAS_DT_XMC4500:
    case TAS_DT_XMC4800:
        return true;
    default:
        return false;
    }
}

//! \brief Performs a check if device type falls into PSoC6 family
//! \returns \c true if yes, otherwise \c false
inline bool tas_df_check_if_psoc6(uint32_t device_type)
{
    switch (device_type & TAS_DT_VERSION_MASK_OUT) {
    case TAS_DT_PSOC62:
        return true;
    default:
        return false;
    }
}

//! \brief Get device family based on device type
//! \returns device family identifier 
inline tas_device_family_t tas_get_device_family(uint32_t device_type)
{
    if (tas_df_check_if_tc4x(device_type))
        return TAS_DF_TC4X;
    if (tas_df_check_if_tc3x(device_type))
        return TAS_DF_TC3X;
    if (tas_df_check_if_tc2x(device_type))
        return TAS_DF_TC2X;
    if (tas_df_check_if_tc1x(device_type))
        return TAS_DF_TC1X;
    if (tas_df_check_if_xmc4x(device_type))
        return TAS_DF_XMC4X;
    if (tas_df_check_if_xmc1x(device_type))
        return TAS_DF_XMC1X;
    if (tas_df_check_if_psoc6(device_type))
        return TAS_DF_PSOC6;
    return TAS_DF_UNKNOWN;
}

//! \brief Performs a check if device family is an AURIX family
//! \returns \c true if yes, otherwise \c false
inline bool tas_device_family_is_aurix(tas_device_family_t device_family)
{
    return ((device_family == TAS_DF_TC4X) ||
            (device_family == TAS_DF_TC3X) ||
            (device_family == TAS_DF_TC2X));
}

//! \brief Performs a check if device type falls into an AURIX family
//! \returns \c true if yes, otherwise \c false
inline bool tas_device_family_is_aurix(uint32_t device_type)
{
    return tas_device_family_is_aurix(tas_get_device_family(device_type));
}

//! \brief Conversion from device fmaily to a string 
//! \param device_family device family
//! \returns pointer to a c-string containing string representation of device family
inline const char* tas_get_device_family_str(tas_device_family_t device_family)
{
    switch (device_family) {
    case TAS_DF_TC4X:  return "TC4x";
    case TAS_DF_TC3X:  return "TC3x";
    case TAS_DF_TC2X:  return "TC2x";
    case TAS_DF_TC1X:  return "TC1x";
    case TAS_DF_XMC4X: return "XMC4000";
    case TAS_DF_XMC1X: return "XMC1000";
    case TAS_DF_PSOC6: return "PSoC6";
    default: assert(false); return "UNKNOWN";
    }
}

//! \} // end of group Client_API