/*
 * sd_data_logger.c
 *
 *  Created on: Mar 4, 2026
 *  Author: Rubin Khadka
 */

#include "sd_data_logger.h"
#include "sd_functions.h"
#include "mpu6050.h"
#include "ds18b20.h"
#include "dht11.h"
#include "timer2.h"
#include "uart.h"
#include "utils.h"
#include "tasks.h"

#define CSV_FILENAME        "test_data.csv"
#define CSV_HEADER          "Entry,DS18B20_C,MPU6050_C,DHT11_C,DHT11_%,AX_g,AY_g,AZ_g,GX_dps,GY_dps,GZ_dps\r\n"
#define MAX_LINE_LENGTH     128

static uint8_t initialized = 0;
static uint32_t entry_count = 0;

// Structure to track logger state
static struct
{
  uint32_t total_entries;
  uint8_t file_exists;
} sd_logger;

// Helper: Format float with specified width
static void print_csv_float(char **ptr, float value, uint8_t decimals)
{
  char buffer[16];
  ftoa(value, buffer, decimals);
  char *s = buffer;
  while(*s)
    *(*ptr)++ = *s++;
}

// Helper: Format integer
static void print_csv_int(char **ptr, uint32_t value)
{
  char buffer[16];
  itoa_32(value, buffer);
  char *s = buffer;
  while(*s)
    *(*ptr)++ = *s++;
}

// Format DHT11 data with decimal point (integer part and decimal part)
static void format_dht11_data(char **ptr, uint8_t integer_part, uint8_t decimal_part)
{
  char buffer[8];
  char *buf_ptr = buffer;

  // Integer part
  if(integer_part >= 10)
  {
    *buf_ptr++ = '0' + (integer_part / 10);
  }
  *buf_ptr++ = '0' + (integer_part % 10);

  // Decimal point
  *buf_ptr++ = '.';

  // Decimal part (tenths)
  *buf_ptr++ = '0' + decimal_part;
  *buf_ptr = '\0';

  // Copy to output
  char *s = buffer;
  while(*s)
    *(*ptr)++ = *s++;
}

// Initialize SD data logger
uint8_t SD_DataLogger_Init(void)
{
  USART1_SendString("Initializing SD Data Logger...\r\n");

  // First, mount the SD card
  if(sd_mount() != FR_OK)
  {
    USART1_SendString("SD Card mount failed!\r\n");
    return SD_LOGGER_ERROR;
  }

  // Check if CSV file exists and count entries
  FIL file;
  uint32_t lines = 0;

  FRESULT res = f_open(&file, CSV_FILENAME, FA_READ);
  if(res == FR_OK)
  {
    // File exists - count lines by reading characters until EOF
    UINT bytes_read;
    uint8_t temp_buf[32];
    uint8_t last_char = 0;

    while(1)
    {
      res = f_read(&file, temp_buf, 1, &bytes_read);
      if(res != FR_OK || bytes_read == 0)
        break;

      if(temp_buf[0] == '\n')
      {
        lines++;
      }
      last_char = temp_buf[0];
    }

    // If file doesn't end with newline, add one to line count
    if(last_char != '\n' && lines > 0)
    {
      lines++;
    }

    f_close(&file);

    // Subtract header line
    if(lines > 0)
    {
      entry_count = lines - 1;
      sd_logger.file_exists = 1;
    }

    USART1_SendString("Found existing CSV with ");
    USART1_SendNumber(entry_count);
    USART1_SendString(" data entries\r\n");
  }
  else
  {
    // File doesn't exist - create with header
    USART1_SendString("No CSV file found. Creating new file...\r\n");

    if(sd_write_file(CSV_FILENAME, CSV_HEADER) == FR_OK)
    {
      USART1_SendString("CSV header created successfully\r\n");
      entry_count = 0;
      sd_logger.file_exists = 1;
    }
    else
    {
      USART1_SendString("Failed to create CSV file!\r\n");
      sd_unmount();
      return SD_LOGGER_ERROR;
    }
  }

  sd_logger.total_entries = entry_count;
  initialized = 1;

  USART1_SendString("SD Data Logger ready. Total entries: ");
  USART1_SendNumber(entry_count);
  USART1_SendString("\r\n");

  return SD_LOGGER_OK;
}

// Save current sensor data to CSV
uint8_t SD_DataLogger_SaveEntry(void)
{
  if(!initialized)
  {
    return SD_LOGGER_UNINIT;
  }

  char csv_line[MAX_LINE_LENGTH];
  char *ptr = csv_line;

  // Entry number (instead of timestamp)
  print_csv_int(&ptr, entry_count + 1);  // Start from 1
  *ptr++ = ',';

  // DS18B20 temperature (float with 2 decimals)
  print_csv_float(&ptr, ds18b20_data.temperature, 2);
  *ptr++ = ',';

  // MPU6050 temperature (float with 2 decimals)
  print_csv_float(&ptr, mpu6050_scaled.temp, 2);
  *ptr++ = ',';

  // DHT11 temperature with decimal point
  format_dht11_data(&ptr, dht11_temperature, dht11_temperature2);
  *ptr++ = ',';

  // DHT11 humidity with decimal point
  format_dht11_data(&ptr, dht11_humidity, dht11_humidity2);
  *ptr++ = ',';

  // Accelerometer X (float with 3 decimals for higher precision)
  print_csv_float(&ptr, mpu6050_scaled.accel_x, 3);
  *ptr++ = ',';

  // Accelerometer Y
  print_csv_float(&ptr, mpu6050_scaled.accel_y, 3);
  *ptr++ = ',';

  // Accelerometer Z
  print_csv_float(&ptr, mpu6050_scaled.accel_z, 3);
  *ptr++ = ',';

  // Gyro X (float with 2 decimals)
  print_csv_float(&ptr, mpu6050_scaled.gyro_x, 2);
  *ptr++ = ',';

  // Gyro Y
  print_csv_float(&ptr, mpu6050_scaled.gyro_y, 2);
  *ptr++ = ',';

  // Gyro Z
  print_csv_float(&ptr, mpu6050_scaled.gyro_z, 2);

  // End line
  *ptr++ = '\r';
  *ptr++ = '\n';
  *ptr = '\0';

  // Append to CSV file
  if(sd_append_file(CSV_FILENAME, csv_line) != FR_OK)
  {
    USART1_SendString("Failed to append to CSV!\r\n");
    return SD_LOGGER_ERROR;
  }

  entry_count++;
  sd_logger.total_entries = entry_count;

  return SD_LOGGER_OK;
}

// Lightweight line display - no parsing, just raw output with formatting
uint32_t SD_DataLogger_ReadAll(void)
{
  if(!initialized)
  {
    USART1_SendString("Logger not initialized!\r\n");
    return 0;
  }

  FIL file;
  char line[MAX_LINE_LENGTH];
  UINT bytes_read;
  uint32_t lines_read = 0;
  uint32_t line_pos = 0;
  uint8_t in_line = 0;

  FRESULT res = f_open(&file, CSV_FILENAME, FA_READ);
  if(res != FR_OK)
  {
    USART1_SendString("Failed to open CSV file! Error: ");
    USART1_SendNumber(res);
    USART1_SendString("\r\n");
    return 0;
  }

  // Print table header
  USART1_SendString("\r\n");
  USART1_SendString("+------+-------------------------------------------------------------+\r\n");
  USART1_SendString("| Line | Data                                                        |\r\n");
  USART1_SendString("+------+-------------------------------------------------------------+\r\n");

  // Read character by character and display lines
  while(1)
  {
    uint8_t ch;
    res = f_read(&file, &ch, 1, &bytes_read);

    if(res != FR_OK || bytes_read == 0)
    {
      break;
    }

    // Start of new line
    if(!in_line)
    {
      // Skip header line (line 0)
      if(lines_read == 0)
      {
        if(ch == '\n')
        {
          lines_read++;
        }
        continue;
      }

      USART1_SendString("| ");

      // Print line number
      if(lines_read < 10)
        USART1_SendString("  ");
      else if(lines_read < 100)
        USART1_SendString(" ");
      USART1_SendNumber(lines_read);
      USART1_SendString(" | ");

      in_line = 1;
      line_pos = 0;
    }

    // Store character in line buffer
    if(line_pos < MAX_LINE_LENGTH - 1)
    {
      line[line_pos++] = ch;
    }

    // End of line
    if(ch == '\n')
    {
      line[line_pos] = '\0';

      // Display the line (up to 60 chars for formatting)
      for(uint32_t i = 0; i < line_pos && i < 60; i++)
      {
        if(line[i] == '\r')
          continue; // Skip carriage return
        USART1_SendChar(line[i]);
      }

      // Pad if needed
      if(line_pos < 60)
      {
        for(uint32_t i = line_pos; i < 60; i++)
        {
          USART1_SendChar(' ');
        }
      }

      USART1_SendString(" |\r\n");
      lines_read++;
      in_line = 0;
    }
  }

  // Print table footer
  USART1_SendString("+------+-------------------------------------------------------------+\r\n");
  USART1_SendString("Total data entries: ");
  USART1_SendNumber(lines_read - 1);  // Subtract header
  USART1_SendString("\r\n\r\n");

  f_close(&file);
  return lines_read - 1;
}

// Read last N entries (lightweight version)
uint32_t SD_DataLogger_ReadLastN(uint32_t n)
{
  if(!initialized || entry_count == 0)
  {
    USART1_SendString("No data entries found\r\n");
    return 0;
  }

  FIL file;
  char line[MAX_LINE_LENGTH];
  uint32_t lines_to_read = (n < entry_count) ? n : entry_count;
  uint32_t start_line = entry_count - lines_to_read + 1; // +1 for header

  FRESULT res = f_open(&file, CSV_FILENAME, FA_READ);
  if(res != FR_OK)
    return 0;

  USART1_SendString("\r\n=== Last ");
  USART1_SendNumber(lines_to_read);
  USART1_SendString(" Entries ===\r\n");

  // Skip to start position (including header)
  uint32_t current_line = 0;
  uint8_t ch;
  UINT bytes_read;
  uint32_t line_pos = 0;
  uint8_t in_line = 0;

  while(current_line < start_line)
  {
    res = f_read(&file, &ch, 1, &bytes_read);
    if(res != FR_OK || bytes_read == 0)
      break;

    if(ch == '\n')
    {
      current_line++;
    }
  }

  // Read and display remaining lines
  uint32_t displayed = 0;
  line_pos = 0;
  in_line = 0;

  while(displayed < lines_to_read)
  {
    res = f_read(&file, &ch, 1, &bytes_read);
    if(res != FR_OK || bytes_read == 0)
      break;

    if(!in_line)
    {
      USART1_SendString("[");
      USART1_SendNumber(displayed + 1);
      USART1_SendString("] ");
      in_line = 1;
      line_pos = 0;
    }

    if(ch == '\n')
    {
      line[line_pos] = '\0';
      USART1_SendString(line);
      USART1_SendString("\r\n");
      displayed++;
      in_line = 0;
      line_pos = 0;
    }
    else if(ch != '\r')
    {
      if(line_pos < MAX_LINE_LENGTH - 1)
      {
        line[line_pos++] = ch;
        USART1_SendChar(ch);
      }
    }
  }

  USART1_SendString("\r\n");
  f_close(&file);
  return displayed;
}

// Get entry count
uint32_t SD_DataLogger_GetEntryCount(void)
{
  return entry_count;
}

// Periodic task to log data (call this in main loop)
void Task_SD_DataLogger(void)
{
  USART1_SendString("Task_SD_DataLogger called\r\n");  // DEBUG
  if(!initialized)
  {
    USART1_SendString("Not initialized!\r\n");  // DEBUG
    return;
  }

  if(SD_DataLogger_SaveEntry() == SD_LOGGER_OK)
  {
    USART1_SendString("Logged entry #");
    USART1_SendNumber(entry_count);
    USART1_SendString("\r\n");
  }
  else
  {
    USART1_SendString("Save FAILED!\r\n");  // DEBUG
  }
}

// Clean up - unmount SD card
void SD_DataLogger_Deinit(void)
{
  if(initialized)
  {
    sd_unmount();
    initialized = 0;
    USART1_SendString("SD Data Logger deinitialized\r\n");
  }
}
