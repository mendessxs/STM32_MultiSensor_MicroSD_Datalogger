/*
 * sd_diskio.c
 *
 *  Created on: Mar 3, 2026
 *      Author: Rubin Khadka
 */

#include "sd_diskio.h"
#include "sd_spi.h"

// Check card status
DSTATUS SD_disk_status(BYTE drv)
{
  // Only drive 0 is supported
  if(drv != 0)
    return STA_NOINIT;

  // Return status based on your driver
  return card_initialized ? 0 : STA_NOINIT;
}

// Initialize card
DSTATUS SD_disk_initialize(BYTE drv)
{
  // Only drive 0 is supported
  if(drv != 0)
    return STA_NOINIT;

  // Initialize SD card using your driver
  if(SD_Init() == SD_OK)
    return 0;  // Success
  else
    return STA_NOINIT;
}

// Read blocks
DRESULT SD_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
  // Parameter checks
  if(pdrv != 0)
    return RES_PARERR;

  if(count == 0)
    return RES_PARERR;

  if(buff == NULL)
    return RES_PARERR;

  // Check if card is initialized
  if(!card_initialized)
    return RES_NOTRDY;

  // Read blocks using your driver
  if(SD_ReadBlocks(buff, sector, count) == SD_OK)
    return RES_OK;
  else
    return RES_ERROR;
}

// Write blocks
#if _USE_WRITE == 1
DRESULT SD_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
  // Parameter checks
  if(pdrv != 0)
    return RES_PARERR;

  if(count == 0)
    return RES_PARERR;

  if(buff == NULL)
    return RES_PARERR;

  // Check if card is initialized
  if(!card_initialized)
    return RES_NOTRDY;

  // Write blocks using your driver
  if(SD_WriteBlocks(buff, sector, count) == SD_OK)
    return RES_OK;
  else
    return RES_ERROR;
}
#endif

// Miscellaneous Functions (I/O Control)
#if _USE_IOCTL == 1
DRESULT SD_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
  // Only drive 0 is supported
  if(pdrv != 0)
    return RES_PARERR;

  // Check if card is initialized (except for sync command)
  if(!card_initialized && cmd != CTRL_SYNC)
    return RES_NOTRDY;

  switch(cmd)
  {
    // Make sure all pending writes are finished
    case CTRL_SYNC:
      // Your driver already waits in write functions
      return RES_OK;

      // Get sector size in bytes
    case GET_SECTOR_SIZE:
      if(buff == NULL)
        return RES_PARERR;
      *(WORD*) buff = 512;
      return RES_OK;

      // Get total number of sectors on the card
    case GET_SECTOR_COUNT:
      if(buff == NULL)
        return RES_PARERR;

      // Use a reasonable default based on card type
      if(SD_IsSDHC())
      {
        // For SDHC: 2GB = 4,194,304 sectors * 512 = 2GB
        // 4GB = 8,388,608 sectors, etc.
        // Use 4 million as a safe default (about 2GB)
        *(DWORD*) buff = 3900000;
      }
      else
      {
        // For SDSC: 32MB = 65,536 sectors * 512 = 32MB
        *(DWORD*) buff = 65536;
      }
      return RES_OK;

      // Get erase block size in sectors
    case GET_BLOCK_SIZE:
      if(buff == NULL)
        return RES_PARERR;
      *(DWORD*) buff = 1;  // Most SD cards use 1 sector
      return RES_OK;

      // Unknown command
    default:
      return RES_PARERR;
  }
}
#endif

// FatFS Driver Structure
const Diskio_drvTypeDef SD_Driver = {SD_disk_initialize,  // Initialize
SD_disk_status,      // Status
SD_disk_read,        // Read
#if _USE_WRITE == 1
    SD_disk_write,       // Write
#else
    NULL,
#endif
#if _USE_IOCTL == 1
    SD_disk_ioctl,       // I/O Control
#else
    NULL,
#endif
    };
