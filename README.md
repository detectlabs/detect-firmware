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
| Base UUID                       | EE84xxxx-43B7-4F65-9FB9-D7B92D683E36 |                      |                  |                              | 
| Detect configuration service    | 0100                                 |                      |                  |                              | 
| Device name characteristic      | 0101                                 | Write/Read           | max 10 bytes     | Device name as ASCII string  | 
| Advertising param characteristic| 0102                                 | Write/Read           | 3 bytes          | Advertising parameters (in units):  <ul><li>uint16_t - Adv interval in ms (unit 0.625 ms).</li><ul><li>min 32 -> 20 ms </li></ul><ul><li>max 8000 -> 5 s </li></ul></ul><ul><li>uint8_t - Adv timeout in s (unit 1 s).</li><ul><li>min 0 -> 0 s</li></ul><ul><li>max 180 s -> 3 min</li></ul></ul>  |
| Connection param characteristic | 0103                                 | Write/Read           | 8 bytes          | Connection parameters:  <ul><li>uint16_t - Min connection interval (unit 1.25 ms).</li><ul><li>min 6 -> 7.5 ms</li></ul><ul><li>max 3200 -> 4 s</li></ul></ul><ul><li>uint16_t - Max connection interval (unit 1.25 ms).</li><ul><li>min 6 -> 7.5 ms</li></ul><ul><li>max 3200 -> 4 s</li></ul></ul><ul><li>uint16_t - Slave latency (number of connection events).</li><ul><li>Range 0-499</li></ul></ul><ul><li>uint16_t - Supervision timeout (unit 10 ms).</li><ul><li>Min 10 -> 100 ms</li></ul><ul><li>Max 3200 -> 32 s</li></ul></ul>  The following constraint applies: conn_sup_timeout * 4 > (1 + slave_latency) * max_conn_interval that corresponds to the following Bluetooth Spec requirement: The Supervision_Timeout in milliseconds must be larger than (1 + Conn_Latency) * Conn_Interval_Max * 2, where Conn_Interval_Max is given in milliseconds.  |
| Firmware Version                | 0104                                 | Read                 | 3 bytes          | <ul><li>uint8_t - major </li><li> uint8_t - minor </li><li> uint8_t - patch </li></ul>  |

Detection Service
------
| Name                            | UUID                                 | Type                 | Data             | Description                  | 
| -------                         | ----------------------               | -------------------- | -------          | ------------                 | 
| Base UUID                       | EE84xxxx-43B7-4F65-9FB9-D7B92D683E36 |                      |                  |                              | 
| Detection service               | 0200                                 |                      |                  |                              | 
| Presence characteristic         | 0201                                 | Notify               | 8 bytes          | IR Sensors (unit pA):  <ul><li>int16_t - IR1</li><li>int16_t - IR2</li><li>int16_t - IR3</li><li>int16_t - IR4</li></ul>  |
| Range characteristic            | 0202                                 | Notify               | 2 bytes          | Ranger (unit mm):  <ul><li>uint16_t - mm</li></ul>  |
| Configuration characteristic    | 0203                                 | Write/Read           | 13 bytes         | <ul><li>uint16_t - Presence Interval in ms (20 ms - 200ms).</li></ul><ul><li>uint16_t - Range Interval in ms (20 ms - 200ms).</li></ul><ul><li> Presence Threshold Level</li><ul><li>int16_t - ETH13H [-2048 - 2047]</li><li>int16_t - ETH13L [-2048 - 2047]</li><li>int16_t - ETH24H [-2048 - 2047]</li><li>int16_t - ETH24L [-2048 - 2047]</li></ul></ul><ul><li>uint8_t - Sample Mode</li><ul><li>0 = Continuous - The presence and range sensor are not tied together, and streaming (notifying) will begin when characteristic notification is enabled.</li></ul><ul><li>1 = Motion Activated - When the threshold is passed on the presence sensor, both the presence and range sensor will begin streaming (notifying) at their set intervals if notify is enabled.</li></ul></ul>  |

Environment Service
------
| Name                            | UUID                                 | Type                 | Data             | Description                  | 
| -------                         | ----------------------               | -------------------- | -------          | ------------                 | 
| Base UUID                       | EE84xxxx-43B7-4F65-9FB9-D7B92D683E36 |                      |                  |                              | 

Motion Service
------
| Name                            | UUID                                 | Type                 | Data             | Description                  | 
| -------                         | ----------------------               | -------------------- | -------          | ------------                 | 
| Base UUID                       | EE84xxxx-43B7-4F65-9FB9-D7B92D683E36 |                      |                  |                              | 

Battery Service
------
| Name                            | UUID                                 | Type                 | Data             | Description                  | 
| -------                         | ----------------------               | -------------------- | -------          | ------------                 | 
| UUID                            | 180F                                 |  Notify/Read         | 1 byte           | <ul><li> uint8_t State of charge [%] (0-100) </li></ul> Uses a lookup table to convert from battery voltage to state of charge (SoC). Due to the ADC configuration and battery model, certain percentage values are skipped. Will only update/notify if there is a change in remaining battery level. | 

Secure DFU Service
------
| Name                            | UUID                                 | Type                 | Data             | Description                  | 
| -------                         | ----------------------               | -------------------- | -------          | ------------                 | 
| Base UUID                       | 0xFE59                               |                      |                  |                              | 

## Credit
Heavily Adapted from [Nordic-Thingy52-FW](https://github.com/NordicSemiconductor/Nordic-Thingy52-FW)