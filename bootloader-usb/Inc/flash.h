/*
 * flash.h
 *
 *  Created on: 9 дек. 2015 г.
 *      Author: alexey
 */

#ifndef INCLUDE_FLASH_H_
#define INCLUDE_FLASH_H_

#include <stdint.h>

typedef enum
{
	FLASH_FILE_OK = 0,
	FLASH_FILE_NOT_EXISTS,
	FLASH_FILE_CANNOT_OPEN,
	FLASH_FILE_INVALID_HASH,
	FLASH_FILE_TOO_BIG
} HashCheckResult;

typedef enum
{
	FLASH_RESULT_OK = 0,
	FLASH_RESULT_FILE_ERROR,
	FLASH_RESULT_FLASH_ERROR
} FlashResult;

uint32_t getMaxFlashImageSize();

HashCheckResult checkFlashFile(uint32_t* hash, uint32_t* trueHash, uint32_t* flashFileSize);

void bootIfReady();
FlashResult flash(); ///@todo add callback for progress



#endif /* INCLUDE_FLASH_H_ */
