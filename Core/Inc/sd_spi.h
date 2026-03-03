/*
 * sd_spi.h
 *
 *  Created on: Mar 3, 2026
 *      Author: Rubin Khadka
 */

#ifndef INC_SD_SPI_H_
#define INC_SD_SPI_H_

#include "main.h"

#define CMD0    (0)
#define CMD8    (8)
#define CMD17   (17)
#define CMD24   (24)
#define CMD55   (55)
#define CMD58   (58)
#define ACMD41  (41)

typedef enum {
    SD_OK = 0,
    SD_ERROR
} SD_Status;

extern uint8_t card_initialized;

SD_Status SD_Init(void);
SD_Status SD_ReadBlocks(uint8_t *buff, uint32_t sector, uint32_t count);
SD_Status SD_WriteBlocks(const uint8_t *buff, uint32_t sector, uint32_t count);
SD_Status SD_ReadMultiBlocks(uint8_t *buff, uint32_t sector, uint32_t count);
SD_Status SD_WriteMultiBlocks(const uint8_t *buff, uint32_t sector, uint32_t count);
uint8_t SD_IsSDHC(void);

#endif /* INC_SD_SPI_H_ */
