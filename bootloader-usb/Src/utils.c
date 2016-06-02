/*
 * utils.c
 *
 *  Created on: 10 дек. 2015 г.
 *      Author: alexey
 */

#include "utils.h"

#include "stm32f1xx_hal.h"

#include <string.h>
#include <stdint.h>


void delay()
{
	for (volatile uint32_t i=0; i<10000000; i++) {}
}

uint32_t HashLy(uint32_t hash, const uint8_t * buf, uint32_t size)
{
	for(uint32_t i=0; i<size; i++)
		hash = (hash * 1664525) + buf[i] + 1013904223;

	return hash;
}

void resetAllPins()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
	// Resetting all pins except PA11 and PA12, that are for USBDM, USBDP
	uint16_t pinMask = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
			GPIO_PIN_4 | GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7  | GPIO_PIN_8  |
			GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_13 |
			GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStructure.Pin = pinMask;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	HAL_GPIO_WritePin(GPIOA, pinMask, GPIO_PIN_RESET);

	// For other GPIO we should add this pins
	GPIO_InitStructure.Pin |= GPIO_PIN_11 | GPIO_PIN_12;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	HAL_GPIO_WritePin(GPIOB, pinMask, GPIO_PIN_RESET);

	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	HAL_GPIO_WritePin(GPIOC, pinMask, GPIO_PIN_RESET);

	GPIO_InitStructure.Pin = GPIO_PIN_2;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
}

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
	printf("Wrong parameters value: file %s on line %lu\r\n", file, line);
}

void fatal_error(char* what, int errCode, uint8_t* file, uint32_t line)
{
	printf("Fatal error %s with error code %d: file %s on line %lu\r\n", what, errCode, file, line);
	for(;;) { }
}
