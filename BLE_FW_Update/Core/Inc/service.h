
#ifndef SRC_SERVICE_H_
#define SRC_SERVICE_H_

#include <message_handler.h>

/** Documentation for C union Service_UUID_t */
typedef union Service_UUID_t_s {
  /** 16-bit UUID
  */
  uint16_t Service_UUID_16;
  /** 128-bit UUID
  */
  uint8_t Service_UUID_128[16];
} Service_UUID_t;


/** Documentation for C union Char_UUID_t */
typedef union Char_UUID_t_s {
  /** 16-bit UUID
  */
  uint16_t Char_UUID_16;
  /** 128-bit UUID
  */
  uint8_t Char_UUID_128[16];
} Char_UUID_t;


tBleStatus add_FW_Update_Service(void);
void HCI_Event_CB(void *pData);

void Update_TX_Char(uint8_t *data_buffer, uint8_t num_bytes);

#endif /* SRC_SERVICE_H_ */
