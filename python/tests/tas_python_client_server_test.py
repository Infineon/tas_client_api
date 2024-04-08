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
from PyTAS.PyTAS import TasClientRw, TasReturn, TasServerInfo, TasTargetInfo
import numpy as np
import unittest


class TestTasClientServer(unittest.TestCase):
    SERVER_IP = "localhost"
    SERVER_PORT = 24817
    SERVER_NAME = "TasServer"
    SERVER_V_MIN = 0
    SERVER_V_MAJ = 1
    CLIENT_NAME = "serverTester"

    def test_connect_successful(self):
        c = TasClientRw(self.CLIENT_NAME)
        ret = c.server_connect(self.SERVER_IP, self.SERVER_PORT)
        assert ret == TasReturn.TAS_ERR_NONE, f'Check if the server is running on {self.SERVER_IP}:{self.SERVER_PORT}'
        pass

    def test_connect_unseccessful(self):
        c = TasClientRw(self.CLIENT_NAME)

        #Wrong port
        ret = c.server_connect(port_num=10)
        assert ret == TasReturn.TAS_ERR_SERVER_CON, 'Should not have connected to the server, wrong PORT'

        #Wrong ip
        ret = c.server_connect(ip_addr="192.168.1.10")
        assert ret == TasReturn.TAS_ERR_SERVER_CON, 'Should not have connected to the server, wrong IP'
        pass

    def test_get_ip_address(self):
        c = TasClientRw(self.CLIENT_NAME)
        c.server_connect(self.SERVER_IP, self.SERVER_PORT)
        ip = c.get_server_ip_addr()
        assert ip == self.SERVER_IP, 'Different IP address to the one connected to'
        pass

    def test_get_server_port(self):
        c = TasClientRw(self.CLIENT_NAME)
        c.server_connect(self.SERVER_IP, self.SERVER_PORT)
        port = c.get_server_port_num()
        assert port == self.SERVER_PORT, 'Different port to one connected to'
        pass

    def test_get_server_info(self):
        c = TasClientRw(self.CLIENT_NAME)
        c.server_connect(self.SERVER_IP, self.SERVER_PORT)
        info = c.get_server_info()

        assert isinstance(info, TasServerInfo)
        assert isinstance(info.server_name, str)
        assert isinstance(info.v_minor, int)
        assert isinstance(info.v_major, int)
        assert isinstance(info.date, str)
        assert isinstance(info.supp_protoc_ver, int)
        assert isinstance(info.supp_chl_target, int)
        assert isinstance(info.supp_trc_type, int)
        assert isinstance(info.start_time_us, int)
        pass

    def test_get_targets(self):
        c = TasClientRw(self.CLIENT_NAME)
        ret = c.server_connect(self.SERVER_IP, self.SERVER_PORT)

        targets, ret = c.get_targets()

        assert isinstance(ret, TasReturn)
        assert ret == TasReturn.TAS_ERR_NONE, 'Error getting the targets'
        assert isinstance(targets, list)
        assert len(targets)>0, 'No connected target found'
        assert isinstance(targets[0], TasTargetInfo)
        pass

    def test_connect_to_target(self):
        c = TasClientRw(self.CLIENT_NAME)
        ret = c.server_connect(self.SERVER_IP, self.SERVER_PORT)
        targets, _ = c.get_targets()

        assert len(targets)>0, 'No connected target found'

        ret = c.session_start(targets[0].identifier)

        assert ret == TasReturn.TAS_ERR_NONE, 'Error while connecting to the target'
        pass

    def test_get_target_clients(self):
        c = TasClientRw(self.CLIENT_NAME)
        ret = c.server_connect(self.SERVER_IP, self.SERVER_PORT)
        targets, _ = c.get_targets()

        assert len(targets)>0, 'No connected target found'

        ret = c.session_start(targets[0].identifier)

        assert ret == TasReturn.TAS_ERR_NONE

        clients, session_name, start_time, ret = c.get_target_clients(targets[0].identifier)

        assert ret == TasReturn.TAS_ERR_NONE
        assert isinstance(start_time, int)
        assert isinstance(session_name, str)
        assert isinstance(clients, list)

        found_me = False

        for client in clients:
            if client.client_name == self.CLIENT_NAME:
                found_me = True
        assert found_me, 'Username belonged to the using client not found'
        pass

if __name__ == '__main__':
    unittest.main()
