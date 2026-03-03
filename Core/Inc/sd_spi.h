/*
 * sd_spi.h
 *
 *  Created on: Mar 3, 2026
 *      Author: Rubin Khadka
 */

#ifndef INC_SD_SPI_H_
#define INC_SD_SPI_H_

#include "stdint.h"

// SD Card status codes
typedef enum
{
  SD_OK = 0,
  SD_ERROR = 1,
  SD_TIMEOUT = 2,
  SD_INIT_ERROR = 3
} SD_Status;

extern uint8_t card_initialized;

// Initialize SD card
SD_Status SD_Init(void);

// Check if card is SDHC
uint8_t SD_IsSDHC(void);

// Read operations
SD_Status SD_ReadSingleBlock(uint8_t *buff, uint32_t sector);
SD_Status SD_ReadMultiBlocks(uint8_t *buff, uint32_t sector, uint32_t count);

// Write operations
SD_Status SD_WriteSingleBlock(const uint8_t *buff, uint32_t sector);
SD_Status SD_WriteMultiBlocks(const uint8_t *buff, uint32_t sector, uint32_t count);

// Card status
uint8_t SD_IsInitialized(void);

SD_Status SD_ReadBlocks(uint8_t *buff, uint32_t sector, uint32_t count);
SD_Status SD_WriteBlocks(const uint8_t *buff, uint32_t sector, uint32_t count);

#endif /* INC_SD_SPI_H_ */
