/*
 * tasks.h
 *
 *  Created on: Feb 22, 2026
 *      Author: Rubin Khadka
 */

#ifndef TASKS_H_
#define TASKS_H_

extern volatile uint8_t dht11_humidity;
extern volatile uint8_t dht11_humidity2;
extern volatile uint8_t dht11_temperature;
extern volatile uint8_t dht11_temperature2;

// Task Functions
void Task_MPU6050_Read(void);
void Task_DHT11_Read(void);
void Task_LCD_Update(void);
void Task_UART_Output(void);
void Task_DS18B20_Read(void);
void Task_Button_Status(void);
void Task_Feedback_Update(void);

#endif /* TASKS_H_ */
