/*
 * app_bluenrg.c
 *
 *  Created on: Apr 7, 2023
 *      Author: Francesco Olivieri
 */
#include <stdio.h>

#include "bluenrg_utils.h"
#include "bluenrg_conf.h"
#include "hci.h"
#include "bluenrg_types.h"
#include "bluenrg_aci.h"
#include "bluenrg_gap.h"
#include "hci_le.h"
#include "gpio.h"

#include "service.h"

#include "app_bluenrg.h"

#define BDADDR_SIZE 6

/* change this to change address */
uint8_t SERVER_BDADDRR[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

extern uint8_t set_connectable;
extern uint32_t connected;
extern uint8_t notification_enabled;

/*
 * Initialization task
 */
HAL_StatusTypeDef BlueNRG_Init(void){
	HAL_StatusTypeDef ret = HAL_OK;
	tBleStatus ret_ble;
	uint8_t bdaddr[BDADDR_SIZE];
	const char *name = "MyBLE";

	uint16_t service_handle, dev_name_char_handle, appearance_char_handle; // handlers of GAP service

	BLUENRG_memcpy(bdaddr, SERVER_BDADDRR, sizeof(SERVER_BDADDRR));

	/* Init HCI */
	hci_init(HCI_Event_CB,  NULL);

	/* Reset HCI */
	hci_reset();
	HAL_Delay(100);

	printf("\r\nStart initialization... \n\r");
	fflush(stdout);


	/* Configure device address */
	ret_ble = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, bdaddr);

	if(ret_ble != BLE_STATUS_SUCCESS){
		printf("Failed to set Public Address \n\r");
		ret = HAL_ERROR;
	}

	/* Initialize GATT server */
	ret_ble = aci_gatt_init();
	if(ret_ble != BLE_STATUS_SUCCESS){
		printf("Failed to GATT Initialization \n\r");
		ret = HAL_ERROR;
	}

	/* Initialize GAP service */
	ret_ble = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle); //2nd arg -> privacy (0: no, 1: yes)


	if(ret_ble != BLE_STATUS_SUCCESS){
		printf("Failed to Initialize GAP Service\n\r");
		ret = HAL_ERROR;
	}


	/* Update characteristics */
	ret_ble = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, strlen(name), (uint8_t *) name);
	if(ret_ble != BLE_STATUS_SUCCESS){
		printf("Failed to Update Characteristics %d\n\r", ret);
		ret = HAL_ERROR;
	}

	/* Add custom service */
	ret_ble = add_FW_Update_Service();
	if(ret_ble != BLE_STATUS_SUCCESS){
		printf("Failed to Add Service\n\r");
		ret = HAL_ERROR;
	}

	fflush(stdout);
	return ret;
}


/*
 * Background task
 */
void BlueNRG_Process(void){
	if(set_connectable){
		Enable_Advertising();
		set_connectable = FALSE;
	}

	/* Process user event */
	hci_user_evt_proc();

}

void Enable_Advertising(void){
	char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME, 'B', 'L', 'E', '-', 'G', '-', 'U', 'P'};

	hci_le_set_scan_resp_data(0, NULL);

	/* Set device in General Discoverable mode */
	aci_gap_set_discoverable(ADV_IND, 0, 0, PUBLIC_ADDR, NO_WHITE_LIST_USE, sizeof(local_name), local_name, 0, NULL, 0, 0);

}
