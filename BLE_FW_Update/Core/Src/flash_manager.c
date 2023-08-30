/*
 * flash_manager.c
 *
 *  Created on: 24 apr 2023
 *      Author: Francesco Olivieri
 */
#include <stdio.h>
#include "stm32g0xx_hal.h"
#include "flash_manager.h"
#include <inttypes.h>

uint32_t StartPageAddress = FLASH_APP_ADDR;
bool setStartPageAddress(FlashAppType AppToUpload){

	switch(AppToUpload){
		case FLASH_APP_FULL_SPACE:
		case FLASH_APP_1:
			StartPageAddress = FLASH_APP_1_ADDR;

			break;
		case FLASH_APP_2:
			StartPageAddress = FLASH_APP_2_ADDR;

			break;
		default:
			printf("Failed to set starting flash page address, input not valid! \n\r");

			return false;
			break;
	}

	return true;

}

bool Write_FW_to_flash(uint64_t *data_to_flash, uint16_t num_bytes){
	HAL_StatusTypeDef ret;

	ret = HAL_FLASH_Unlock();
	if(ret != HAL_OK){
		printf("ERROR FLASH Unlock!");
		return false;
	}

	uint8_t sofar = 0;
	uint8_t num_dwords = num_bytes;
	while(sofar < num_dwords){
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, StartPageAddress, data_to_flash[sofar]) == HAL_OK){
			StartPageAddress += 8; // doubleword
			sofar++;
		}else{
			printf("Error while writing in FLASH! (%d, %x) \n\r", sofar, data_to_flash[sofar]);

			return false;
		}
	}

	HAL_FLASH_Lock();

	return true;
}

bool Prepare_Application_Memory(FlashAppType AppToUpload){
	HAL_StatusTypeDef ret;
	uint32_t PageError;

	//printf("before : %x \n\r", *(volatile uint16_t*)0x08040000);

	ret = HAL_FLASH_Unlock();
	if(ret != HAL_OK){
		printf("ERROR Unlock!");
	}

	FLASH_EraseInitTypeDef pEraseInit;

	/* NOT WORKING
	pEraseInit.Banks     = FLASH_BANK_1;
	pEraseInit.NbPages   = 0xc800/FLASH_PAGE_SIZE;
	pEraseInit.Page      = 50;
	pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;

	ret               = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
	*/

	printf("APP_SEL : %d \n\r", AppToUpload);
	switch (AppToUpload) {
		case FLASH_APP_FULL_SPACE:
			/* Clear BANK_2 */
			pEraseInit.Banks     = FLASH_BANK_2;
			pEraseInit.NbPages   = 128;
			pEraseInit.Page      = 0;
			pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;

			break;

		case FLASH_APP_1:
			/* Clear BANK_2 */
			pEraseInit.Banks     = FLASH_BANK_2;
			pEraseInit.NbPages   = 64;
			pEraseInit.Page      = 0;
			pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;

			break;

		case FLASH_APP_2:
			/* Clear BANK_2 */
			pEraseInit.Banks     = FLASH_BANK_2;
			pEraseInit.NbPages   = 64;
			pEraseInit.Page      = 64;
			pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;

			break;

		default:
			printf("Bad parameter \n\r");
			break;
	}


    ret = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

	if(ret != HAL_OK){
		printf("Failed to erase FLASH memory\n\r");

		HAL_FLASH_Lock();
		return false;
	}

	fflush(stdout);

	HAL_FLASH_Lock();


	//printf("after: %x \n\r", *(volatile uint16_t*)0x08040000);

	return setStartPageAddress(AppToUpload);
}

void Set_FLASH_Protection(void){
	FLASH_OBProgramInitTypeDef obConfig;

	HAL_FLASHEx_OBGetConfig(&obConfig);

	/*
	 * Should I set also SEC_PROT ? -> I think so, because with SEC_PROT the section of the
	 * 	memory dedicated will not be visible by the application
	 */

	if((obConfig.WRPArea & OB_WRPAREA_ZONE_A)){
		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();

		obConfig.OptionType = OPTIONBYTE_WRP;

		obConfig.WRPArea = OB_WRPAREA_ZONE_A;
		obConfig.WRPStartOffset = 0;
		obConfig.WRPEndOffset = 10;

		HAL_FLASHEx_OBProgram(&obConfig);
		HAL_FLASH_OB_Launch();

		HAL_FLASH_OB_Lock();
		HAL_FLASH_Lock();
	}

}

void go2App(FlashAppType AppSelected){
	uint32_t JumpAddress;
	pFunction Jump_TO_Application;

	printf("APP I'm jumping into : %d \n\r", AppSelected);

	//if(((*(uint32_t *) FLASH_APP_ADDR) & 0x2FFD8000) == 0x20000000){
	if( AppSelected == FLASH_APP_1 || AppSelected == FLASH_APP_FULL_SPACE)
		JumpAddress =  *(__IO uint32_t *) (FLASH_APP_ADDR + 4);
	else
		JumpAddress =  *(__IO uint32_t *) (FLASH_APP_2_ADDR + 4);

	Jump_TO_Application = (pFunction) JumpAddress;


	HAL_RCC_DeInit();
	HAL_DeInit();

	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL  = 0;

	if( AppSelected == FLASH_APP_1 || AppSelected == FLASH_APP_FULL_SPACE)
		__set_MSP(*(uint32_t *)FLASH_APP_ADDR);
	else
		__set_MSP(*(uint32_t *)FLASH_APP_2_ADDR);

	Jump_TO_Application();

}
