/*
 * filesystem.h
 *
 *  Created on: 5 сент. 2016 г.
 *      Author: alexey
 */

#ifndef BOOTLOADER_USB_BOOTLOADER_USB_INC_FILESYSTEM_H_
#define BOOTLOADER_USB_BOOTLOADER_USB_INC_FILESYSTEM_H_

#include <stdint.h>

#define FILESYSTEM_INIT_OK      0
#define FILESYSTEM_INIT_FAIL    1

uint8_t initFilesystem();
void deinitFilesystem();




#endif /* BOOTLOADER_USB_BOOTLOADER_USB_INC_FILESYSTEM_H_ */
