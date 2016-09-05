#include "filesystem.h"
#include "fatfs.h"

static FATFS fatfs;

extern SD_HandleTypeDef hsd;

uint8_t initFilesystem()
{
	MX_SDIO_SD_Init();
	MX_FATFS_Init();
	FRESULT res = f_mount(&fatfs, "", 1);
	if (res != FR_OK)
	{
		printf("Cannot mount FS!\n");
		return FILESYSTEM_INIT_FAIL;
	}
	return FILESYSTEM_INIT_OK;
}

void deinitFilesystem()
{
	f_mount(&fatfs, "", 0);
	HAL_SD_DeInit(&hsd);
}
