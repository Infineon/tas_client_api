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
