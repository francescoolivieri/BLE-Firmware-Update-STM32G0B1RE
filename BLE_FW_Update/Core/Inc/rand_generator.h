/*
 * rand_generator.h
 *
 *  Created on: 2 lug 2023
 *      Author: Francesco Olivieri
 */

#ifndef INC_RAND_GENERATOR_H_
#define INC_RAND_GENERATOR_H_

#include <stdio.h>

void init_rand_generator(uint16_t initializer);

uint8_t get_rand_byte();

#endif /* INC_RAND_GENERATOR_H_ */
