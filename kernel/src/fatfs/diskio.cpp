/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "fatfs/ff.h"	  /* Obtains integer types */
#include "fatfs/diskio.h" /* Declarations of disk functions */

#include <storage/ahci/ahci.hpp>
#include <kernel.hpp>
#include <system/cmos.hpp>

/* Definitions of physical drive number for each drive */
#define DEV_RAM 0 /* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC 1 /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB 2 /* Example: Map USB MSD to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	if (!mainDriver)
		return RES_ERROR;

	if (mainDriver->status(pdrv))
		return RES_OK;

	return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	if (!mainDriver)
		return RES_ERROR;

	if (mainDriver->initialize(pdrv))
		return RES_OK;

	return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,	  /* Physical drive nmuber to identify the drive */
	BYTE *buff,	  /* Data buffer to store read data */
	LBA_t sector, /* Start sector in LBA */
	UINT count	  /* Number of sectors to read */
)
{
	if (!mainDriver)
		return RES_ERROR;

	if (mainDriver->read(pdrv, sector, count, buff))
		return RES_OK;

	return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
	BYTE pdrv,		  /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	LBA_t sector,	  /* Start sector in LBA */
	UINT count		  /* Number of sectors to write */
)
{
	if (!mainDriver)
		return RES_ERROR;

	if (mainDriver->write(pdrv, sector, count, (BYTE *)buff))
		return RES_OK;

	return RES_ERROR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv, /* Physical drive nmuber (0..) */
	BYTE cmd,  /* Control code */
	void *buff /* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (cmd)
	{
	case CTRL_TRIM:
	case CTRL_SYNC:
	{
		return RES_OK;
	}
	case GET_SECTOR_COUNT:
	{
		DWORD sectorCount = mainDriver->getSectorCount(pdrv);
		LBA_t *lbat = (LBA_t *)buff;
		*lbat = sectorCount;

		return RES_OK;
	}

	case GET_SECTOR_SIZE:
	{
		WORD *word = (WORD *)buff;
		*word = 512;
		return RES_OK;
	}
	case GET_BLOCK_SIZE:
	{
		DWORD *dword = (DWORD *)buff;
		*dword = 1;
		return RES_OK;
	}
	}

	return RES_PARERR;
}

DWORD get_fattime(void)
{
	k_datetime datetime = cmosGetDatetime();

	return (DWORD)(datetime.year - 80) << 25 |
		   (DWORD)(datetime.month + 1) << 21 |
		   (DWORD)datetime.day << 16 |
		   (DWORD)datetime.hour << 11 |
		   (DWORD)datetime.minute << 5 |
		   (DWORD)datetime.second >> 1;
}