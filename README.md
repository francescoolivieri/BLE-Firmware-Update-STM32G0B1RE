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
This protocol implements a Cumulative Acknowledgment mechanism with a cumulative block size of 16 packets. This choice was made to address two important topics: the limitation of 20-byte size packets and the reduction of total messages in the communication (less energy in the communication required).  
The communication process begins with a request from the master of the connection to the slave (periperhal in our case), based on the type of request the communication will take a specific path with its own purpose which in our case is performing a secure firmware update.

<p align="center">
  <img width="300" height="386.55" src="https://github.com/francescoolivieri/BLE-Firmware-Update-STM32G0B1RE/assets/113623927/7c4f8c41-1cfe-498a-b417-5d3cdce43624">
   </p>

**FW UPLOAD REQ** -> Firmware Upload Request  
**ACK** -> Firmware Packet Response  
**NEW_FW\[x\]** -> Firmware Packet   

See [here](#packet-types) how those packets are built.

#### Security
To secure this communication I chose the AES GCM AEAD, an authenticated encryption which provides data confidentiality, integrity, and availability.  
In the packets "Firmware Packet" ad "Firmware Packet Response" you can find a 4-byte length field called "TAG", which is the part of the protocol that provide the authentication on the whole packet (headers included).  
In the "Firmware Packet Response" there are is a 3-byte length field called "RAND_NUM", which are used by both master and slave to modify their IV in synchrony enabling to have different ciphertext even when sending the same firmware over-the-air repeatedly.

#### Packet Types
<p align="center">
  <img width="600" height="243.41" src="https://github.com/francescoolivieri/BLE-Firmware-Update-STM32G0B1RE/assets/113623927/e2b30521-f13a-4686-9547-fae5772a2094">
</p>

### FSM STATES - COMMUNICATION LOGIC
This is final state machine summarizes how I decided to implement the logic flow of the communication when receveing new firmware. 

<p align="center">
  <img width="600" height="125" src="https://github.com/francescoolivieri/BLE-Firmware-Update-STM32G0B1RE/assets/113623927/d09983fa-91b0-4000-9def-7a66b43a3570">
</p>

- **Idle:** the device is advertising, no connection has been established and no message is expected
- **Connected:** a connection is established, and the device is ready to receive and process incoming requests. An important check is done on the correctness of the request and on new firmware size constraints.
- **Receveing Firmware:** upon receiving and acknowledging a valid Firmware Upload Request, the peripheral enters this state. The device is now ready to receive firmware packets and send firmware packet response.
- **Closing Connection:** after the successful receipt and verification of the new firmware, the device enters this state. At this point, the device terminates the connection and jumps to the newly received firmware stored in flash memory
