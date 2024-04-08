#!/usr/bin/env python3
#
#  Copyright (c) 2024 Infineon Technologies AG.
#
#  This file is part of TAS Client, an API for device access for Infineon's 
#  automotive MCUs. 
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  ****************************************************************************************************************#
from PyTAS.PyTAS import TasReturn, TasClientRw, TasRwTransaction, TasRwTransType
import unittest

class TestTasClientRw(unittest.TestCase):
    BASE_ADDRESS = 0x70100000
    READ_BYTE_SIZE = 8
    DATA_2_WRITE = "BEBACAFECAFEBEBABEBABEBA"
    CLIENT_NAME = "TestRWclient"

    def get_session():
        c = TasClientRw(TestTasClientRw.CLIENT_NAME)
        ret = c.server_connect()
        assert ret == TasReturn.TAS_ERR_NONE
        targets, _ = c.get_targets()
        assert len(targets) > 0, 'No target found'
        ret = c.session_start(targets[0].identifier)
        assert ret == TasReturn.TAS_ERR_NONE

        return c


    def test_read8(self):
        c = TestTasClientRw.get_session()

        _ , ret = c.read8(self.BASE_ADDRESS)
        assert ret == TasReturn.TAS_ERR_NONE
        pass

    def test_read16(self):
        c = TestTasClientRw.get_session()

        _ , ret = c.read16(self.BASE_ADDRESS)
        assert ret == TasReturn.TAS_ERR_NONE
        pass

    def test_read32(self):
        c = TestTasClientRw.get_session()

        _ , ret = c.read32(self.BASE_ADDRESS)
        assert ret == TasReturn.TAS_ERR_NONE
        pass

    def test_read64(self):
        c = TestTasClientRw.get_session()

        value, ret = c.read64(self.BASE_ADDRESS)
        assert ret == TasReturn.TAS_ERR_NONE
        pass

    def test_read(self):
        c = TestTasClientRw.get_session()

        value, ret = c.read(self.BASE_ADDRESS, self.READ_BYTE_SIZE)
        assert ret == TasReturn.TAS_ERR_NONE
        assert len(value) == self.READ_BYTE_SIZE
        pass
    
    def test_write8(self):
        c = TestTasClientRw.get_session()

        ret = c.write8(self.BASE_ADDRESS, 0xFF)
        assert ret == TasReturn.TAS_ERR_NONE
        pass

    def test_write16(self):
        c = TestTasClientRw.get_session()

        ret = c.write16(self.BASE_ADDRESS, 0xFF00)
        assert ret == TasReturn.TAS_ERR_NONE
        pass

    def test_write32(self):
        c = TestTasClientRw.get_session()

        ret = c.write32(self.BASE_ADDRESS, 0xF0F0F0F0)
        assert ret == TasReturn.TAS_ERR_NONE
        pass

    def test_write64(self):
        c = TestTasClientRw.get_session()

        ret = c.write64(self.BASE_ADDRESS, 0x0F0F0F0F0F0F0F0F)
        assert ret == TasReturn.TAS_ERR_NONE
        pass

    def test_write(self):
        c = TestTasClientRw.get_session()

        data = bytes.fromhex(self.DATA_2_WRITE)

        ok_bytes, ret = c.write(self.BASE_ADDRESS, data)
        assert ok_bytes == len(data)
        assert ret == TasReturn.TAS_ERR_NONE
        pass

    def test_read_write(self):
        c = TestTasClientRw.get_session()

        data = bytes.fromhex(self.DATA_2_WRITE)

        ok_bytes, ret = c.write(self.BASE_ADDRESS, data)
        assert ok_bytes == len(data)
        assert ret == TasReturn.TAS_ERR_NONE

        value, ret = c.read(self.BASE_ADDRESS, len(data))
        assert ret == TasReturn.TAS_ERR_NONE
        assert len(value) == len(data)
        assert value == data
        pass

    def test_unaligned_write_read(self):
        c = TestTasClientRw.get_session()

        value = 0xEB33 #0b11101011 0b00110011

        for offset in range(5):
            read_mask = 0xFFFFFFFF << (8*offset)

            ret = c.write16(self.BASE_ADDRESS + offset,        value)
            assert ret == TasReturn.TAS_ERR_NONE
            ret = c.write16(self.BASE_ADDRESS + offset + 2,    value)
            assert ret == TasReturn.TAS_ERR_NONE
            read_value, ret = c.read64(self.BASE_ADDRESS)

            assert ret == TasReturn.TAS_ERR_NONE

            expected_value = (value << (8*offset)) | (value << (8*(offset + 2)))

            assert (read_value & read_mask) == expected_value
        pass


    def test_fill32(self):
        c = TestTasClientRw.get_session()
        data_pattern = 0xBEBACAFE
        num_bytes = 8

        data = bytes(num_bytes*2)
        ok_bytes, ret = c.write(self.BASE_ADDRESS, data)
        assert ok_bytes == len(data)

        ret = c.fill32(self.BASE_ADDRESS, data_pattern, num_bytes)
        assert ret == TasReturn.TAS_ERR_NONE

        value , ret = c.read32(self.BASE_ADDRESS)
        assert ret == TasReturn.TAS_ERR_NONE
        assert data_pattern == value

        value , ret = c.read32(self.BASE_ADDRESS+4)
        assert ret == TasReturn.TAS_ERR_NONE
        assert data_pattern == value

        value , ret = c.read64(self.BASE_ADDRESS+8)
        assert ret == TasReturn.TAS_ERR_NONE
        assert 0 == value

        pass

    def test_fill64(self):
        c = TestTasClientRw.get_session()
        data_pattern = 0xBEBACAFEDEADBEEF
        num_bytes = 16

        data = bytes(num_bytes*2)
        ok_bytes, ret = c.write(self.BASE_ADDRESS, data)
        assert ok_bytes == len(data)

        ret = c.fill64(self.BASE_ADDRESS, data_pattern, num_bytes)
        assert ret == TasReturn.TAS_ERR_NONE

        value , ret = c.read64(self.BASE_ADDRESS)
        assert ret == TasReturn.TAS_ERR_NONE
        assert data_pattern == value

        value , ret = c.read64(self.BASE_ADDRESS+8)
        assert ret == TasReturn.TAS_ERR_NONE
        assert data_pattern == value

        value , ret = c.read64(self.BASE_ADDRESS+16)
        assert ret == TasReturn.TAS_ERR_NONE
        assert 0 == value
        pass

    def test_execute_transaction(self):
        c = TestTasClientRw.get_session()
        data_pattern = 0xBEBACAFEDEADBEEF

        trans = list()
        t1 = TasRwTransaction()
        t1.addr = self.BASE_ADDRESS
        t1.data = bytes.fromhex(self.DATA_2_WRITE)
        t1.num_bytes = len(t1.data)
        t1.type = TasRwTransType.TAS_RW_TT_WR
        trans.append(t1)

        t2 = TasRwTransaction()
        t2.addr = self.BASE_ADDRESS
        t2.num_bytes = t1.num_bytes
        t2.type = TasRwTransType.TAS_RW_TT_RD
        t2.data = bytes()
        trans.append(t2)

        t3 = TasRwTransaction()
        t3.addr = self.BASE_ADDRESS + 16
        t3.num_bytes = 16
        t3.type = TasRwTransType.TAS_RW_TT_FILL
        t3.data = data_pattern.to_bytes(8, 'little')
        trans.append(t3)

        t4 = TasRwTransaction()
        t4.addr = self.BASE_ADDRESS + 24
        t4.num_bytes = 8
        t4.type = TasRwTransType.TAS_RW_TT_RD
        t4.data = bytes()
        trans.append(t4)


        trans, ret = c.execute_trans(trans)
        assert ret == TasReturn.TAS_ERR_NONE, 'Error while executing execute_trans'

        value, ret = c.read(self.BASE_ADDRESS, t1.num_bytes)
        assert ret == TasReturn.TAS_ERR_NONE, 'Read error'

        assert value == t1.data, 'Write instruction in execute transaction failed'
        assert trans[1].data == t1.data, 'Read instruction failed in execute transaction'

        value, ret = c.read64(self.BASE_ADDRESS + 16)
        assert ret == TasReturn.TAS_ERR_NONE, 'Read error'
        assert value == data_pattern
        assert trans[3].data == t3.data

        value, ret = c.read64(self.BASE_ADDRESS + 24)
        assert ret == TasReturn.TAS_ERR_NONE, 'Read error'
        assert value == data_pattern

        pass


if __name__ == '__main__':
    unittest.main()
