# detect-firmware
This is the source for the detect sensor firmware.

**TODO
This repo is huge! Remove all unnecessary code.

## Compiling
Using nRF5 SDK V15.0.0 (included in source) found [here](http://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/)

GNU Arm Embedded Toolchain - Version 6-2017-q2-update Linux 64-bit
[Download tar](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)

Need to update GNU_INSTALL_ROOT in 

'/nordic_nRF5/components/toolchain/gcc/Makefile.posix' for Linux 
or  
'/nordic_nRF5/components/toolchain/gcc/Makefile.windows' for Windows*

## Programming
Using nrfjprog utlilty found [here](https://www.nordicsemi.com/eng/Products/nRF52840)

## Programmer / Debugger
Using J-Link Plus found [here](https://www.segger.com/products/debug-probes/j-link/models/j-link-plus/)

Using JTAG 20 pin 0.1 inch to 10 pin 0.05 inch adapter found [here](https://www.olimex.com/Products/ARM/JTAG/ARM-JTAG-20-10/)  

Using J-Link Software found [here](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)

## RTT Logging
RTT Logging is enabled and can be read using the cmdline utility JLinkRTTLogger with a J-Link Debugger.  
cmdline options:  
```
Device name. Default: NRF52832_XXAA > NRF52840_XXAA
Target interface. > SWD
Interface speed [kHz]. Default: 4000 kHz > 
RTT Control Block address. Default: auto-detection > 
RTT Channel name or index. Default: channel 1 > 0
Output file. Default: RTT_<ChannelName>_<Time>.log > /dev/stdout
```

## BLE Services

Detect Configuration Service
------
| Name                            | UUID                                 | Type                 | Data             | Description                  | 
| -------                         | ----------------------               | -------------------- | -------          | ------------                 | 
| Base UUID                       | EF68xxxx-9B35-4933-9B10-52FFA9740042 |                      |                  |                              | 
| Detect configuration service    | 0100                                 |                      |                  |                              | 
| Device name characteristic      | 0101                                 | Write/Read           | max 10 bytes     | Device name as ASCII string  | 
| Advertising param characteristic| 0102                                 | Read                 | 3 bytes          | <ul><li>uint8_t - major </li><li> uint8_t - minor </li><li> uint8_t - patch </li></ul>  |
| Connection param characteristic | 0103                                 | Read                 | 3 bytes          | <ul><li>uint8_t - major </li><li> uint8_t - minor </li><li> uint8_t - patch </li></ul>  |
| Firmware Version                | 0104                                 | Read                 | 3 bytes          | <ul><li>uint8_t - major </li><li> uint8_t - minor </li><li> uint8_t - patch </li></ul>  |
