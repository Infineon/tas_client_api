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
from PyTAS.PyTAS import *

print("TAS API rw demo\n")

# Create an instance of TAS Client RW 
client = TasClientRw("DemoClientRw")

# Connect to the server, provide IP address or localhost
ret = client.server_connect("localhost")
if (ret != TAS_ERR_NONE):
    print(f"Failed to connect to the server, {client.get_error_info()}")
    # other error processing code
    exit() # Fatal


# Print the server info
print()
server_info = client.get_server_info()
print("Server Info: ")

start_time = server_info.start_time_us / 1000000;  # Convert to seconds
print(f"Started at {start_time}\n")    

print(f"{server_info.server_name} V{server_info.v_major}.{server_info.v_minor} ({server_info.date})")

# Print the server IP address and its port number
print(f"Server IP:port: {client.get_server_ip_addr()}:{client.get_server_port_num()}")    


# Print the list of targets connected to the server
print()
targets, ret = client.get_targets()
if (ret != TAS_ERR_NONE):
    print(f"Failed to get the list of targets {client.get_error_info()}")
    # other error processing code
    exit()  # Fatal

print(f"Number of targets: {len(targets)}\n")

for i in range(len(targets)):
    print(f"Target {i}: {targets[i].identifier}")



# Connect to the first target from the list
print()
print("Connecting to the first target from the list...\n")
ret = client.session_start(targets[0].identifier, "DemoSession")
if (ret != TAS_ERR_NONE):
    print(f"Failed to start a session, {client.get_error_info()}\n")
    # other error processing code
    exit()  # Fatal


# Print con_info
con_info = client.get_con_info()

print(f"TARGET:")
print(f"\tidentifier: {con_info.identifier}")
print(f"\thash: {con_info.device_id_hash_str}")


# Reset the target
ret = client.device_connect(TAS_CLNT_DCO_RESET_AND_HALT)
if (ret != TAS_ERR_NONE):
    print(f"Failed to reset the device, {client.get_error_info()}")
    # other error processing code

# Basic read/write operations
base_addr = 0x70000000

print()
print("Basic read/write operations")  
addr = base_addr
print(f"\tRead - write - read 1 Byte at address {hex(addr)}")    
rd_data_8, ret = client.read8(addr)
assert ret == TAS_ERR_NONE
print(f"Read data: {hex(rd_data_8)}")    
value8 = 0xAB
print(f"Write {hex(value8)}")
assert client.write8(addr, value8) == TAS_ERR_NONE
rd_data_8, ret = client.read8(addr)
assert ret == TAS_ERR_NONE
print(f"Read back data: {hex(rd_data_8)}")

print()
addr = base_addr + 1
print(f"\tRead - write - read 1 Byte at address {hex(addr)}")   
rd_data_8, ret = client.read8(addr)
assert ret == TAS_ERR_NONE
print(f"Read data: {hex(rd_data_8)}")     
value8 = 0xCD
print(f"Write {hex(value8)}")
assert client.write8(addr, value8) == TAS_ERR_NONE
rd_data_8, ret = client.read8(addr)
assert ret == TAS_ERR_NONE
print(f"Read back data: {hex(rd_data_8)}")

# Reset the target
assert client.device_connect(TAS_CLNT_DCO_RESET_AND_HALT) == TAS_ERR_NONE

print()
addr = base_addr
print(f"\tRead - write - read 4 Bytes at address {hex(addr)}")
rd_data, ret = client.read32(addr)
print(f"Read data: {hex(rd_data)}")    
wr_data = 0xABCDEF09
print(f"Write {hex(wr_data)}")
assert client.write32(addr, wr_data) == TAS_ERR_NONE
rd_data, ret = client.read32(addr)
assert ret == TAS_ERR_NONE
print(f"Read back data: {hex(rd_data)}")


print()
addr = base_addr + 1
print(f"\tRead - write - read 4 Bytes at unaligned address {hex(addr)}")
rd_data, ret = client.read32(addr)
print(f"Read data: {hex(rd_data)}")    
wr_data = 0x12345678
print(f"Write {hex(wr_data)}")
assert client.write32(addr, wr_data) == TAS_ERR_NONE
rd_data, ret = client.read32(addr & 0xFFFFFFFE)
assert ret == TAS_ERR_NONE
rd_data1, ret = client.read32((addr & 0xFFFFFFFE) + 4)
assert ret == TAS_ERR_NONE
print(f"Read back data @{hex(addr & 0xFFFFFFFE)}: {hex(rd_data)}")
print(f"Read back data @{hex((addr & 0xFFFFFFFE) + 4)}: {hex(rd_data1)}")

# Reset the target
assert client.device_connect(TAS_CLNT_DCO_RESET_AND_HALT) == TAS_ERR_NONE

# Example fill 
print()
print("Fill data starting from target address")
addr = base_addr
# fill the memory with 0x123ABCD from 0x60000000 to 0x6000007C
assert client.fill32(addr, 0x1234ABCD, 128) == TAS_ERR_NONE
rd_data, ret = client.read(addr, 128) 
for i in range(int(128 / 4)):
    data = int.from_bytes(rd_data[i*4:(i*4)+4], "little")
    print(f"Data @{hex(addr | (i*4))}: {hex(data)}")

# Reset the target
assert client.device_connect(TAS_CLNT_DCO_RESET_AND_HALT) == TAS_ERR_NONE

# Example with transaction list
print()
print("Read - write - read with transaction list")
rd_data = 0
addr = 0x60000000
wr_data = 0xFFABCDEF

trans = list()
t1 = TasRwTransaction()
t1.addr = addr
t1.data = bytes()
t1.num_bytes = 4
t1.type = TasRwTransType.TAS_RW_TT_RD
trans.append(t1)

t2 = TasRwTransaction()
t2.addr = addr
t2.data = wr_data.to_bytes(4, byteorder="little")
t2.num_bytes = 4
t2.type = TasRwTransType.TAS_RW_TT_WR
trans.append(t2)

t3 = TasRwTransaction()
t3.addr = addr
t3.data = bytes()
t3.num_bytes = 4
t3.type = TasRwTransType.TAS_RW_TT_RD
trans.append(t3)



# Execute list
trans, ret = client.execute_trans(trans)
assert ret == TAS_ERR_NONE
print(f"Read transaction:  {trans[0].data}")
print(f"Write transaction: {trans[1].data}")
print(f"Read transaction:  {trans[2].data}")

# Reset the target
assert client.device_connect(TAS_CLNT_DCO_RESET) == TAS_ERR_NONE

# Destructor of clientRw will automatically end the session