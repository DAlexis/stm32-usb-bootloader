#include "boot.h"
#include "state.h"

#include "console.h"
#include "stm32f1xx_hal.h"

typedef void (*Callable)();

void bootMainProgram()
{
	Callable pReset_Handler = (Callable) state.mainProgramResetHandler;
	printf("Booting at %lX\n", (uint32_t)pReset_Handler);
	//__disable_irq();
	deinitConsile();
	HAL_DeInit();
	// Disabling SysTick interrupt
	SysTick->CTRL = 0;
	moveVectorTable(0x00);
	// Setting initial value to stack pointer
	__set_MSP(state.mainProgramStackPointer);
	// booting really
	pReset_Handler();
}

/*
#include "flash.h"
#include "fatfs.h"
#include "console.h"
#include "utils.h"
#include "flash-consts.h"
#include "sd-utils.h"
#include "stm32f1xx_hal.h"

#include <stdio.h>
#include <string.h>

#define LOADER_SATE_NO_FLASH          0x1234
#define LOADER_SATE_HAS_FLASH         0x4321
#define LOADER_SATE_SHOULD_BOOT       0x1111
#define LOADER_SATE_SHOULD_CHECK      0x2222

extern unsigned int _isr_real;
extern unsigned int _loader_state_addr;
extern unsigned int _estack;

void Reset_Handler();

typedef void (*Callable)();

static FATFS fatfs;
static FIL fil;
static FILINFO info;

extern SD_HandleTypeDef hsd;

const char flashFileName[] = "flash.bin";
const char hashFileName[] = "flash.ly";

typedef struct {
	uint32_t state;
	uint32_t mainProgramStackPointer;
	uint32_t mainProgramResetHandler;
	uint32_t hash;
} LoaderState;

LoaderState state;

// Initial loader state when no real program is flashed yet
__attribute__ ((section(".loader_state"),used))
uint32_t loaderStateStub[] =
{
		LOADER_SATE_NO_FLASH,
		0x0,
		0x0,
		0x0
};

void saveState()
{
	HAL_FLASH_Unlock();
	if (HAL_OK != erase((uint32_t) &_loader_state_addr, (uint32_t) &_loader_state_addr))
	{
		printf("Error while loader state saving: cannot erase state page. Rebooting...\n");
		reboot();
	}
	//FLASH_PageErase((uint32_t) &_loader_state_addr);
	for (uint32_t i=0; i < sizeof(LoaderState) / 4; i++)
	{
		uint32_t targetAddress = (uint32_t) &_loader_state_addr + 4*i;
		uint32_t* pWord = ((uint32_t*) &state) + i;
		if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, targetAddress, *pWord))
		{
			printf("Error while loader state saving: cannot program state page. Rebooting...\n");
			reboot();
		}
	}
	HAL_FLASH_Lock();
}

void readState()
{
	memcpy(&state, &_loader_state_addr, sizeof(LoaderState));
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

void reboot()
{
	f_mount(&fatfs, "", 0);
	deinitConsile();
	NVIC_SystemReset();
}

void bootMainProgram()
{
	Callable pReset_Handler = (Callable) state.mainProgramResetHandler;
	printf("Booting at %lX\n", (uint32_t)pReset_Handler);
	//__disable_irq();
	deinitConsile();
	HAL_DeInit();
	// Disabling SysTick interrupt
	SysTick->CTRL = 0;
	moveVectorTable(0x00);
	// Setting initial value to stack pointer
	__set_MSP(state.mainProgramStackPointer);
	// booting really
	pReset_Handler();
}

void bootIfReady()
{
	readState();
	if (state.state == LOADER_SATE_SHOULD_BOOT) {
		printf("MCU is flashed, we can boot\n");
		// Next reboot we must check for updates
		state.state = LOADER_SATE_SHOULD_CHECK;
		saveState();
		bootMainProgram();
	}
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
		printf("Cannot f_lseek(pfil, 0) for hasing: %d\n", res);
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
			printf("Cannot read file to calculate hash: %d\n", res);
			return 0;
		}
		result = HashLy(result, buf, readed);
	} while (readed == blockSize);

	res = f_lseek(pfil, 0);
	if (FR_OK != res)
	{
		printf("Cannot f_lseek(fil, 0) after hashing: %d\n", res);
		return 0;
	}
	return result;
}

void rebootToMainCode()
{
	state.state = LOADER_SATE_SHOULD_BOOT;
	saveState();
	reboot();
}
*/
