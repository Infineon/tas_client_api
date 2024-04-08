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
// TAS includes
#include "tas_pkt_handler_rw.h"
#include "tas_pkt.h"
#include "tas_am15_am14.h"

// Standard includes
#include <cassert>
#include <cstdio>
#include <cinttypes>
#include <memory>
#include <array>

CTasPktHandlerRw::CTasPktHandlerRw(tas_error_info_st* ei, const tas_con_info_st* con_info)
    : CTasPktHandlerBase(ei)
{
    mInit(PKT_BUF_SIZE_DEFAULT, PKT_BUF_SIZE_DEFAULT, con_info->pl0_max_num_rw);
    assert(mConInfo.max_pl2rq_pkt_size >= con_info->max_pl2rq_pkt_size);
    assert(mConInfo.max_pl2rsp_pkt_size >= con_info->max_pl2rsp_pkt_size);
    memcpy(&mConInfo, con_info, sizeof(tas_con_info_st));
}

CTasPktHandlerRw::CTasPktHandlerRw(tas_error_info_st* ei, uint32_t max_rq_size, uint32_t max_rsp_size, uint32_t max_num_rw)
    : CTasPktHandlerBase(ei)
{
    mInit(max_rq_size, max_rsp_size, max_num_rw);
}

void CTasPktHandlerRw::mInit(uint32_t max_rq_size, uint32_t max_rsp_size, uint32_t max_num_rw)
{
    assert(max_rq_size % 4 == 0);
    assert(max_rsp_size % 4 == 0);
    assert(max_rq_size >= 4 * BUF_ALLOWANCE);  // There is no reason not to be generous
    assert(max_rsp_size >= 4 * BUF_ALLOWANCE);  // There is no reason not to be generous

    // From CTasPktHandlerBase, allocated and set here in the derived class:
    mRqBuf = new uint32_t[max_rq_size / 4];
    mMaxRqSize = max_rq_size - BUF_ALLOWANCE;
    mRqWiMax = mMaxRqSize / 4;
    mMaxRspSize = max_rsp_size - BUF_ALLOWANCE;

    mConInfo.max_pl2rq_pkt_size = mMaxRqSize;
    mConInfo.max_pl2rsp_pkt_size = mMaxRspSize;
    mConInfo.pl0_max_num_rw = max_num_rw;

    mNumTransMax = max_num_rw;
    mRwTrans = new tas_rw_trans_st[mNumTransMax];
    mRwTransRsp = new tas_rw_trans_rsp_st[mNumTransMax];
    mPl0Trans = new tas_rw_trans_st[mNumTransMax];
    mPl0TransRsp = new tas_rw_trans_rsp_st[mNumTransMax];

    mRqBufWi = 0;
    mPl0NumTrans = 0;
    mPl2NumTrans = 0;
    mRwNumTrans = 0;

    mRspSize = 0;
    mPl2RspPktStart = 0;

    mPl0AccMode = 0;
    mPl0AddrMap = 0;
    mPl0BaseAddr = 0;

    mGetPktRqWasCalled = false;

    mDeviceResetCount = 0;
}

CTasPktHandlerRw::~CTasPktHandlerRw()
{
    delete[] mRqBuf;
    delete[] mRwTrans;
    delete[] mRwTransRsp;
    delete[] mPl0Trans;
    delete[] mPl0TransRsp;
}

void CTasPktHandlerRw::rw_start()
{
    mNumPl2Pkt = 0;

    mPl0AccMode = 0;
    mPl0AddrMap = 0;
    mPl0AddrMapMask = 0;

    mPl0BaseAddr = 0;

    mPl0NumTrans = 0;
    mRwNumTrans = 0;

    mRqBufWi = 0;

    mRspSize = 0;  // Updated in mPl2PktInit()
    mPl2PktInit();

    mPl2RspPktStart = 0;

    mMaxWrDataBlkSizeInPktRq  = mConInfo.max_pl2rq_pkt_size  - 24;  // For blk
    if (mMaxWrDataBlkSizeInPktRq > TAS_PL0_DATA_BLK_SIZE)
        mMaxWrDataBlkSizeInPktRq = TAS_PL0_DATA_BLK_SIZE;
    mMaxRdDataBlkSizeInPktRsp = mConInfo.max_pl2rsp_pkt_size - 24;  // For blk
    if (mMaxRdDataBlkSizeInPktRsp > TAS_PL0_DATA_BLK_SIZE)
        mMaxRdDataBlkSizeInPktRsp = TAS_PL0_DATA_BLK_SIZE;

    mPl1CntOutstandingOldest = mPl1CntOutstandingLast + 1; 
    mGetPktRqWasCalled = false;
}

void CTasPktHandlerRw::mPktAdd_SetAddrMapAccModeBaseAddr(uint8_t addr_map, uint16_t acc_mode, uint64_t addr)
{
    uint16_t addrMapMask = 1 << addr_map;
    if (mPl0AddrMapMask == 0) {  // First transaction in PL1 packet
        assert(mPl0AddrMap == TAS_AM0);  // TAS_AM0 default at start of PL1 packet
    }
    mPl0AddrMapMask |= addrMapMask;
    assert(mCheckAddrMapRulesInPkt(addr_map));

    // addr_map needs to be first since it will route the packet in the server
    if (addr_map != mPl0AddrMap) {
        auto pl0AddrMap = (tas_pl0rq_addr_map_st*)&mRqBuf[mRqBufWi];
        pl0AddrMap->wl = 0;
        pl0AddrMap->cmd = TAS_PL0_CMD_ADDR_MAP;
        pl0AddrMap->addr_map = addr_map;
        pl0AddrMap->reserved = 0;
        mPl0AddrMap = addr_map;
        mRqBufWi += 1;
        // mPktRspSize += 0; since no response
    }

    if (acc_mode != mPl0AccMode) {
        auto pl0AccMode = (tas_pl0rq_acc_mode_st*)&mRqBuf[mRqBufWi];
        pl0AccMode->wl = 0;
        pl0AccMode->cmd = TAS_PL0_CMD_ACCESS_MODE;
        pl0AccMode->acc_mode = acc_mode;
        mPl0AccMode = acc_mode;
        mRqBufWi += 1;
        // mPktRspSize += 0; since no response
    }

    if ((addr < mPl0BaseAddr) || (addr > (mPl0BaseAddr + 0xFFFF))) {
        mPl0BaseAddr = addr & 0xFFFFFFFFFFFF0000;
        if (addr < 0x100000000LL) {
            auto pl0Ba32 = (tas_pl0rq_base_addr32_st*)&mRqBuf[mRqBufWi];
            pl0Ba32->wl = 0;
            pl0Ba32->cmd = TAS_PL0_CMD_BASE_ADDR32;
            pl0Ba32->ba31to16 = (uint16_t)(addr >> 16);
            mRqBufWi += 1;
            // mPktRspSize += 0; since no response
        }
        else {
            auto pl0Ba64 = (tas_pl0rq_base_addr64_st*)&mRqBuf[mRqBufWi];
            pl0Ba64->wl = 1;
            pl0Ba64->cmd = TAS_PL0_CMD_BASE_ADDR64;
            pl0Ba64->ba31to16 = (uint16_t)(addr >> 16);
            pl0Ba64->ba63to32 = (uint32_t)(addr >> 32);
            mRqBufWi += 2;
            // mPktRspSize += 0; since no response
        }
    }
}

void tphrCheckNumBytes(uint32_t num_bytes)
{
    switch (num_bytes) {
    case 0:
    case 3:
    case 5:
    case 6:
    case 7: assert(false);
    default: break;
    }
    if (((num_bytes > 8) && (num_bytes % 4 != 0)) || (num_bytes > TAS_PL0_DATA_BLK_SIZE)) {
        assert(false);
    }
}

void CTasPktHandlerRw::mPktAdd_Rd(uint64_t addr, uint32_t num_bytes, void* data, uint16_t acc_mode, uint8_t addr_map)
{
    if (num_bytes == 0)  // Simplifies calling function
        return;

    tphrCheckNumBytes(num_bytes);

    if (mPl2NumTrans >= mConInfo.pl0_max_num_rw) {
        assert(mPl2NumTrans == mConInfo.pl0_max_num_rw);
        mPktFinalize();  // Start a new PL2
    }

    uint32_t numBytesAmAmBa = mGetNumBytesAddrMapAccModeBaseAddr(addr_map, acc_mode, addr);
    uint32_t numBytesNeededPktrq  = numBytesAmAmBa + sizeof(tas_pl0rq_rdblk_st) + sizeof(tas_pl1rq_pl0_end_st);
    uint32_t numBytesNeededPktrsp = sizeof(tas_pl0rsp_rd_st) + (((num_bytes + 3)/4)*4) + sizeof(tas_pl1rsp_pl0_end_st);
    mPktFinalizeIfNeeded(numBytesNeededPktrq, numBytesNeededPktrsp);

    mPktAdd_SetAddrMapAccModeBaseAddr(addr_map, acc_mode, addr);

    if (num_bytes <= 8) {
        auto pl0Rd = (tas_pl0rq_rd_st*)&mRqBuf[mRqBufWi];
        pl0Rd->wl = 0;
        switch (num_bytes) {
        case 1: pl0Rd->cmd = TAS_PL0_CMD_RD8;  break;
        case 2: pl0Rd->cmd = TAS_PL0_CMD_RD16; assert((addr & 0x1) == 0); break;
        case 4: pl0Rd->cmd = TAS_PL0_CMD_RD32; assert((addr & 0x3) == 0); break;
        case 8: pl0Rd->cmd = TAS_PL0_CMD_RD64; assert((addr & 0x7) == 0); break;
        default: assert(false);
        }
        pl0Rd->a15to0 = (uint16_t)(addr & 0xFFFF);

        mRqBufWi += 1;
        if (pl0Rd->cmd == TAS_PL0_CMD_RD64)
            mRspSize += sizeof(tas_pl0rsp_rd_st) + 8;
        else
            mRspSize += sizeof(tas_pl0rsp_rd_st) + 4;
    }
    else {  // 64 bit aligned and sized block reads
        assert((addr & 0x7) == 0);
        assert((num_bytes & 0x7) == 0);
        auto pl0RdBlk = (tas_pl0rq_rdblk_st*)&mRqBuf[mRqBufWi];
        pl0RdBlk->wl = 1;
        pl0RdBlk->cmd = TAS_PL0_CMD_RDBLK;
        pl0RdBlk->a15to0 = (uint16_t)(addr & 0xFFFF);
        mRqBuf[mRqBufWi + 1] = 0;  // reserved[3]
        pl0RdBlk->wlrd = (uint8_t)(num_bytes >> 2);
        mRqBufWi += 2;

        mRspSize += sizeof(tas_pl0rsp_rd_st) + num_bytes;
    }

    tas_rw_trans_st* pt = &mPl0Trans[mPl0NumTrans];
    pt->addr = addr;
    pt->num_bytes = num_bytes;
    pt->acc_mode = mPl0AccMode;
    pt->addr_map = mPl0AddrMap;
    pt->type = TAS_RW_TT_RD;
    pt->rdata = data;

    mPl0TransRsp[mPl0NumTrans].num_bytes_ok = 0;
    mPl0TransRsp[mPl0NumTrans].pl_err = TAS_PL_ERR_PROTOCOL;
    
    mPl0NumTrans++;
    mPl2NumTrans++;
}

void CTasPktHandlerRw::mPktAdd_Wr(uint64_t addr, uint32_t num_bytes, const void* data, uint16_t acc_mode, uint8_t addr_map)
{
    if (num_bytes == 0)  // Simplifies calling function
        return;

    tphrCheckNumBytes(num_bytes);

    if (mPl2NumTrans >= mConInfo.pl0_max_num_rw) {
        assert(mPl2NumTrans == mConInfo.pl0_max_num_rw);
        mPktFinalize();  // Start a new PL2
    }

    uint32_t numBytesAmAmBa = mGetNumBytesAddrMapAccModeBaseAddr(addr_map, acc_mode, addr);
    uint32_t numBytesNeededPktrq = numBytesAmAmBa + sizeof(tas_pl0rq_wrblk_st) + (((num_bytes + 3) / 4) * 4) + sizeof(tas_pl1rq_pl0_end_st);
    uint32_t numBytesNeededPktrsp = sizeof(tas_pl0rsp_wr_st) + sizeof(tas_pl1rsp_pl0_end_st);
    mPktFinalizeIfNeeded(numBytesNeededPktrq, numBytesNeededPktrsp);

    mPktAdd_SetAddrMapAccModeBaseAddr(addr_map, acc_mode, addr);

    if (num_bytes <= 8) {
        auto pl0Wr = (tas_pl0rq_wr_st*)&mRqBuf[mRqBufWi];
        pl0Wr->wl = (num_bytes > 4) ? 2 : 1;
        switch (num_bytes) {
        case 1: pl0Wr->cmd = TAS_PL0_CMD_WR8;  break;
        case 2: pl0Wr->cmd = TAS_PL0_CMD_WR16; assert((addr & 0x1) == 0); break;
        case 4: pl0Wr->cmd = TAS_PL0_CMD_WR32; assert((addr & 0x3) == 0); break;
        case 8: pl0Wr->cmd = TAS_PL0_CMD_WR64; assert((addr & 0x7) == 0); break;
        default: assert(false);
        }
        pl0Wr->a15to0 = (uint16_t)(addr & 0xFFFF);

        mRqBuf[mRqBufWi + 1] = 0;  // Set upper bytes in case of WR8 and WR16
        if (data != nullptr) {
            memcpy(&mRqBuf[mRqBufWi + 1], data, num_bytes);
        }
        else assert(false);

        mRqBufWi += 1 + pl0Wr->wl;
        mRspSize += sizeof(tas_pl0rsp_wr_st);
    }
    else {  // 64 bit aligned and sized block write
        assert((addr & 0x7) == 0);
        assert((num_bytes & 0x7) == 0);
        auto pl0WrBlk = (tas_pl0rq_wrblk_st*)&mRqBuf[mRqBufWi];
        pl0WrBlk->wl = num_bytes / 4;
        pl0WrBlk->cmd = TAS_PL0_CMD_WRBLK;
        pl0WrBlk->a15to0 = (uint16_t)(addr & 0xFFFF);

        uint32_t wl = pl0WrBlk->wl;
        if (wl == 0)
            wl = 256;  // 1KB
        uint32_t mPl1WiNew = mRqBufWi + 1 + wl;

        if (mPl1WiNew < mRqWiMax) {
            memcpy(&mRqBuf[mRqBufWi + 1], data, num_bytes);
        }
        else assert(false);

        mRqBufWi = mPl1WiNew;
        mRspSize += sizeof(tas_pl0rsp_wr_st);
    }

    tas_rw_trans_st* pt = &mPl0Trans[mPl0NumTrans];
    pt->addr = addr;
    pt->num_bytes = num_bytes;
    pt->acc_mode = mPl0AccMode;
    pt->addr_map = mPl0AddrMap;
    pt->type = TAS_RW_TT_WR;
    pt->wdata = data;

    mPl0TransRsp[mPl0NumTrans].num_bytes_ok = 0;
    mPl0TransRsp[mPl0NumTrans].pl_err = TAS_PL_ERR_PROTOCOL;

    mPl0NumTrans++;
    mPl2NumTrans++;
}

void CTasPktHandlerRw::mPktAdd_Fill(uint64_t addr, uint32_t num_bytes, uint64_t value, uint16_t acc_mode, uint8_t addr_map)
{
    assert((num_bytes >= 8) && (num_bytes <= 1024) && !(num_bytes % 8));

    if (mPl2NumTrans >= mConInfo.pl0_max_num_rw) {
        assert(mPl2NumTrans == mConInfo.pl0_max_num_rw);
        mPktFinalize();  // Start a new PL2
    }

    uint32_t numBytesAmAmBa = mGetNumBytesAddrMapAccModeBaseAddr(addr_map, acc_mode, addr);
    uint32_t numBytesNeededPktrq = numBytesAmAmBa + sizeof(tas_pl0rq_wrblk_st) + 8 + sizeof(tas_pl1rq_pl0_end_st);
    uint32_t numBytesNeededPktrsp = sizeof(tas_pl0rsp_wr_st) + sizeof(tas_pl1rsp_pl0_end_st);
    mPktFinalizeIfNeeded(numBytesNeededPktrq, numBytesNeededPktrsp);

    mPktAdd_SetAddrMapAccModeBaseAddr(addr_map, acc_mode, addr);

    auto pl0Fill = (tas_pl0rq_fill_st*)&mRqBuf[mRqBufWi];
    pl0Fill->wl = (sizeof(tas_pl0rq_fill_st) / 4) - 1;
    pl0Fill->cmd = TAS_PL0_CMD_FILL;
    pl0Fill->a15to0 = (uint16_t)(addr & 0xFFFF);
    pl0Fill->wlwr = (uint8_t)(num_bytes / 4);
    pl0Fill->reserved[0] = pl0Fill->reserved[1] = pl0Fill->reserved[2] = 0;
    pl0Fill->value = value;

    mRqBufWi += (1 + pl0Fill->wl);
    assert(mRqBufWi < mRqWiMax);
    mRspSize += sizeof(tas_pl0rsp_wr_st);

    tas_rw_trans_st* pt = &mPl0Trans[mPl0NumTrans];
    pt->addr = addr;
    pt->num_bytes = num_bytes;
    pt->acc_mode = mPl0AccMode;
    pt->addr_map = mPl0AddrMap;
    pt->type = TAS_RW_TT_FILL;
    pt->wdata = &pl0Fill->value;

    mPl0TransRsp[mPl0NumTrans].num_bytes_ok = 0;
    mPl0TransRsp[mPl0NumTrans].pl_err = TAS_PL_ERR_PROTOCOL;

    mPl0NumTrans++;
    mPl2NumTrans++;
}

void CTasPktHandlerRw::mPl2PktInit()
{
    assert(mPl2NumTrans <= mConInfo.pl0_max_num_rw);
    mPl2NumTrans = 0;

    mPl2HdrWi = mRqBufWi;
    mPl0Start = (tas_pl1rq_pl0_start_st*)&mRqBuf[mRqBufWi + 1];
    memset(&mRqBuf[mRqBufWi], 0, 3 * 4);  // PL2 and PL1 header set in mPktFinalize()

    mRqBufWi += 3;  // After PL2 pkt length and tas_pl1rq_pl0_start_st

    mPl2RspPktStart = mRspSize;
    mRspSize += 4 + sizeof(tas_pl1rsp_pl0_start_st);

    mPl0AccMode = 0;
    mPl0AddrMap = 0;
    mPl0AddrMapMask = 0;
    mPl0BaseAddr = 0;
}

void CTasPktHandlerRw::mPktFinalize(bool init_next_pl2_pkt)
{
    assert(mRqBufWi != 0);

    mNumPl2Pkt++;  // Packet will be sent

    mPl1CntOutstandingLast++;

    mPl0Start->wl = 1;
    mPl0Start->cmd = TAS_PL1_CMD_PL0_START;
    mPl0Start->protoc_ver = PROTOC_VER;
    mPl0Start->con_id = 0xFF;  // Overwritten and used on lower layer
    mPl0Start->pl1_cnt = mPl1CntOutstandingLast;
    mPl0Start->pl0_addr_map_mask = mPl0AddrMapMask;

    auto pl0End = (tas_pl1rq_pl0_end_st*)&mRqBuf[mRqBufWi];
    pl0End->wl = 0;
    pl0End->cmd = TAS_PL1_CMD_PL0_END;
    pl0End->num_pl0_rw = mPl0NumTrans;
    mRqBufWi++;  // For tas_pl1rq_pl0_end_st

    mRqBuf[mPl2HdrWi] = (mRqBufWi - mPl2HdrWi) * 4;  // Set length of current PL2 packet in header

    mRspSize += sizeof(tas_pl1rsp_pl0_end_st);

    if (init_next_pl2_pkt) {
        mPl2PktInit();
        // Enforce refresh of accMode, addrMap and baseAddr
        mPl0AddrMap = 0;
        mPl0AccMode = 0; 
        mPl0BaseAddr = 0;  
    }
}

bool CTasPktHandlerRw::mCheckLimits(uint32_t num_bytes_rd, uint32_t num_bytes_wr) const
{
    if (mRqBufWi * 4 + num_bytes_wr + 4 >= mRqWiMax * 4) {
        assert(mRqBufWi * 4 <= mRqWiMax * 4 + BUF_ALLOWANCE);
        return false;
    }
    if (mRspSize + num_bytes_rd + 4 >= mMaxRspSize) {
        assert(mRspSize <= mMaxRspSize + BUF_ALLOWANCE);
        return false;
    }
    return true;
}

uint32_t CTasPktHandlerRw::mGetNumBytesAddrMapAccModeBaseAddr(uint8_t addr_map, uint16_t acc_mode, uint64_t addr) const
{
    uint32_t nbAddrMap;
    uint32_t nbAccMode;
    uint32_t nbBaseAddr;

    nbAddrMap = (addr_map != mPl0AddrMap) ? sizeof(tas_pl0rq_addr_map_st) : 0;
    nbAccMode = (acc_mode != mPl0AccMode) ? sizeof(tas_pl0rq_acc_mode_st) : 0;

    nbBaseAddr = 0;
    if ((addr < mPl0BaseAddr) || (addr > (mPl0BaseAddr + 0xFFFF))) {
        nbBaseAddr = (addr < 0x100000000LL) ? sizeof(tas_pl0rq_base_addr32_st) : sizeof(tas_pl0rq_base_addr64_st);
    }
    return nbAddrMap + nbAccMode + nbBaseAddr;
}

bool CTasPktHandlerRw::mCheckAddrMapRulesInPkt(uint8_t addr_map) const
{
    assert(addr_map <= TAS_AM15);

    uint16_t addrMapMask = 1 << addr_map;
    if ((mPl0AddrMapMask == 0) ||  // First transaction in PL1 packet
        (mPl0AddrMapMask == addrMapMask)) {
        return true;
    }

    // A PL1 packet may contain a mix of PL0 address maps if all are less than 12.
    // Rules for address maps 12, 13, TAS_AM14 and TAS_AM15:
    // 1. May only be used exclusively in a Pl1 packet.
    if ((addrMapMask & TAS_AM_MASK_SHARED) && !(mPl0AddrMapMask & TAS_AM_MASK_EXCLUSIVE)) {
        assert(addr_map <= TAS_AM1);  // AURIX usage
        return true;
    }

    return false;
}

bool tphrCheckIfNaturalAligned(uint64_t addr, uint32_t num_bytes)
{
    assert(num_bytes <= 8);
    switch (num_bytes) {
    case 0:
    case 3:
    case 5:
    case 6:
    case 7: return false;   
    default: break;
    }
    return (addr % num_bytes == 0);
}


bool CTasPktHandlerRw::mNumTransManageableRd(uint64_t addr, uint32_t num_bytes) const
{
    if (num_bytes <= 8) {
        uint32_t numTrans = 4;  // Worst case for unaligned access
        if (tphrCheckIfNaturalAligned(addr, num_bytes))
            numTrans = 1;
        else if ((num_bytes == 8) && (addr % 8 == 4))
            numTrans = 2; // 32bit aligned DAS block transfer

        return ((mPl2NumTrans + numTrans) <= mNumTransMax);
    }
    else {
        uint32_t payloadPerPkt = mConInfo.max_pl2rsp_pkt_size
            - sizeof(uint32_t)
            - sizeof(tas_pl1rsp_pl0_start_st)
            - sizeof(tas_pl0rsp_wr_st)
            - sizeof(tas_pl1rsp_pl0_end_st);

        uint32_t numTransAdder = 3;  // Worst case adder
        if ((num_bytes % 8 == 0) && (addr % 8 == 0))
            numTransAdder = 0;  // Fully aligned block transfer

        uint32_t numTransPayload = (num_bytes + payloadPerPkt - 1) / payloadPerPkt;

        return ((mPl2NumTrans + numTransPayload + numTransAdder) <= mNumTransMax);
    }
}

bool CTasPktHandlerRw::mNumTransManageableWr(uint64_t addr, uint32_t num_bytes) const
{
    if (num_bytes <= 8) {
        uint32_t numTrans = 4;  // Worst case for unaligned access
        if (tphrCheckIfNaturalAligned(addr, num_bytes))
            numTrans = 1;
        else if ((num_bytes == 8) && (addr % 8 == 4))
            numTrans = 2; // 32bit aligned DAS block transfer

        return ((mPl2NumTrans + numTrans) <= mNumTransMax);
    }
    else {
        uint32_t payload = num_bytes
            + sizeof(tas_pl0rq_acc_mode_st)
            + sizeof(tas_pl0rq_addr_map_st)
            + sizeof(tas_pl0rq_base_addr64_st);

        uint32_t payloadPerPkt = mConInfo.max_pl2rq_pkt_size
            - sizeof(uint32_t)
            - sizeof(tas_pl1rq_pl0_start_st)
            - sizeof(tas_pl0rq_wrblk_st)
            - sizeof(tas_pl1rq_pl0_end_st);

        uint32_t numTransAdder = 3;  // Worst case adder
        if ((num_bytes % 8 == 0) && (addr % 8 == 0))
            numTransAdder = 0;  // Fully aligned block transfer

        uint32_t numTransPayload = (payload + payloadPerPkt - 1) / payloadPerPkt;

        return ((mPl2NumTrans + numTransPayload + numTransAdder) <= mNumTransMax);
    }
}

uint32_t CTasPktHandlerRw::mGetRemainingSizeInPktRq() const
{
    assert(mRqBufWi > mPl2HdrWi);
    uint32_t numBytesUsed = 4 * (mRqBufWi - mPl2HdrWi);

    uint32_t numBytesMax = mConInfo.max_pl2rq_pkt_size;
    if (numBytesMax > (mRqWiMax * 4))
        numBytesMax = (mRqWiMax * 4);

    if (numBytesUsed < numBytesMax) {
        return numBytesMax - numBytesUsed;
    }
    else {
        assert(numBytesUsed == numBytesMax);
        return 0;
    }
}

uint32_t CTasPktHandlerRw::mGetRemainingSizeInPktRsp() const
{
    assert(mRspSize >= mPl2RspPktStart);
    uint32_t numBytesUsed = mRspSize - mPl2RspPktStart;

    uint32_t numBytesMax = mConInfo.max_pl2rsp_pkt_size;
    if (numBytesMax > mMaxRspSize)
        numBytesMax = mMaxRspSize;

    if (numBytesUsed < numBytesMax) {
        return numBytesMax - numBytesUsed;
    }
    else {
        assert(false);
        return 0;
    }
}

uint32_t CTasPktHandlerRw::mGetWrDataBlkSizeInPktRq(uint32_t num_bytes, uint64_t addr) const
{
    assert(num_bytes >= 8);
    uint32_t nb = num_bytes;

    uint32_t numBytesProtocol   = sizeof(tas_pl0rq_wrblk_st) + sizeof(tas_pl1rq_pl0_end_st);
    uint32_t numDataBlkBytesMax = mGetRemainingSizeInPktRq();
    if (numBytesProtocol >= numDataBlkBytesMax) {
        assert(numDataBlkBytesMax >= sizeof(tas_pl1rq_pl0_end_st));  // Need to finalize this pkt
        return 0;
    }

    numDataBlkBytesMax -= numBytesProtocol;

    if (addr >= 0x100000000)
        numDataBlkBytesMax -= sizeof(tas_pl0rq_base_addr64_st);

    if (nb > numDataBlkBytesMax)
        nb = numDataBlkBytesMax;
    if (nb > mMaxWrDataBlkSizeInPktRq)
        nb = mMaxWrDataBlkSizeInPktRq;

    return nb & ~0x7;  // Multiple of 64 bits;
}

uint32_t CTasPktHandlerRw::mGetRdDataBlkSizeInPktRsp(uint32_t num_bytes) const
{
    assert(num_bytes >= 8);
    uint32_t nb = num_bytes;

    assert(sizeof(tas_pl0rsp_rd_st) == sizeof(tas_pl0rsp_rdblk1kb_st));
    uint32_t numBytesProtocol = sizeof(tas_pl0rsp_rd_st) + sizeof(tas_pl1rsp_pl0_end_st);
    uint32_t numDataBlkBytesMax = mGetRemainingSizeInPktRsp();
    if (numBytesProtocol >= numDataBlkBytesMax) {
        assert(numDataBlkBytesMax >= sizeof(tas_pl1rsp_pl0_end_st));
        return 0;
    }
    numDataBlkBytesMax -= numBytesProtocol;

    if (nb > numDataBlkBytesMax)
        nb = numDataBlkBytesMax;
    if (nb > mMaxRdDataBlkSizeInPktRsp)
        nb = mMaxRdDataBlkSizeInPktRsp;

    return nb & ~0x7;  // Multiple of 64 bits;
}

bool CTasPktHandlerRw::rw_add_rd(uint64_t addr, uint32_t num_bytes, void* data, uint16_t acc_mode, uint8_t addr_map)
{
    if (num_bytes == 0)
        return false;  // Useful for simple regression testing loops -> no assertion

    if (!mCheckLimits(num_bytes, 0))
        return false;

    uint8_t addrMap = (addr_map == TAS_AM132) ? TAS_AM15 : addr_map;
    if (addrMap > TAS_AM15)
        return false;

    if (!mNumTransManageableRd(addr, num_bytes))
        mPktFinalize();

    // Start new PL2 packet if needed
    bool newPl2Pkt = false;
    if (addr_map >= TAS_AM12) {
        assert(addr < 0x100000000);  // Only 32 bit addresses allowed
        newPl2Pkt = !mCheckRemainingPktSizeSufficient(32, 16 + ((num_bytes + 3) / 4) * 4);  // Never split block reads
    }
    else {
        // Enforce a new PL2 packet if there is no reasonable size left for another PL0
        if (num_bytes <= 16)
            newPl2Pkt = !mCheckRemainingPktSizeSufficient(32, 16 + ((num_bytes + 3) / 4) * 4);
        else
            newPl2Pkt = !mCheckRemainingPktSizeSufficient(32, 64);  // Increase probability that block reads are not split
    }
    newPl2Pkt |= !mCheckAddrMapRulesInPkt(addrMap);
    if (newPl2Pkt)
        mPktFinalize();  // Start new PL2 packet

    uint64_t a = addr;
    uint32_t nb = num_bytes;
    auto d = (uint8_t*)data;

    if ((nb > 0) && (a & 1)) {
        mPktAdd_Rd(a, 1, d, acc_mode, addrMap);
        a++; d++; nb--;
    }
    if ((nb >= 2) && (a & 2)) {
        mPktAdd_Rd(a, 2, d, acc_mode, addrMap);
        a += 2; d += 2; nb-= 2;
    }
    if ((nb >= 4) && (a & 4)) {
        mPktAdd_Rd(a, 4, d, acc_mode, addrMap);
        a += 4; d += 4; nb -= 4;
    }
    if (nb >= 8) {
        assert((a & 0x7) == 0);  // 64 bit aligned address
        do {
            uint32_t nbBlk = mGetRdDataBlkSizeInPktRsp(nb);
            mPktAdd_Rd(a, nbBlk, d, acc_mode, addrMap);
            if ((nbBlk < TAS_PL0_DATA_BLK_SIZE) && (nbBlk < (nb & ~0x7)))
                mPktFinalize();  // Start new PL2 packet
            a += nbBlk; d += nbBlk; nb -= nbBlk;
        } while (nb > 7);
    }
    assert(nb < 8);
    if (nb >= 4) {
        mPktAdd_Rd(a, 4, d, acc_mode, addrMap);
        a += 4; d += 4; nb -= 4;
    }
    if (nb >= 2) {
        mPktAdd_Rd(a, 2, d, acc_mode, addrMap);
        a += 2; d += 2; nb -= 2;
    }
    if (nb > 0) {
        mPktAdd_Rd(a, 1, d, acc_mode, addrMap);
        a++; d++; nb--;
    }
    assert(nb == 0);
    assert(a == addr + num_bytes);
    assert(d == (uint8_t*)data + num_bytes);

    tas_rw_trans_st* pt = &mRwTrans[mRwNumTrans];
    pt->addr = addr;
    pt->num_bytes = num_bytes;
    pt->acc_mode = mPl0AccMode;
    pt->addr_map = mPl0AddrMap;
    pt->type = TAS_RW_TT_RD;
    pt->rdata = data;

    mRwTransRsp[mRwNumTrans].num_bytes_ok = 0;
    mRwTransRsp[mRwNumTrans].pl_err = TAS_PL_ERR_PROTOCOL;

    mRwNumTrans++;

    return true;
}

bool CTasPktHandlerRw::rw_add_wr(uint64_t addr, uint32_t num_bytes, const void* data, uint16_t acc_mode, uint8_t addr_map)
{
    if (num_bytes == 0)
        return false;   // Useful for simple regression testing loops -> no assertion

    if (!mCheckLimits(0, num_bytes))
        return false;

    uint8_t addrMap = (addr_map == TAS_AM132) ? TAS_AM15 : addr_map;
    if (addrMap > TAS_AM15)
        return false;

    if (!mNumTransManageableWr(addr, num_bytes))
        mPktFinalize();

    // Start new PL2 packet if needed
    bool newPl2Pkt = false;
    if (addr_map >= TAS_AM12) {
        assert(addr < 0x100000000);  // Only 32 bit addresses allowed
        newPl2Pkt = !mCheckRemainingPktSizeSufficient(32 + ((num_bytes + 3) / 4) * 4, 32);  // Never split block writes
    }
    else {
        // Enforce a new PL2 packet if there is no reasonable size left for another PL0
        if (num_bytes <= 16)
            newPl2Pkt = !mCheckRemainingPktSizeSufficient(32 + ((num_bytes + 3) / 4) * 4, 32);
        else
            newPl2Pkt = !mCheckRemainingPktSizeSufficient(80, 32);  // Increase probability that block writes are not split
    }
    newPl2Pkt |= !mCheckAddrMapRulesInPkt(addrMap);
    if (newPl2Pkt)
        mPktFinalize();  // Start new PL2 packet

    uint64_t a = addr;
    uint32_t nb = num_bytes;
    auto d = (const uint8_t*)data;

    if ((nb > 0) && (a & 1)) {
        mPktAdd_Wr(a, 1, d, acc_mode, addrMap);
        a++; d++; nb--;
    }
    if ((nb >= 2) && (a & 2)) {
        mPktAdd_Wr(a, 2, d, acc_mode, addrMap);
        a += 2; d += 2; nb -= 2;
    }
    if ((nb >= 4) && (a & 4)) {
        mPktAdd_Wr(a, 4, d, acc_mode, addrMap);
        a += 4; d += 4; nb -= 4;
    }
    if (nb >= 8) {
        assert((a & 0x7) == 0);  // 64 bit aligned address
        do {
            uint32_t nbBlk = mGetWrDataBlkSizeInPktRq(nb, a);
            mPktAdd_Wr(a, nbBlk, d, acc_mode, addrMap);
            if ((nbBlk < TAS_PL0_DATA_BLK_SIZE) && (nbBlk < (nb & ~0x7)))
                mPktFinalize();  // Start new PL2 packet
            a += nbBlk; d += nbBlk; nb -= nbBlk;
        } while (nb > 7);
    }
    assert(nb < 8);
    if (nb >= 4) {
        mPktAdd_Wr(a, 4, d, acc_mode, addrMap);
        a += 4; d += 4; nb -= 4;
    }
    if (nb >= 2) {
        mPktAdd_Wr(a, 2, d, acc_mode, addrMap);
        a += 2; d += 2; nb -= 2;
    }
    if (nb > 0) {
        mPktAdd_Wr(a, 1, d, acc_mode, addrMap);
        a++; d++; nb--;
    }
    assert(nb == 0);
    assert(a == addr + num_bytes);
    assert(d == (const uint8_t*)data + num_bytes);

    tas_rw_trans_st* pt = &mRwTrans[mRwNumTrans];
    pt->addr = addr;
    pt->num_bytes = num_bytes;
    pt->acc_mode = mPl0AccMode;
    pt->addr_map = mPl0AddrMap;
    pt->type = TAS_RW_TT_WR;
    pt->wdata = data;

    mRwTransRsp[mRwNumTrans].num_bytes_ok = 0;
    mRwTransRsp[mRwNumTrans].pl_err = TAS_PL_ERR_PROTOCOL;

    mRwNumTrans++;

    return true;
}

bool CTasPktHandlerRw::rw_add_fill(uint64_t addr, uint32_t num_bytes, uint64_t value, uint16_t acc_mode, uint8_t addr_map)

{
    if ((addr % 8) || (num_bytes == 0) || (num_bytes % 8) || (acc_mode != 0) || (addr_map >= TAS_AM12)) {
        assert(false);
        return false;
    }

    if (!mCheckLimits(0, num_bytes))  // 8 not num_bytes for fill
        return false;

    if (!mNumTransManageableWr(addr, 8))  // 8 not num_bytes for fill
        mPktFinalize();

    // Start new PL2 packet if needed
    bool newPl2Pkt = !mCheckRemainingPktSizeSufficient(32 + 8, 32);  // 8 not num_bytes for fill
    newPl2Pkt |= !mCheckAddrMapRulesInPkt(addr_map);
    if (newPl2Pkt)
        mPktFinalize();  // Start new PL2 packet

    uint64_t a = addr;
    uint32_t nb = num_bytes;
    do {
        uint32_t nbNow = (nb > TAS_PL0_DATA_BLK_SIZE) ? TAS_PL0_DATA_BLK_SIZE : nb;
        mPktAdd_Fill(a, nbNow, value, acc_mode, addr_map);
        a += nbNow; nb -= nbNow;
    } while (nb > 0);

    tas_rw_trans_st* pt = &mRwTrans[mRwNumTrans];
    pt->addr = addr;
    pt->num_bytes = num_bytes;
    pt->acc_mode = mPl0AccMode;
    pt->addr_map = mPl0AddrMap;
    pt->type = TAS_RW_TT_FILL;
    pt->wdata = nullptr;

    mRwTransRsp[mRwNumTrans].num_bytes_ok = 0;
    mRwTransRsp[mRwNumTrans].pl_err = TAS_PL_ERR_PROTOCOL;

    mRwNumTrans++;

    return true;
}

bool CTasPktHandlerRw::rw_set_trans(const tas_rw_trans_st* trans, uint32_t num_trans)
{
    rw_start();

    bool succ = false;
    for (uint32_t t = 0; t < num_trans; t++) {
        if (trans[t].type == TAS_RW_TT_RD) {
            succ = rw_add_rd(trans[t].addr, trans[t].num_bytes, trans[t].rdata, trans[t].acc_mode, trans[t].addr_map);
        }
        else if (trans[t].type == TAS_RW_TT_WR) {
            succ = rw_add_wr(trans[t].addr, trans[t].num_bytes, trans[t].wdata, trans[t].acc_mode, trans[t].addr_map);
        }
        else if (trans[t].type == TAS_RW_TT_FILL) {
            uint64_t value = *(const uint64_t*)trans[t].wdata;
            succ = rw_add_fill(trans[t].addr, trans[t].num_bytes, value, trans[t].acc_mode, trans[t].addr_map);
        }
        else {
            assert(false);
            succ = false;
        }
        if (succ == false) {
            rw_start();  // Enforce all or nothing
            break;
        }
    }

    if (succ)
        assert(mRwNumTrans == num_trans);

    return succ;
}

uint32_t CTasPktHandlerRw::rw_get_rq_size() const
{
    assert((mRqBufWi * 4) <= mMaxRqSize + BUF_ALLOWANCE);
    if (mGetPktRqWasCalled == false)
        return (mRqBufWi * 4) + sizeof(tas_pl1rq_pl0_end_st);
    else
        return (mRqBufWi * 4);
}

uint32_t CTasPktHandlerRw::rw_get_rsp_size() const
{
    assert(mRspSize <= mMaxRspSize + BUF_ALLOWANCE);
    if (mGetPktRqWasCalled == false)
        return mRspSize + sizeof(tas_pl1rsp_pl0_end_st);
    else
        return mRspSize;
}

void CTasPktHandlerRw::rw_get_rq(const uint32_t** rq, uint32_t* rq_num_bytes, uint32_t* rsp_num_bytes_max, uint32_t* num_pl2_pkt)
{
    if (mGetPktRqWasCalled == false) {
        mPktFinalize(false);
        mGetPktRqWasCalled = true;
    }
    else assert(false);  // Calling twice makes no sense

    *rq = mRqBuf;
    *rq_num_bytes = rw_get_rq_size();
    *rsp_num_bytes_max = rw_get_rsp_size();
    *num_pl2_pkt = mNumPl2Pkt;
}

uint32_t CTasPktHandlerRw::rw_get_num_pl2_pkt(const uint32_t* rsp, uint32_t num_bytes) const
{
    if (mNumPl2Pkt == 0) {
        assert(false);  // This case will be handled in a robust higher layer
        return 0;
    }
    assert(num_bytes <= mRspSize);

    uint32_t numBytesDone = 0;
    uint32_t numPl2Pkt = 0;
    uint32_t wiPl2Hdr = 0;
    while (true) {
        uint32_t pl2PktSize = rsp[wiPl2Hdr];
        if (pl2PktSize % 4) {
            assert(false);
            break;
        }
        numBytesDone += pl2PktSize;
        wiPl2Hdr += pl2PktSize / 4;
        numPl2Pkt++;
        if (numBytesDone >= num_bytes) {
            assert(numBytesDone == num_bytes);
            break;
        }
        if (numPl2Pkt == mNumPl2Pkt) {
            assert(numBytesDone == num_bytes);
            break;  // Success
        }
    }
    return numPl2Pkt;
}


bool ctprhcPl0CmdIsRd(tas_pl_cmd_et cmd)
{
    switch (cmd) {
    case TAS_PL0_CMD_RD8:
    case TAS_PL0_CMD_RD16:
    case TAS_PL0_CMD_RD32:
    case TAS_PL0_CMD_RD64:
    case TAS_PL0_CMD_RDBLK:
    case TAS_PL0_CMD_RDBLK1KB: return true;
    default: 
        return false;
    }
}

bool ctprhcPl0CmdIsWrOrFill(tas_pl_cmd_et cmd)
{
    switch (cmd) {
    case TAS_PL0_CMD_WR8:
    case TAS_PL0_CMD_WR16:
    case TAS_PL0_CMD_WR32:
    case TAS_PL0_CMD_WR64:
    case TAS_PL0_CMD_WRBLK:
    case TAS_PL0_CMD_FILL:  return true;
    default:
        return false;
    }
}

tas_return_et CTasPktHandlerRw::rw_set_rsp(const uint32_t* rsp, uint32_t num_bytes)
{
    assert(mRwTransRsp[0].pl_err == TAS_PL_ERR_PROTOCOL);   // As well for all others
    assert(mPl0TransRsp[0].pl_err == TAS_PL_ERR_PROTOCOL);  // As well for all others

    assert(mRwNumTrans > 0);
    assert(mPl0NumTrans > 0);
    assert(mRspSize > 0);

    if ((num_bytes < 8) || (num_bytes % 4) || (num_bytes > mRspSize)) {
        return mSetPktRspErrConnectionProtocol();
    }

    tas_clear_error_info(mEip);  // Capture first error

    uint32_t wi;
    uint32_t wiMax;
    uint32_t wiPktStartNext;
    uint32_t iTrans;
    wiMax = num_bytes / 4;

    wi = wiPktStartNext = iTrans = 0;
    while (wi < wiMax) {

        // TAS_PL1_CMD_PL0_START
        if (wi == wiPktStartNext) {
            if ((rsp[wi] % 4 != 0) || (rsp[wi] > mMaxRspSize)) {
                return mSetPktRspErrConnectionProtocol();
            }
            wiPktStartNext += rsp[wi] / 4;
            auto pl1Start = (const tas_pl1rsp_pl0_start_st*)&rsp[wi+1];
            if ((pl1Start->wl != 0) || (pl1Start->cmd != TAS_PL1_CMD_PL0_START)) {
                return mSetPktRspErrConnectionProtocol();
            }
            if (pl1Start->err == TAS_PL1_ERR_DEV_ACCESS) {
                return mSetPktRspErrDeviceAccess();
            }
            else if (pl1Start->err == TAS_PL1_ERR_DEV_RESET) {
                mDeviceResetCount++;
            }
            else if (pl1Start->err != TAS_PL_ERR_NO_ERROR) {
                return mSetPktRspErrConnectionProtocol();
            }

            wi += (4 + sizeof(tas_pl1rsp_pl0_end_st)) / 4;
            continue;
        }

        uint8_t       wl   = rsp[wi] & 0xFF;
        auto cmd  = (tas_pl_cmd_et)((rsp[wi] >> 8) & 0xFF);
        
        // TAS_PL1_CMD_PL0_END
        if (cmd == TAS_PL1_CMD_PL0_END) {
            auto pl1End = (const tas_pl1rsp_pl0_end_st*)&rsp[wi];
            if (pl1End->wl != 0) {
                return mSetPktRspErrConnectionProtocol();
            }
            if (pl1End->pl1_cnt != mPl1CntOutstandingOldest) {
                return mSetPktRspErrPl1Cnt();
            }
            wi += sizeof(tas_pl1rsp_pl0_end_st) / 4;
            if (iTrans == mPl0NumTrans) {
                assert(mPl1CntOutstandingOldest == mPl1CntOutstandingLast);
                break;
            }
            else {
                mPl1CntOutstandingOldest++;
                assert(wi < wiMax);
                continue;
            }
        }

        // TAS_PL0_CMD_
        tas_rw_trans_rsp_st* pktRsp = &mPl0TransRsp[iTrans];
        assert(pktRsp->pl_err == TAS_PL_ERR_PROTOCOL);
        tas_rw_trans_st* pt = &mPl0Trans[iTrans];
        uint16_t wlrwNoErr = ((pt->num_bytes + 3) / 4);  // Expected wlrw value if no error

        if (ctprhcPl0CmdIsWrOrFill(cmd)) {
            auto pkt = (const tas_pl0rsp_wr_st*)&rsp[wi];
            if (wl != 0) {
                return mSetPktRspErrConnectionProtocol();
            }
            if ((pt->type != TAS_RW_TT_WR) && (pt->type != TAS_RW_TT_FILL)) {
                return mSetPktRspErrConnectionProtocol();
            }
            if (pkt->err != TAS_PL0_ERR_NO_ERROR) {
                pktRsp->pl_err = pkt->err;
                if (cmd == TAS_PL0_CMD_WRBLK) {
                    if (pkt->wlwr >= wlrwNoErr) {
                        return mSetPktRspErrConnectionProtocol();
                    }
                    pktRsp->num_bytes_ok = pkt->wlwr * 4;
                }
                else {
                    if (pkt->wlwr != 0) {
                        return mSetPktRspErrConnectionProtocol();
                    }
                    pktRsp->num_bytes_ok = 0;
                }
                mSetPktRspErrPl0Data(pktRsp->pl_err, TAS_ERR_RW_WRITE, pt->addr + pkt->wlwr * 4, pt->addr_map);
            }
            else {  // TAS_PL0_ERR_NO_ERROR
                if ((pkt->wlwr != (0xFF & (pt->num_bytes + 3) / 4)) || (pkt->wl != 0)) {
                    return mSetPktRspErrConnectionProtocol();
                }
                else {
                    pktRsp->num_bytes_ok = pt->num_bytes;
                    pktRsp->pl_err = TAS_PL0_ERR_NO_ERROR;
                }
            }
            wi += 1 + wl;
            iTrans++;
        }
        else if (ctprhcPl0CmdIsRd(cmd)) {
            auto pkt = (const tas_pl0rsp_rd_st*)&rsp[wi];
            if (wl != pkt->wlrd) {
                return mSetPktRspErrConnectionProtocol();
            }
            if (pt->type != TAS_RW_TT_RD) {
                return mSetPktRspErrConnectionProtocol();
            }
            if (cmd == TAS_PL0_CMD_RDBLK1KB) {
                if ((wlrwNoErr != 0x100) || (wl != 0) ||  // wl 0 means 256 words -> 1kB
                    (pkt->wlrd != 0) || (pkt->err != TAS_PL0_ERR_NO_ERROR)) {
                    return mSetPktRspErrConnectionProtocol();
                }
                pktRsp->num_bytes_ok = TAS_PL0_DATA_BLK_SIZE;
                pktRsp->pl_err = TAS_PL0_ERR_NO_ERROR;
                memcpy(pt->rdata, &rsp[wi + 1], TAS_PL0_DATA_BLK_SIZE);
                wi += 1 + 256;
            }
            else {
                if (pkt->err != TAS_PL0_ERR_NO_ERROR) {
                    if (cmd == TAS_PL0_CMD_RDBLK) {
                        if (pkt->wlrd > wlrwNoErr) {
                            return mSetPktRspErrConnectionProtocol();
                        }
                        pktRsp->num_bytes_ok = pkt->wlrd * 4;
                    }
                    else {
                        if (pkt->wlrd != 0) {
                            return mSetPktRspErrConnectionProtocol();
                        }
                        pktRsp->num_bytes_ok = 0;
                    }
                    pktRsp->pl_err = pkt->err;
                    mSetPktRspErrPl0Data(pktRsp->pl_err, TAS_ERR_RW_READ, pt->addr + pkt->wlrd * 4, pt->addr_map);
                }
                else {  // TAS_PL0_ERR_NO_ERROR
                    if ((pkt->wlrd != (0xFF & (pt->num_bytes + 3) / 4)) || (pkt->wl != pkt->wlrd)) {
                        return mSetPktRspErrConnectionProtocol();
                    }
                    else {
                        pktRsp->num_bytes_ok = pt->num_bytes;
                        pktRsp->pl_err = TAS_PL0_ERR_NO_ERROR;
                    }
                }
                memcpy(pt->rdata, &rsp[wi + 1], pktRsp->num_bytes_ok);
                wi += 1 + wl;
            }
            iTrans++;
        }
        else {
            return mSetPktRspErrConnectionProtocol();
        }
    }
    assert(wi == wiMax);

    return mEip->tas_err;
}

uint32_t CTasPktHandlerRw::rw_get_trans_rsp(const tas_rw_trans_rsp_st** trans_rsp)
{
    assert(mPl0Trans[0].addr == mRwTrans[0].addr);

    uint64_t addrStart = 0;
    uint64_t addrEnd = 0;
    uint64_t addrNextPl0 = 0;
    uint32_t t = 0;
    mRwTransRsp[t].pl_err = TAS_PL0_ERR_NO_ERROR;
    mRwTransRsp[t].num_bytes_ok = 0;
    for (uint32_t p = 0; p < mPl0NumTrans; p++) {
        assert(mPl0Trans[p].type == mRwTrans[t].type);
        if (mPl0Trans[p].addr == mRwTrans[t].addr) {
            addrStart = mRwTrans[t].addr;
            addrEnd   = addrStart + mRwTrans[t].num_bytes;
        }
        else {
            assert(mPl0Trans[p].addr == addrNextPl0);
        }
        addrNextPl0 = mPl0Trans[p].addr + mPl0Trans[p].num_bytes;  // In case an RW trans was split

        if (mRwTransRsp[t].pl_err == TAS_PL0_ERR_NO_ERROR) {  // Only up to first error
            mRwTransRsp[t].pl_err = mPl0TransRsp[p].pl_err;
            assert(mPl0TransRsp[p].num_bytes_ok <= mPl0Trans[p].num_bytes);
            if (mPl0TransRsp[p].pl_err == TAS_PL0_ERR_NO_ERROR)
                assert(mPl0TransRsp[p].num_bytes_ok == mPl0Trans[p].num_bytes);
            mRwTransRsp[t].num_bytes_ok += mPl0TransRsp[p].num_bytes_ok;
        }

        if (addrNextPl0 >= addrEnd) {
            assert(addrNextPl0 == addrEnd);
            t++;
            if (t == mRwNumTrans)
                break;  // for p
            mRwTransRsp[t].pl_err = TAS_PL0_ERR_NO_ERROR;
            mRwTransRsp[t].num_bytes_ok = 0;
        }
    }

    *trans_rsp = mRwTransRsp;
    return mRwNumTrans;
}

uint32_t CTasPktHandlerRw::rw_get_pl0_trans(const tas_rw_trans_st** pl0_trans, const tas_rw_trans_rsp_st** pl0_trans_rsp) const
{
    *pl0_trans     = mPl0Trans;
    *pl0_trans_rsp = mPl0TransRsp;
    return mPl0NumTrans;
}

tas_return_et CTasPktHandlerRw::mSetPktRspErrPl1Cnt()
{
    assert(false);
    snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Server connection protocol (PL1 packet count)");
    mEip->tas_err = TAS_ERR_SERVER_CON;
    return mEip->tas_err;
}

void CTasPktHandlerRw::mSetPktRspErrPl0Data(uint8_t pl_err, tas_return_et tas_err, uint64_t addr, uint8_t addr_map)
{
    if (mEip->tas_err != TAS_ERR_NONE) {
        return;  // Capture the first one
    }
    assert(mEip->info[0] == 0);

    const char* typeStr = (tas_err == TAS_ERR_RW_READ) ? "Read" : "Write";

    std::array<char, 32> addrMapStr;
    addrMapStr[0] = 0;
    if (addr_map > 0)
        snprintf(addrMapStr.data(), addrMapStr.size(), "in addr_map %d ", addr_map);

    if (pl_err == TAS_PL0_ERR_DATA) {
        assert((tas_err == TAS_ERR_RW_READ) || (tas_err == TAS_ERR_RW_WRITE));
        mEip->tas_err = tas_err;
        snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: %s of addr %" PRIX64 " %sfailed", typeStr, addr, addrMapStr.data());
    }
    else if (pl_err == TAS_PL0_ERR_DEV_LOCKED) {
        mEip->tas_err = TAS_ERR_DEVICE_LOCKED;
        snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Device is locked");
    }
    else if (pl_err == TAS_PL0_ERR_DEV_ACCESS) {
        mEip->tas_err = TAS_ERR_DEVICE_ACCESS;
        snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Device access failed");
    }
    else if (pl_err == TAS_PL0_ERR_ACC_MODE) {
        mEip->tas_err = tas_err;
        snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: Set acc_mode not supported for %s of addr %" PRIX64 " %s", typeStr, addr, addrMapStr.data());
    }
    else if (pl_err == TAS_PL0_ERR_ADDR_MAP) {
        mEip->tas_err = tas_err;
        snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: addr_map %d not supported", addr_map);
    }
    else if (pl_err == TAS_PL0_ERR_ADDR_BLOCKED) {
        mEip->tas_err = tas_err;
        snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: %s of addr %" PRIX64 " %swas blocked", typeStr, addr, addrMapStr.data());
    }
    else {
        mEip->tas_err = tas_err;
        snprintf(mEip->info, TAS_INFO_STR_LEN, "ERROR: %s of addr %" PRIX64 " %sreturned code 0x%2.2X", typeStr, addr, addrMapStr.data(), pl_err);
        assert(false);
    }
}
