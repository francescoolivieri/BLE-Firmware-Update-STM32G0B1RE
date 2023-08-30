/*
 * flash_manager.h
 *
 *  Created on: 24 apr 2023
 *      Author: Francesco Olivieri
 */

#ifndef INC_FLASH_MANAGER_H_
#define INC_FLASH_MANAGER_H_

typedef void (*pFunction)(void);

typedef enum{
	false,
	true
}bool;

#define FLASH_APP_ADDR 0x08040000
#define FLASH_APP_1_ADDR 0x08040000
#define FLASH_APP_2_ADDR 0x08060000

typedef enum{
	FLASH_APP_FULL_SPACE,
	FLASH_APP_1,
	FLASH_APP_2,
	NONE
} FlashAppType;

bool setStartPageAddress(FlashAppType AppToUpload);

bool Write_FW_to_flash(uint64_t *data_to_flash, uint16_t num_bytes);
bool Prepare_Application_Memory(FlashAppType AppToUpload);
void Set_FLASH_Protection(void);


void go2App(FlashAppType AppSelected);

#endif /* INC_FLASH_MANAGER_H_ */
