<!--
 Copyright (c) 2025 Infineon Technologies AG.

 This file is part of TAS Client, an API for device access for Infineon's 
 automotive MCUs. 

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ****************************************************************************************************************-->
Tool Access Socket (TAS) Client API {#mainpage}
===============================================================================

The Tool Access Socket (TAS) Client API can be used in line with Infineon's Microcontroller Starter Kits, Application 
Kits and DAP miniWiggler. All access is done through a TAS server, which can be downloaded and installed with the latest
release of [DAS](https://www.infineon.com/DAS). Applications using the TAS Client API are then able to communicate whit
the installed TAS server. In case of any breaking changes to the TAS protocol, the TAS server will be updated accordingly.

A Tool access hardware (JTAG, DAP, SPD, SWD) can be found here: [DAP miniWiggler](https://www.infineon.com/cms/en/product/evaluation-boards/kit_miniwiggler_3_usb/?redirId=54610) 

What you are reading is a TAS Client API reference documentation, which does not cover a design of debug tools, but 
gives an overview of the API and it's usage. The following subpages go in more detail in respect to different types of 
clients supported by the TAS architecture.

- @ref Read_Write_API
- @ref Channel_API
- @ref Trace_API

In addition to the above descriptions, the repository includes demo projects, which demonstrate the basic usage of each
API.
