/*
 * sd_data_logger.h
 *
 *  Created on: Mar 3, 2026
 *      Author: Rubin Khadka
 */

#ifndef INC_SD_DATA_LOGGER_H_
#define INC_SD_DATA_LOGGER_H_

#include "stdint.h"

// Return codes
#define SD_LOGGER_OK        0
#define SD_LOGGER_ERROR     1
#define SD_LOGGER_UNINIT    2
#define SD_LOGGER_BUSY      3

extern volatile uint32_t entry_count;

// Initialize the SD data logger
uint8_t SD_DataLogger_Init(void);

// Save current sensor data to CSV
uint8_t SD_DataLogger_SaveEntry(void);

// Read all entries and display in formatted table
uint32_t SD_DataLogger_ReadAll(void);

// Read last N entries
uint32_t SD_DataLogger_ReadLastN(uint32_t n);

// Get total number of logged entries
uint32_t SD_DataLogger_GetEntryCount(void);

// Periodic task
void Task_SD_DataLogger(void);

// Unmount SD card
void SD_DataLogger_Deinit(void);

#endif /* INC_SD_DATA_LOGGER_H_ */
