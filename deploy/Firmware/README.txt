
SBL
===============
- The serial bootloader is based on the SBL project available in the TI Zigbee Stack - Z-Stack Home 1.2.2a.44539\Projects\zstack\Utilities\BootLoad\CC2538\Boot.eww
- The bootloader project contains a compiler switch PANL_SMART_LIVING, removing it will compile the bootloader for the SmartRF06+CC2538EM board
- The default boot time is 27s - i.e. the bootloader will wait for 27s for any character over UART before switching to application mode. This has been reduced to around 7s.
- The bootloader is located in the range - 0x0027B000 - 0x0027C800
- The application valid flag is stored in the first page of memory - so this must be erased first when performing a software download to ensure that any inadvertent termination of the process will result in an "invalid" image.

Tools
===============
sbl_tool.bin - Linux program to update firmware, input argument is the path to the 492KB application image file to update
ZNP-PSL-SBLv2.7.0.bin (512kB) - This is a combined image with ZNP and BL firmware - suitable for factory. This file is generated using the command:

copy /b ZNP-PSL-v2.7.bin + Boot-7s-boot.bin ZNP-PSL-SBLv2.7.0.bin
	- This image can be programmed using the SmartRF Programmer tool from TI, start address is 0x00200000 

These two additional images are available for those who want to load the application using the host program:

Boot-7s-boot.bin (20KB)  - bootloader binary to be programmed at 0x0027B000 
ZNP-PSL_v2.7.bin (492kB) - ZNP firmware compiled to be compatible with the bootloader, it can be loaded to a module flashed with Boot-7s-boot.bin using the sbl_tool.bin program.

NOTE
========
- when booting up the 512kB combined image for the *first* time, the module will take upto 30s to calculate and verify the checksum and mark the application as valid before entering applicatiom mode.
- After this first boot, the bootloader will switch to application in around 7 seconds.