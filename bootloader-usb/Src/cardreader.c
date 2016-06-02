/*
 * cardreader.c
 *
 *  Created on: 15 дек. 2015 г.
 *      Author: alexey
 */

#include "cardreader.h"
#include "sd-utils.h"
#include "bsp_driver_sd.h"
#include "usb_device.h"
#include "usbd_core.h"

extern HAL_SD_CardInfoTypedef SDCardInfo;
extern SD_HandleTypeDef hsd;
extern USBD_HandleTypeDef hUsbDeviceFS;

int USBDeviceActivated = 0;

void initCardreader()
{
	MX_SDIO_SD_Init();
	BSP_SD_Init();
	MX_USB_DEVICE_Init();
	/*
	HAL_SD_Get_CardInfo(&hsd, &SDCardInfo);
	printf("SDCardInfo.CardCapacity = %d\n", (int) SDCardInfo.CardCapacity);
	printf("SDCardInfo.CardBlockSize = %d\n", (int) SDCardInfo.CardBlockSize);
	printf("SDCardInfo.CardType = %d\n", (int) SDCardInfo.CardType);*/
}

void deinitCardreader()
{
	/* Stop the low level driver  */
	USBD_LL_Stop(&hUsbDeviceFS);

	/* Initialize low level driver */
	USBD_LL_DeInit(&hUsbDeviceFS);
}

int isCardreaderActive()
{
	return USBDeviceActivated;
}
