/*
 * msg_handler.h
 *
 *  Created on: 24 apr 2023
 *      Author: Francesco Olivieri
 */

#ifndef INC_MESSAGE_HANDLER_H_
#define INC_MESSAGE_HANDLER_H_

#include <stdio.h>
#include "flash_manager.h"

/* ----- UTILS ----- */



/*
 * !!! IT SUMS ALSO THE "end" INDEX !!!
 */
uint8_t sum_payload(uint8_t *payload, uint8_t start, uint8_t end);



/* ----- PACKETS UTILS ----- */
// length in bytes
#define CHAT_DATA_LEN 20

#define PAYLOAD_LEN 16
#define SECURE_PAYLOAD_LEN 14

#define START_FLASH_MODE_CMD 0xbb
#define START_SECURE_FLASH_MODE_CMD 0xbc

#define WRITE_FLASH_PACKET_CMD 0xbb
#define WRITE_SECURE_FLASH_PACKET_CMD 0xc

#define ACK_CMD 0X0
#define NAK_CMD 0x1

#define START_FLASH_MODE_LEN 5
#define START_SECURE_FLASH_MODE_LEN 5

#define WRITE_FLASH_PACKET_RESPONSE_LEN 4
#define WRITE_SECURE_FLASH_PACKET_RESPONSE_LEN 10

#define AES_TAG_LEN 4

typedef enum{
	START_FLASH_MODE,
	START_SECURE_FLASH_MODE,
	WRITE_FLASH_PACKET,
	WRITE_SECURE_FLASH_PACKET,
	WRITE_FLASH_PACKET_RESPONSE,
	WRITE_SECURE_FLASH_PACKET_RESPONSE,
	MASTER_NAK
}PckType;

/*
 * Checks if the packet received is the one expected
 */
bool verify_data_type(uint8_t *data_buffer, uint8_t num_bytes, PckType expected_pck);


/* ----- CONNECTION UTILS ----- */

#define SECURE_NUM_CUMULATIVE_ACK 16
#define RAW_NUM_CUMULATIVE_ACK 10

typedef enum{
	IDLE,
	CONNECTED,
	RECEVEING_RAW_FW,
	RECEVEING_SECURE_FW,
	CLOSING_CONNECTION
}ConnectionStatus;

void data_handler(uint8_t *data_buffer, uint8_t num_bytes);
void send_ack(bool ack, PckType reply_type);
extern void Update_TX_Char(uint8_t *data_buffer, uint8_t num_bytes);

void decrypt_data(uint8_t *data_buffer, uint8_t num_bytes, uint8_t *add_data, uint8_t add_data_len);
void encrypt_data(uint8_t *plaintext, uint8_t plaintext_len, uint8_t *add_data, uint8_t add_data_len, uint8_t *ciphertext, uint8_t *tag);
void init_crypto();

bool checkFWSize(uint16_t num_pckts, uint8_t fw_payload_len, FlashAppType AppSelected);

#endif /* INC_MESSAGE_HANDLER_H_ */
