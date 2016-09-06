/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "console.h"
#include "cardreader.h"
#include "filesystem.h"
#include "boot.h"
#include "state.h"
#include "flash.h"
#include "utils.h"
#include "fatfs.h"
#include "bootloader-config.h"
#include "usb_device.h"

#include "stm32f1xx_hal.h"

HAL_SD_CardInfoTypedef SDCardInfo;
extern USBD_HandleTypeDef hUsbDeviceFS;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);


int main(void)
{
	// Reset of all peripherals, Initializes the Flash interface and the Systick.
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	// Resetting all pins in case of open gates of mosfets
	resetAllPins();

	initConsole();
	printf("Starting bootloader\n");

	// Reading state from MCU flash's last page
	readState();

	// Ok, something was programmed
	// Enabling FAT FS
	if (initFilesystem() != FILESYSTEM_INIT_OK)
	{
		// Cannot mount SD-card and pick FAT fs for some reason
		// If nothing was programmed yet
		printf("Cannot mount file system or find file with flash\n");
		if (state.state == LOADER_SATE_NO_FLASH)
		{
			// Infinite cardreader
			initCardreader();
			infiniteMessage("System in USB device mode\n");
		} else {
			temporaryEnableCardreader(USB_WAITING_PERIOD);
			printf("Booting...\n");
			bootMainProgram();
		}
	}

	temporaryEnableCardreader(USB_WAITING_PERIOD);

	// We have properly mounted FAT fs and now we should check hash sum of flash.bin
	uint32_t fileHash, trueHash, size;
	HashCheckResult hcr = checkFlashFile(&fileHash, &trueHash, &size);
	switch (hcr)
	{
	case FLASH_FILE_OK:
		// Check if no upgrade was provided
		if (state.hash == fileHash)
		{
			printf("Firmware is up-to-date, nothing to update\n");
			break;
		}
		// Here we should flash our MCU
		FlashResult fr = flash();
		switch (fr)
		{
		case FLASH_RESULT_OK:
			printf("MCU successfuly programmed\n");
			break;
		case FLASH_RESULT_FILE_ERROR:
			infiniteMessage("Flash error: cannot read file\n");
			break;
		case FLASH_RESULT_FLASH_ERROR:
			infiniteMessage("Flash error: cannot write or erase flash memory. Maybe you MCU is totally broken\n");
			break;
		}

		state.hash = fileHash;
		state.state = LOADER_SATE_HAS_FLASH;
		saveState();
		break;
	case FLASH_FILE_NOT_EXISTS:
		printf("Flash file not exists on sd-card\n");
		break;
	case FLASH_FILE_CANNOT_OPEN:
		printf("Cannot read from flash file\n");
		break;
	case FLASH_FILE_INVALID_HASH:
		printf("Flash file hash is invalid: %lu against %lu specified in .ly file\n", fileHash, trueHash);
		break;
	case FLASH_FILE_TOO_BIG:
		printf("Flash file is too big: %lu against %lu available in MCU\n", size, getMaxFlashImageSize());
		break;
	}
	printf("Booting...\n");
	bootMainProgram();

	infiniteMessage("Unexpected fail\n");
	return 0;
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBPLLCLK_DIV1_5;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}



/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */


/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
