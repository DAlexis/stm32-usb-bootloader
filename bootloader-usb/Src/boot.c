#include "boot.h"
#include "state.h"
#include "filesystem.h"

#include "console.h"
#include "stm32f1xx_hal.h"

typedef void (*Callable)();

void bootMainProgram()
{
	deinitFilesystem();
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

void reboot()
{
	deinitFilesystem();
	deinitConsile();
	NVIC_SystemReset();
}
