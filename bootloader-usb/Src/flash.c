/*
 * flash.c
 *
 *  Created on: 9 дек. 2015 г.
 *      Author: alexey
 */

#include "state.h"
#include "flash.h"
#include "fatfs.h"
#include "console.h"
#include "utils.h"
#include "flash-consts.h"
#include "sd-utils.h"
#include "stm32f1xx_hal.h"

#include <stdio.h>
#include <string.h>

void Reset_Handler();

extern unsigned int _isr_real;
extern unsigned int _estack;

uint32_t firstPageAddr = 0x8000000;
uint32_t secondPageAddr = 0x8000000 + FLASH_PAGE_SIZE;

uint32_t lastPageAddr = ((uint32_t) &_isr_real) - FLASH_PAGE_SIZE;

uint8_t firstPage[FLASH_PAGE_SIZE];
uint32_t fistPageLen = 0;

uint8_t buffer[FLASH_PAGE_SIZE];
uint32_t bufferLen = 0;

static FATFS fatfs;
static FIL fil;
static FILINFO info;

extern SD_HandleTypeDef hsd;

const char flashFileName[] = "flash.bin";
const char hashFileName[] = "flash.ly";

uint32_t getMaxFlashImageSize()
{
	return (uint32_t) &_isr_real - firstPageAddr;
}

void moveVectorTable(uint32_t Offset)
{
	SCB->VTOR = FLASH_BASE | Offset;
}

HAL_StatusTypeDef erase(uint32_t from, uint32_t to)
{
	HAL_StatusTypeDef res = HAL_OK;
	for (uint32_t i = from; i <= to; i += FLASH_PAGE_SIZE)
	{
		FLASH_EraseInitTypeDef erase;
		erase.TypeErase = FLASH_TYPEERASE_PAGES;
		erase.Banks = FLASH_BANK_1;
		erase.PageAddress = i;
		erase.NbPages = 1;
		uint32_t error = 0;
		res = HAL_FLASHEx_Erase(&erase, &error);
		if (res != HAL_OK) {
			printf("Error while erasing page at %lX\n", i);
			return res;
		}
	}
	printf("Pages from %lX to %lX erased\n", from, to);
	return res;
}

FRESULT readNextPage(uint8_t *target, uint32_t *readed)
{
	uint16_t blocksCount = 16;
	uint16_t fileBlock = FLASH_PAGE_SIZE / blocksCount;
	*readed = 0;
	UINT readedNow = 0;
	FRESULT res = FR_OK;
	for (uint16_t i = 0; i<blocksCount; i++)
	{
		res = f_read(&fil, target, fileBlock, &readedNow);
		target += readedNow;
		*readed += readedNow;
		if (res != FR_OK || readedNow != fileBlock)
			break;
	}
	return res;
}

HAL_StatusTypeDef flashWrite(uint32_t position, uint8_t *data, uint32_t size)
{
	HAL_StatusTypeDef res = HAL_OK;
	for (uint32_t i=0; i<size; i+=2)
	{
		res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, position + i, *(uint16_t*)&data[i]);
		if (res != HAL_OK)
		{
			printf("Error! Flash programming failed at %lX: %d\n", position+i, res);
			break;
		}
	}
	return res;
}

uint32_t calculateFlashHash(size_t size)
{
	uint32_t result = 0;
	// First get hash from original two first adresses int vectors table
	result = HashLy(result, (uint8_t*) &state.mainProgramStackPointer, 4);
	result = HashLy(result, (uint8_t*) &state.mainProgramResetHandler, 4);
	// Reading all flash word by word
	for (size_t i=8; i<size; i +=4)
	{
		uint32_t word = * (uint32_t*) (FLASH_BEGIN + i);
		result = HashLy(result, (uint8_t*) &word, 4);
	}
	return result;
}

uint32_t calculateFileHash(FIL* pfil)
{
	FRESULT res = f_lseek(pfil, 0);
	if (FR_OK != res)
	{
		//printf("Cannot f_lseek(pfil, 0) for hashing: %d\n", res);
		return 0;
	}
	UINT readed = 0;
	UINT blockSize = 256;
	uint8_t buf[256];
	uint32_t result = 0;

	do {
		res = f_read(pfil, buf, blockSize, &readed);
		if (FR_OK != res)
		{
			//printf("Cannot read file to calculate hash: %d\n", res);
			return 0;
		}
		result = HashLy(result, buf, readed);
	} while (readed == blockSize);

	res = f_lseek(pfil, 0);
	if (FR_OK != res)
	{
		//printf("Cannot f_lseek(fil, 0) after hashing: %d\n", res);
		return 0;
	}
	return result;
}

HashCheckResult checkFlashFile(uint32_t* hash, uint32_t* trueHash, uint32_t* flashFileSize)
{
	// Testing flash file existance
	FRESULT res = f_stat(flashFileName, &info);
	if (res != FR_OK)
		return FLASH_FILE_NOT_EXISTS;

	// Checking file size
	if (info.fsize > getMaxFlashImageSize())
		return FLASH_FILE_TOO_BIG;

	// Reading file with hash
	*trueHash = 0; // keep 0 if we have no hash value
	res = f_open(&fil, hashFileName, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK) {
		UINT readed = 0;
		f_read(&fil, (void*) trueHash, sizeof(*trueHash), &readed);
		f_close(&fil);
		if (readed != sizeof(*trueHash))
			*trueHash = 0;
	}

	res = f_open(&fil, flashFileName, FA_OPEN_EXISTING | FA_READ);
	if (res != FR_OK)
		return FLASH_FILE_CANNOT_OPEN;

	*hash = calculateFileHash(&fil);
	f_close(&fil);
	if (*hash == 0)
		return FLASH_FILE_CANNOT_OPEN;
	if (*hash == *trueHash || *trueHash == 0)
		return FLASH_FILE_OK;
	else
		return FLASH_FILE_INVALID_HASH;
}

FlashResult flash()
{
	FRESULT res = f_open(&fil, flashFileName, FA_OPEN_EXISTING | FA_READ);

	if (res != FR_OK)
		return FLASH_RESULT_FILE_ERROR;

	uint32_t position = FLASH_BEGIN + FLASH_PAGE_SIZE;
	printf("Erasing MCU pages except first...\n");
	HAL_StatusTypeDef hal_res = HAL_FLASH_Unlock();
	if (hal_res != HAL_OK)
	{
		printf("Unlock error: %d\n", res);
		return FLASH_RESULT_FLASH_ERROR;
	}

	if (HAL_OK != erase(secondPageAddr, info.fsize + firstPageAddr))
	{
		printf("Error during erasing MCU, rebooting system.\n");
		return FLASH_RESULT_FLASH_ERROR;
	}

	if (FR_OK == readNextPage(firstPage, &fistPageLen))
	{
		printf("First page of flash readed successfuly\n");
		// Reading original SP and Reset_Handler
		state.mainProgramStackPointer = *(uint32_t*)&firstPage[0];
		state.mainProgramResetHandler = *(uint32_t*)&firstPage[4];
		// Changing it to bootloader ones
		*(uint32_t*)&firstPage[0] = (uint32_t) &_estack;
		*(uint32_t*)&firstPage[4] = (uint32_t) Reset_Handler;
	} else {
		f_close(&fil);
		return FLASH_RESULT_FILE_ERROR;
	}

	do {
		readNextPage(buffer, &bufferLen);
		if (HAL_OK != flashWrite(position, buffer, bufferLen))
		{
			printf("Cannot write flash page at %lX, rebooting system.\n", position);
			return FLASH_RESULT_FLASH_ERROR;
		}
		position += bufferLen;
		printf("Page at %lX written\n", position);
	} while (bufferLen != 0);
	f_close(&fil);

	printf("Erasing first page...\n");
	if (HAL_OK != erase(FLASH_BEGIN, FLASH_BEGIN))
	{
		printf("Error during erasing first page, rebooting system.\n");
		return FLASH_RESULT_FLASH_ERROR;
	}
	printf("Writing first page...\n");
	if (HAL_OK != flashWrite(FLASH_BEGIN, firstPage, fistPageLen))
	{
		infiniteMessage("Cannot write first flash page. Your MCU bricked, reflash bootloader with SWD/JTAG!\n");
	}

	printf("First page written\n");
	HAL_FLASH_Lock();
	printf("Flash locked\n");
}
