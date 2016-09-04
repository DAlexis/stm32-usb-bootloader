/*
 * state.c
 *
 *  Created on: 5 сент. 2016 г.
 *      Author: alexey
 */

#include "state.h"
#include "stm32f1xx_hal.h"

LoaderState state;

extern unsigned int _loader_state_addr;

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

