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

print("TAS API channels demo\n")

# Create an instance of TAS Client Chl 
client = TasClientChl("DemoClientChl")

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
print()

client2 = TasClientChl("DemoClientChl2")

ret = client2.server_connect("localhost")
if (ret != TAS_ERR_NONE):
    print(f"Failed to connect to the server, {client2.get_error_info()}")
    # other error processing code
    exit() # Fatal

ret = client2.session_start(targets[0].identifier, "DemoSession")
if (ret != TAS_ERR_NONE):
    print(f"Failed to start a session, {client2.get_error_info()}\n")
    # other error processing code
    exit()  # Fatal


# Bidirectional client
chl = 5
channel_type = TAS_CHT_BIDI
con_mode = TAS_CHSO_DEFAULT

ret = client.subscribe(chl, channel_type, con_mode)
assert ret == TAS_ERR_NONE

s_msg = "Hello World! PyMCDS demo"
ret = client.send_string(s_msg)
assert ret == TAS_ERR_NONE
print(f"Message sent to chl {chl}: {s_msg}")

r_msg, init, ret = client.rcv_string(1000)
assert ret == TAS_ERR_NONE, client.get_error_info()
print(f"Message received from chl {chl}: {r_msg}")

s_bmsg = (0xBEBACAFE).to_bytes(4, 'little') 
ret = client.send_msg(s_bmsg)
assert ret == TAS_ERR_NONE
print(f"Message sent to chl {chl}: {s_bmsg}")

r_msg, init, ret = client.rcv_msg(1000)
assert ret == TAS_ERR_NONE
print(f"Message received from chl {chl}: {r_msg}")

ret = client.unsubscribe()
assert ret == TAS_ERR_NONE


# Single direction clients
chl = 3

ret = client.subscribe(chl, TAS_CHT_SEND, con_mode)
assert ret == TAS_ERR_NONE

ret = client2.subscribe(chl, TAS_CHT_RCV, con_mode)
assert ret == TAS_ERR_NONE

s_msg = "Hello World!"
ret = client.send_string(s_msg)
assert ret == TAS_ERR_NONE
print(f"Message sent: {s_msg}")

r_msg, init, ret = client2.rcv_string(1000)
assert ret == TAS_ERR_NONE
print(f"Message received: {r_msg[:-1]}")


r_msg, init, ret = client.rcv_msg(1000)
assert ret == TAS_ERR_FN_USAGE

s_msg = "Hello World!"
ret = client2.send_string(s_msg)
assert ret == TAS_ERR_FN_USAGE