# Bootloader for STM32G0B1RE
Implementation of a bootloader for STM32G0B1RE, placed in the first address of the FLASH memory (0x0800 0000) while the applications (max 2) uploaded will be placed in the second sector of the FLASH memory.  
This bootloader aim to demonstrate how a firmware update could be performed in a secure manner through Bluetooth Low Energy, in this case we used the module X-NUCLEO-IDB05A1. I do not recommend to use this BLE module for this type of project since it does not support Data Length Extension (DLE), hence the maximum payload size available at Application Layer is 20 bytes.  

⚠️**NOTICE⚠️ :**  This version is built to work esclusively with a STM32G0B1RE device, however it could be easily modified to fit others MCU.

## FSM STATES - BOOTING LOGIC 
![Frame 10 (2)](https://github.com/francescoolivieri/BLE-Firmware-Update-STM32G0B1RE/assets/113623927/d24426d0-c5a1-415d-8a8b-da7c5238e0ff)

### FSM STATE - FIRMWARE UPLOAD
When the system is performing a firmware upload (the peripheral is receveing a new firmware, so downloading for its point of view), it will follow the communication protocol descripted **here**.  
In case of error in the transmission it will delete all the code received and check for new authorized connections.

### FSM STATE - SETUP BLE
To perform a communication between the host and slave of the communication I decided to build an ad-hoc Service with two charateristics: TX-Characteristic and RX-Characteristic.

<p align="center">
  <img width="300" height="422.63" src="https://github.com/francescoolivieri/BLE-Firmware-Update-STM32G0B1RE/assets/113623927/18513506-233a-4f55-b607-8f7ba2e2b44d">
</p>

-  **TX-Characteristic:** used by the master to transmit information to the device. This characteristic is disposed of the Notify property, this enables the master to send data to the slave device without the latter's explicit request. One of the key features of the notify property is the one-way communication, no response is required. This is useful since we are using a cumulative ACK protocol (see [here](https://github.com/francescoolivieri/BLE-Firmware-Update-STM32G0B1RE/blob/main/README.md#communication-protocol) for more detail about the communication protocol).
-  **RX-Characteristic:** used by the slave to transmit information to the master. This characteristic has the Write Without Response and Write property that enables the slave to send packets without receveing the response on the same characteristic.
