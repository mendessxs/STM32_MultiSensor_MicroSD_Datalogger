/*
 * sd_diskio.c
 *
 *  Created on: Mar 3, 2026
 *      Author: Rubin Khadka
 */

#include "sd_spi.h"
#include "sd_diskio.h"

DSTATUS SD_disk_status(BYTE drv)
{
  if(drv != 0)
    return STA_NOINIT;
  return 0;
}

DSTATUS SD_disk_initialize(BYTE drv)
{
  if(drv != 0)
    return STA_NOINIT;

  return (SD_Init() == SD_OK) ? 0 : STA_NOINIT;
}

DRESULT SD_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
  if(pdrv != 0 || count == 0)
    return RES_PARERR;
  if(!card_initialized)
    return RES_NOTRDY;
  return (SD_ReadBlocks(buff, sector, count) == SD_OK) ? RES_OK : RES_ERROR;
}

DRESULT SD_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
  if(pdrv || !count)
    return RES_PARERR;
  if(!card_initialized)
    return RES_NOTRDY;
  return (SD_WriteBlocks(buff, sector, count) == SD_OK) ? RES_OK : RES_ERROR;
}

DRESULT SD_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
  if(pdrv != 0)
    return RES_PARERR;

  switch(cmd)
  {
    case CTRL_SYNC:
      return RES_OK;
    case GET_SECTOR_SIZE:
      *(WORD*) buff = 512;
      return RES_OK;
    case GET_SECTOR_COUNT:
      *(DWORD*) buff = 0x10000;
      return RES_OK;
    case GET_BLOCK_SIZE:
      *(DWORD*) buff = 1;
      return RES_OK;
    default:
      return RES_PARERR;
  }
}

const Diskio_drvTypeDef SD_Driver = {SD_disk_initialize, SD_disk_status, SD_disk_read,
#if _USE_WRITE
    SD_disk_write,
#endif
#if _USE_IOCTL
    SD_disk_ioctl,
#endif
    };
