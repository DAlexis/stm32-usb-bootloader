/*
 * flash.h
 *
 *  Created on: 9 дек. 2015 г.
 *      Author: alexey
 */

#ifndef INCLUDE_FLASH_H_
#define INCLUDE_FLASH_H_

#include <stdint.h>

#define FLASH_RESULT_OK          0
#define FLASH_RESULT_ERROR       1

#define FLASH_FILE_OK            0
#define FLASH_FILE_NOT_EXISTS    1
#define FLASH_FILE_INVALID_HASH  2
#define FLASH_FILE_TOO_BIG       3

uint8_t checkFlashFile(uint32_t* hash);

void bootIfReady();
void flash();

uint32_t getMaxFlashImageSize();

#endif /* INCLUDE_FLASH_H_ */
