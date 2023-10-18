# Bootloader for STM32G0B1RE
Implementation of a bootloader for STM32G0B1RE, placed in the first address of the FLASH memory (0x0800 0000) while the applications (max 2) uploaded will be placed in the second sector of the FLASH memory.  
This bootloader aim to demonstrate how a firmware update could be performed in a secure manner through Bluetooth Low Energy, in this case we used the module X-NUCLEO-IDB05A1. I do not recommend to use this BLE module for this type of project since it does not support Data Length Extension (DLE), hence the maximum payload size available at Application Layer is 20 bytes.

## FSM STATES - BOOTING LOGIC 
![Frame 10 (2)](https://github.com/francescoolivieri/BLE-Firmware-Update-STM32G0B1RE/assets/113623927/d24426d0-c5a1-415d-8a8b-da7c5238e0ff)

### FSM STATE - FIRMWARE UPLOAD
When the system is performing a firmware upload (the peripheral is receveing a new firmware, so downloading for its point of view), it will follow the communication protocol descripted **here**.  
In case of error in the transmission it will delete all the code received and check for new authorized connections.
