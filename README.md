# detect-firmware
This is the source for the detect sensor firmware.

**TODO
This repo is huge! Remove all unnecessary code.

## Compiling
Using nRF5 SDK V15.0.0 (included in source) found [here](http://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/)

GNU Arm Embedded Toolchain - Version 6-2017-q2-update Linux 64-bit
[Download tar](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)

Need to update GNU_INSTALL_ROOT in 

*'components/toolchain/gcc/Makefile.posix' for Linux 
or  
'components/toolchain/gcc/Makefile.windows' for Windows*

## Programming
Using nrfjprog utlilty found [here](http://linkhttps://www.nordicsemi.com/eng/Products/nRF52840)

## Programmer / Debugger
Using J-Link Plus found [here](http://linkhttps://www.segger.com/products/debug-probes/j-link/models/j-link-plus/)

Using JTAG 20 pin 0.1 inch to 10 pin 0.05 inch adapter found [here](https://www.olimex.com/Products/ARM/JTAG/ARM-JTAG-20-10/)  

Using J-Link Software found [here](http://linkhttps://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)
