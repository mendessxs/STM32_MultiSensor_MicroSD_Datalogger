/*
 * spi1.c
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"

void SPI1_Init(void)
{
    SPI1->CR1 |= SPI_CR1_SPE;
}
