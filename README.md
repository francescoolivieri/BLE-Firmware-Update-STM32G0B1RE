# BLE-Firmware-Update-STM32G0B1RE
Code for performing a firmware update through Bluetooth 4.2 (BLE), this project contains the code for both host (in this case simulated with an STM32G0B1RE) and peripheral side.

## Project Components
 - STM32G0B1RE
 - X-NUCLEO-IDB05A1
 - Computer equipped with Dual Mode Bluetooth

## Project layout

```
├── README.md  
├── BLE_FW_UPDATE       # folder containing code for STM32G0B1RE (peripheral side)
└── Python_GUI        	# folder containing code for GUI (host side) 
    
```
  
Inside each folder you will find more informations about that specific topic.

## Communication Protocol
An ad-hoc communication has been designed and implemented in order to rule the communication between the host and peripheral.  
This protocol could be easily be adapted to serve other purposes, by sending another type of command in the first field of the request (CMD) but in this project we are not interested in this.

### Communication Flow
This protocol implements an Cumulative Acknowledgment, with a cumulative block of 16 packets. This choice was taken to fight the poorness of payload (better performances) and the energy related topic (less message in the total communication). 
The communication starts with a request from the master of the connection to the slave (periperhal in our case), based on the type of request the communication will take a specific path with specific aim.

<p align="center">
  <img width="300" height="386.55" src="https://github.com/francescoolivieri/BLE-Firmware-Update-STM32G0B1RE/assets/113623927/7c4f8c41-1cfe-498a-b417-5d3cdce43624">
   </p>

#### Packet Types
<p align="center">
  <img width="600" height="243.41" src="https://github.com/francescoolivieri/BLE-Firmware-Update-STM32G0B1RE/assets/113623927/e2b30521-f13a-4686-9547-fae5772a2094">
</p>

### FSM STATES - COMMUNICATION LOGIC

<p align="center">
  <img width="600" height="125" src="https://github.com/francescoolivieri/BLE-Firmware-Update-STM32G0B1RE/assets/113623927/d09983fa-91b0-4000-9def-7a66b43a3570">
</p>
