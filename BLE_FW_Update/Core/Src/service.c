#include <message_handler.h>
#include <stdio.h>

#include "bluenrg_conf.h"
#include "bluenrg_gap.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_aci_const.h"
#include "hci_const.h"
#include "service.h"

#include "tim.h"

#include "flash_manager.h"


uint8_t CHAT_SERVICE_UUID[16] = {0x88, 0x7b, 0x98, 0x2b, 0x6b, 0xfc, 0x89, 0x9d, 0xf4, 0x48, 0xae, 0xb8, 0x88, 0x39, 0x4f, 0x98};
uint8_t RX_CHAR_UUID[16] = {0x88, 0x7b, 0x98, 0x2b, 0x6b, 0xfc, 0x89, 0x9d, 0xf4, 0x49, 0xae, 0xb8, 0x88, 0x39, 0x4f, 0x98};
uint8_t TX_CHAR_UUID[16] = {0x88, 0x7b, 0x98, 0x2b, 0x6b, 0xfc, 0x89, 0x9d, 0xf4, 0x4a, 0xae, 0xb8, 0x88, 0x39, 0x4f, 0x98};

uint16_t chat_service_handle, rx_char_handle, tx_char_handle;

uint32_t connected = FALSE;
uint8_t set_connectable = TRUE;
uint16_t connection_handle = 0;
uint8_t notification_enabled = FALSE;

extern ConnectionStatus connection_status;
extern uint16_t count_pck;
extern uint16_t cont_buff;
extern uint16_t total_pck;
extern FlashAppType AppToUpdate;

tBleStatus add_FW_Update_Service(void){
	tBleStatus ret;
	Service_UUID_t chat_service_uuid;
	Char_UUID_t rx_char_uuid, tx_char_uuid;

	BLUENRG_memcpy(&chat_service_uuid.Service_UUID_128, CHAT_SERVICE_UUID, 16);

	BLUENRG_memcpy(rx_char_uuid.Char_UUID_128, RX_CHAR_UUID, 16);
	BLUENRG_memcpy(tx_char_uuid.Char_UUID_128, TX_CHAR_UUID, 16);

	/* ---- Add FW update Service ---- */
	ret = aci_gatt_add_serv(UUID_TYPE_128, chat_service_uuid.Service_UUID_128, PRIMARY_SERVICE, 7, &chat_service_handle);
	if(ret != BLE_STATUS_SUCCESS){
		printf("Error in the creation of the service \n\r");
		return ret;
	}

	/* Add Characteristics */
	ret = aci_gatt_add_char(chat_service_handle, UUID_TYPE_128, rx_char_uuid.Char_UUID_128, CHAT_DATA_LEN, CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,16, 1, &rx_char_handle);
	if(ret != BLE_STATUS_SUCCESS){
		printf("Failed to add RX char to the service \n\r");
		return ret;
	}


	ret = aci_gatt_add_char(chat_service_handle, UUID_TYPE_128, tx_char_uuid.Char_UUID_128, CHAT_DATA_LEN, CHAR_PROP_NOTIFY, ATTR_PERMISSION_NONE, 0, 16, 1, &tx_char_handle);

	if(ret != BLE_STATUS_SUCCESS){
		printf("Failed to add TX char to the service \n\r");
		return ret;
	}



	return ret;
}

void Update_TX_Char(uint8_t *data_buffer, uint8_t num_bytes){
	tBleStatus ret;

	printf("response: ");
	for(int i=0; i<num_bytes ; i++){
		printf("%02x ", data_buffer[i]);
	}
	printf("\n\r");

	ret = aci_gatt_update_char_value(chat_service_handle, tx_char_handle, 0, num_bytes, data_buffer);

	if(ret != BLE_STATUS_SUCCESS){
		printf("Error while updating tx_char value \n\r");
	}

}

void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle){
	connected = TRUE;
	connection_handle = handle;
	connection_status = CONNECTED;

	printf("Connection Complete...\n\r");

	for(int i=5; i>=0 ; i--){
		printf("%02X -", addr[i]);
	}
	printf("%02X\n\r",addr[0]);
}

void GAP_DisconnectionComplete_CB(void){
	connected = FALSE;
	connection_status = IDLE;
	printf("Disconnection Complete...\n\r");

	if(count_pck == total_pck && total_pck != 0){
		printf("Jump to the new FW :\n\n\r");
		go2App(AppToUpdate);
	}else{
		printf("Upload FW interrupted, received %d packets over %d\n\r", count_pck, total_pck);

		count_pck = 0;
		cont_buff = -1;

		printf("Erasing firmware saved in memory...");
		Prepare_Application_Memory(AppToUpdate);
		printf("Done\n\r");

		printf("Device discoverable again...\n\r");
	}

	set_connectable = TRUE;
	notification_enabled = FALSE;
}

void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data){


	if(handle == rx_char_handle+1){
		data_handler(att_data, data_length);

	}else if(handle == tx_char_handle+2){

		if(att_data[0] == 0x01){
			notification_enabled = TRUE;
		}

	}
}


/**
 * @brief  Callback processing the ACI events.
 * @note   Inside this function each event must be identified and correctly
 *         parsed.
 * @param  void* Pointer to the ACI packet
 * @retval None
 */
void HCI_Event_CB(void *pData){
	hci_uart_pckt *hci_pckt = (hci_uart_pckt *)pData;

	/* Process event packet */
	if(hci_pckt->type == HCI_EVENT_PKT){
		/* Get data from packet */
		hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;

		switch(event_pckt->evt){
			case EVT_DISCONN_COMPLETE:
				GAP_DisconnectionComplete_CB();

				break;

			case EVT_LE_META_EVENT:
				{
					evt_le_meta_event *evt = (evt_le_meta_event*)event_pckt->data;

					switch(evt->subevent){


						case EVT_LE_CONN_COMPLETE:
						{
							evt_le_connection_complete *cc = (evt_le_connection_complete *)evt->data;
							GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
						}
							break;
					}
				}

				break;

			case EVT_VENDOR:
			{
				evt_blue_aci *blue_evt = (evt_blue_aci*)event_pckt->data;

				switch(blue_evt->ecode){

						/* consequence of write by the client */
					case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED: {
						evt_gatt_attr_modified_IDB05A1 *evt =
								(evt_gatt_attr_modified_IDB05A1*) blue_evt->data;

						printf("JUST RECEIVED %lu \r\n",__HAL_TIM_GetCounter(&htim1));
						Attribute_Modified_CB(evt->attr_handle, evt->data_length,
								evt->att_data);
						printf("FINISHED RECEIVEING %lu \r\n",__HAL_TIM_GetCounter(&htim1));
					}
						break;
					case EVT_BLUE_GATT_NOTIFICATION: {
						evt_gatt_attr_notification *evt =
								(evt_gatt_attr_notification*) blue_evt->data;

						data_handler(evt->attr_value, evt->event_data_length - 2);
					}

						break;
/*
					case EVT_BLUE_GATT_DISC_READ_CHAR_BY_UUID_RESP:
					{
						printf("Ricevuto\n\r");
						evt_gatt_disc_read_char_by_uuid_resp *evt = (evt_gatt_disc_read_char_by_uuid_resp *)blue_evt->data;
						evt->attr_handle;
					}
						break;
*/
				}
			}

				break;


			default:
				printf("%d\n\r",event_pckt->evt);
				break;
		}

	}else{
		printf("pckt_type: %d", hci_pckt->type);
	}
}
