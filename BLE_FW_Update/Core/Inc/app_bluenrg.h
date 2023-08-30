/*
 * app_bluenrg.h
 *
 *  Created on: Apr 7, 2023
 *      Author: Francesco Olivieri
 */

#ifndef SRC_APP_BLUENRG_H_
#define SRC_APP_BLUENRG_H_

HAL_StatusTypeDef BlueNRG_Init(void);
void BlueNRG_Process(void);

void Enable_Advertising(void);

#endif /* SRC_APP_BLUENRG_H_ */
