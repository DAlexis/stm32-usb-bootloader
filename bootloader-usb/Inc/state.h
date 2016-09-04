/*
 * state.h
 *
 *  Created on: 5 сент. 2016 г.
 *      Author: alexey
 */

#ifndef BOOTLOADER_USB_BOOTLOADER_USB_INC_STATE_H_
#define BOOTLOADER_USB_BOOTLOADER_USB_INC_STATE_H_

#define LOADER_SATE_NO_FLASH          0x1234
#define LOADER_SATE_HAS_FLASH         0x4321
#define LOADER_SATE_SHOULD_BOOT       0x1111
#define LOADER_SATE_SHOULD_CHECK      0x2222

#include <stdint.h>

typedef struct {
	uint32_t state;
	uint32_t mainProgramStackPointer;
	uint32_t mainProgramResetHandler;
	uint32_t hash;
} LoaderState;

extern LoaderState state;

void saveState();
void readState();


#endif /* BOOTLOADER_USB_BOOTLOADER_USB_INC_STATE_H_ */
