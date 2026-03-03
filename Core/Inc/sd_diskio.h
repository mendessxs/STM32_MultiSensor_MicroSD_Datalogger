/*
 * sd_diskio.h
 *
 *  Created on: Mar 3, 2026
 *      Author: Rubin Khadka
 */

#ifndef INC_SD_DISKIO_H_
#define INC_SD_DISKIO_H_

#include "diskio.h"
#include "ff_gen_drv.h"

extern const Diskio_drvTypeDef SD_Driver;
DSTATUS SD_disk_status(BYTE drv);
DSTATUS SD_disk_initialize(BYTE drv);
DRESULT SD_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
DRESULT SD_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
DRESULT SD_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff);

#endif /* INC_SD_DISKIO_H_ */
