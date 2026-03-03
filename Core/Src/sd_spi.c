/*
 * sd_spi.c
 *
 *  Created on: Mar 3, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"
#include "timer2.h"
#include "sd_spi.h"

// CS Pin control
#define SD_CS_LOW()     (GPIOB->BRR = GPIO_BRR_BR6)   // Reset PB6 (CS low)
#define SD_CS_HIGH()    (GPIOB->BSRR = GPIO_BSRR_BS6) // Set PB6 (CS high)

// Variable declarations
static volatile uint8_t dma_tx_complete = 0;
static volatile uint8_t dma_rx_complete = 0;

static uint8_t sdhc = 0;
uint8_t card_initialized = 0;

// DMA channel2 (RX) IRQ handler
void DMA1_Channel2_IRQHandler(void)
{
  if(DMA1->ISR & DMA_ISR_TCIF2)
  {
    DMA1->IFCR = DMA_IFCR_CTCIF2;  // Clear TC flag for channel 2
    dma_rx_complete = 1;
  }
  DMA1->IFCR = DMA_IFCR_CGIF2;  // Clear all flags for channel 2
}

// DMA channel3 (TX) IRQ handler
void DMA1_Channel3_IRQHandler(void)
{
  if(DMA1->ISR & DMA_ISR_TCIF3)
  {
    DMA1->IFCR = DMA_IFCR_CTCIF3;  // Clear TC flag for channel 3
    dma_tx_complete = 1;
  }
  DMA1->IFCR = DMA_IFCR_CGIF3;  // Clear all flags for channel 3
}

// SPI Helper functions
static void SPI_WaitForTx(void)
{
  while(!(SPI1->SR & SPI_SR_TXE));
}

static void SPI_WaitForRx(void)
{
  while(!(SPI1->SR & SPI_SR_RXNE));
}

static void SPI_WaitForNotBusy(void)
{
  while(SPI1->SR & SPI_SR_BSY);
}

static void SPI_TransmitByte(uint8_t data)
{
  SPI_WaitForTx();
  SPI1->DR = data;
  SPI_WaitForNotBusy();  // Wait for transmission to complete
}

static uint8_t SPI_ReceiveByte(void)
{
  // Transmit dummy byte to generate clock
  SPI_WaitForTx();
  SPI1->DR = 0xFF;

  // Wait for reception
  SPI_WaitForRx();
  return (uint8_t) SPI1->DR;
}

static void SPI_TransmitBuffer_DMA(const uint8_t *buffer, uint16_t len)
{
  // Reset DMA completion flag
  dma_tx_complete = 0;

  // Disable DMA channel for configuration
  DMA1_Channel3->CCR &= ~DMA_CCR_EN;

  // Configure DMA for SPI1 TX
  DMA1_Channel3->CPAR = (uint32_t) &SPI1->DR;     // Peripheral address
  DMA1_Channel3->CMAR = (uint32_t) buffer;        // Memory address
  DMA1_Channel3->CNDTR = len;                     // Number of bytes

  // Configure: Memory-to-peripheral, 8-bit, increment memory, priority medium
  DMA1_Channel3->CCR = DMA_CCR_DIR |       // Memory to peripheral
      DMA_CCR_MINC |             // Memory increment
      DMA_CCR_PSIZE_0 |          // Peripheral size 8-bit
      DMA_CCR_MSIZE_0 |          // Memory size 8-bit
      DMA_CCR_PL_0 |             // Priority medium
      DMA_CCR_TCIE;              // Enable transfer complete interrupt

  // Enable SPI TX DMA
  SPI1->CR2 |= SPI_CR2_TXDMAEN;

  // Enable DMA channel
  DMA1_Channel3->CCR |= DMA_CCR_EN;

  // Wait for DMA completion
  uint32_t timeout = TIMER2_GetMillis() + 1000;
  while(!dma_tx_complete && (TIMER2_GetMillis() < timeout));

  // Disable SPI TX DMA
  SPI1->CR2 &= ~SPI_CR2_TXDMAEN;

  // Wait for SPI to finish
  SPI_WaitForNotBusy();

  // Disable DMA channel
  DMA1_Channel3->CCR &= ~DMA_CCR_EN;
}

static void SPI_ReceiveBuffer_DMA(uint8_t *buffer, uint16_t len)
{
  static uint8_t dummy_tx[512];  // Maximum block size
  uint16_t i;

  // Fill dummy buffer with 0xFF
  for(i = 0; i < len; i++)
  {
    dummy_tx[i] = 0xFF;
  }

  // Reset DMA completion flag
  dma_rx_complete = 0;

  // Disable DMA channels for configuration
  DMA1_Channel2->CCR &= ~DMA_CCR_EN;
  DMA1_Channel3->CCR &= ~DMA_CCR_EN;

  // Configure RX DMA
  DMA1_Channel2->CPAR = (uint32_t) &SPI1->DR;      // Peripheral address
  DMA1_Channel2->CMAR = (uint32_t) buffer;         // Memory address
  DMA1_Channel2->CNDTR = len;                      // Number of bytes

  // Configure RX: Peripheral-to-memory, 8-bit, increment memory, priority medium
  DMA1_Channel2->CCR = DMA_CCR_MINC |             // Memory increment
      DMA_CCR_PSIZE_0 |            // Peripheral size 8-bit
      DMA_CCR_MSIZE_0 |            // Memory size 8-bit
      DMA_CCR_PL_0 |               // Priority medium
      DMA_CCR_TCIE;                // Enable transfer complete interrupt

  // Configure TX DMA for dummy data
  DMA1_Channel3->CPAR = (uint32_t) &SPI1->DR;       // Peripheral address
  DMA1_Channel3->CMAR = (uint32_t) dummy_tx;        // Memory address
  DMA1_Channel3->CNDTR = len;                       // Number of bytes

  // Configure TX: Memory-to-peripheral, 8-bit, increment memory, priority medium
  DMA1_Channel3->CCR = DMA_CCR_DIR |              // Memory to peripheral
      DMA_CCR_MINC |               // Memory increment
      DMA_CCR_PSIZE_0 |            // Peripheral size 8-bit
      DMA_CCR_MSIZE_0 |            // Memory size 8-bit
      DMA_CCR_PL_0;                // Priority medium

  // Enable both RX and TX DMA in SPI
  SPI1->CR2 |= SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN;

  // Enable DMA channels (RX first, then TX)
  DMA1_Channel2->CCR |= DMA_CCR_EN;
  DMA1_Channel3->CCR |= DMA_CCR_EN;

  // Wait for RX completion
  uint32_t timeout = TIMER2_GetMillis() + 1000;
  while(!dma_rx_complete && (TIMER2_GetMillis() < timeout));

  // Disable DMA
  SPI1->CR2 &= ~(SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN);
  DMA1_Channel2->CCR &= ~DMA_CCR_EN;
  DMA1_Channel3->CCR &= ~DMA_CCR_EN;
}

static uint8_t SD_WaitReady(void)
{
  uint32_t timeout = TIMER2_GetMillis() + 500;
  uint8_t resp;

  do
  {
    resp = SPI_ReceiveByte();
    if(resp == 0xFF)
      return 0;  // Ready
  }
  while(TIMER2_GetMillis() < timeout);
  return 1;  // Timeout
}

static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc)
{
  uint8_t response;
  uint8_t retry = 0xFF;

  // Wait for card ready
  if(SD_WaitReady() != 0)
    return 0xFF;

  // Send command packet
  SPI_TransmitByte(0x40 | cmd);       // Command
  SPI_TransmitByte(arg >> 24);        // Argument MSB
  SPI_TransmitByte(arg >> 16);
  SPI_TransmitByte(arg >> 8);
  SPI_TransmitByte(arg);              // Argument LSB
  SPI_TransmitByte(crc);              // CRC

  // Read response
  do
  {
    response = SPI_ReceiveByte();
  }
  while((response & 0x80) && --retry);

  return response;
}

SD_Status SD_Init(void)
{
  uint8_t i, response;
  uint8_t r7[4];
  uint32_t retry;

  card_initialized = 0;
  sdhc = 0;

  // Initialization sequence
  SD_CS_HIGH();

  // Send 80 clocks (10 bytes of 0xFF) for card initialization
  for(i = 0; i < 10; i++)
  {
    SPI_TransmitByte(0xFF);
  }

  // CMD0 - Enter SPI mode (GO_IDLE_STATE)
  SD_CS_LOW();
  response = SD_SendCommand(0, 0, 0x95);  // CMD0 with valid CRC
  SD_CS_HIGH();
  SPI_TransmitByte(0xFF);  // Extra clock

  if(response != 0x01)
    return SD_INIT_ERROR;  // Expected R1: 0x01 (idle state)

  // CMD8 - Check for SDHC/SDXC (SEND_IF_COND)
  SD_CS_LOW();
  response = SD_SendCommand(8, 0x000001AA, 0x87);  // CMD8 with valid CRC
  for(i = 0; i < 4; i++)
  {
    r7[i] = SPI_ReceiveByte();
  }
  SD_CS_HIGH();
  SPI_TransmitByte(0xFF);

  retry = TIMER2_GetMillis() + 1000;  // 1 second timeout

  if(response == 0x01 && r7[2] == 0x01 && r7[3] == 0xAA)
  {
    // SDHC/SDXC card detected
    do
    {
      SD_CS_LOW();
      SD_SendCommand(55, 0, 0xFF);  // CMD55 (APP_CMD prefix)
      response = SD_SendCommand(41, 0x40000000, 0xFF);  // ACMD41 with HCS bit
      SD_CS_HIGH();
      SPI_TransmitByte(0xFF);
    }
    while(response != 0x00 && TIMER2_GetMillis() < retry);

    if(response != 0x00)
      return SD_INIT_ERROR;

    // CMD58 - Read OCR to confirm card type
    SD_CS_LOW();
    response = SD_SendCommand(58, 0, 0xFF);  // CMD58 (READ_OCR)
    for(i = 0; i < 4; i++)
    {
      r7[i] = SPI_ReceiveByte();  // OCR register bytes
    }
    SD_CS_HIGH();

    if(r7[0] & 0x40)
      sdhc = 1;  // Check CCS bit (bit 6)
  }
  else
  {
    // Standard SDSC card
    do
    {
      SD_CS_LOW();
      SD_SendCommand(55, 0, 0xFF);  // CMD55 (APP_CMD prefix)
      response = SD_SendCommand(41, 0, 0xFF);  // ACMD41 without HCS
      SD_CS_HIGH();
      SPI_TransmitByte(0xFF);
    }
    while(response != 0x00 && TIMER2_GetMillis() < retry);

    if(response != 0x00)
      return SD_INIT_ERROR;
  }

  card_initialized = 1;
  return SD_OK;
}

SD_Status SD_ReadSingleBlock(uint8_t *buff, uint32_t sector)
{
  if(!card_initialized)
    return SD_ERROR;

  // Adjust sector for SDSC cards (byte addressing)
  if(!sdhc)
    sector *= 512;

  SD_CS_LOW();

  // Send CMD17 (READ_SINGLE_BLOCK)
  if(SD_SendCommand(17, sector, 0xFF) != 0x00)
  {
    SD_CS_HIGH();
    return SD_ERROR;
  }

  // Wait for data token
  uint8_t token;
  uint32_t timeout = TIMER2_GetMillis() + 200;  // 200ms timeout

  do
  {
    token = SPI_ReceiveByte();
    if(token == 0xFE)
      break;  // Start token
  }
  while(TIMER2_GetMillis() < timeout);

  if(token != 0xFE)
  {
    SD_CS_HIGH();
    return SD_ERROR;
  }

  // Read 512 bytes using DMA
  SPI_ReceiveBuffer_DMA(buff, 512);

  // Discard 2-byte CRC
  SPI_ReceiveByte();
  SPI_ReceiveByte();

  SD_CS_HIGH();
  SPI_TransmitByte(0xFF);  // Extra clocks

  return SD_OK;
}

SD_Status SD_ReadMultiBlocks(uint8_t *buff, uint32_t sector, uint32_t count)
{
  if(!card_initialized || count == 0)
    return SD_ERROR;

  if(!sdhc)
    sector *= 512;

  SD_CS_LOW();

  // Send CMD18 (READ_MULTIPLE_BLOCK)
  if(SD_SendCommand(18, sector, 0xFF) != 0x00)
  {
    SD_CS_HIGH();
    return SD_ERROR;
  }

  while(count--)
  {
    uint8_t token;
    uint32_t timeout = TIMER2_GetMillis() + 200;

    do
    {
      token = SPI_ReceiveByte();
      if(token == 0xFE)
        break;
    }
    while(TIMER2_GetMillis() < timeout);

    if(token != 0xFE)
    {
      SD_CS_HIGH();
      return SD_ERROR;
    }

    // Read block using DMA
    SPI_ReceiveBuffer_DMA(buff, 512);

    // Discard CRC
    SPI_ReceiveByte();
    SPI_ReceiveByte();

    buff += 512;
  }

  // Send CMD12 (STOP_TRANSMISSION)
  SD_SendCommand(12, 0, 0xFF);

  SD_CS_HIGH();
  SPI_TransmitByte(0xFF);

  return SD_OK;
}

SD_Status SD_WriteSingleBlock(const uint8_t *buff, uint32_t sector)
{
  if(!card_initialized)
    return SD_ERROR;

  if(!sdhc)
    sector *= 512;

  SD_CS_LOW();

  // Send CMD24 (WRITE_SINGLE_BLOCK)
  if(SD_SendCommand(24, sector, 0xFF) != 0x00)
  {
    SD_CS_HIGH();
    return SD_ERROR;
  }

  // Send data token
  SPI_TransmitByte(0xFE);

  // Write data using DMA
  SPI_TransmitBuffer_DMA(buff, 512);

  // Send dummy CRC
  SPI_TransmitByte(0xFF);
  SPI_TransmitByte(0xFF);

  // Check response
  uint8_t resp = SPI_ReceiveByte();
  if((resp & 0x1F) != 0x05)
  {  // Data accepted pattern
    SD_CS_HIGH();
    return SD_ERROR;
  }

  // Wait for write completion
  uint32_t timeout = TIMER2_GetMillis() + 500;
  while(SPI_ReceiveByte() == 0 && TIMER2_GetMillis() < timeout);

  SD_CS_HIGH();
  SPI_TransmitByte(0xFF);

  return SD_OK;
}

SD_Status SD_WriteMultiBlocks(const uint8_t *buff, uint32_t sector, uint32_t count)
{
  if(!card_initialized || count == 0)
    return SD_ERROR;

  if(!sdhc)
    sector *= 512;

  SD_CS_LOW();

  // Send CMD25 (WRITE_MULTIPLE_BLOCK)
  if(SD_SendCommand(25, sector, 0xFF) != 0x00)
  {
    SD_CS_HIGH();
    return SD_ERROR;
  }

  while(count--)
  {
    // Send start block token for multi-write
    SPI_TransmitByte(0xFC);

    // Write data using DMA
    SPI_TransmitBuffer_DMA(buff, 512);

    // Send dummy CRC
    SPI_TransmitByte(0xFF);
    SPI_TransmitByte(0xFF);

    // Check response
    uint8_t resp = SPI_ReceiveByte();
    if((resp & 0x1F) != 0x05)
    {
      SD_CS_HIGH();
      return SD_ERROR;
    }

    // Wait for write completion
    uint32_t timeout = TIMER2_GetMillis() + 500;
    while(SPI_ReceiveByte() == 0 && TIMER2_GetMillis() < timeout);

    buff += 512;
  }

  // Send stop token
  SPI_TransmitByte(0xFD);

  // Wait for final completion
  uint32_t timeout = TIMER2_GetMillis() + 500;
  while(SPI_ReceiveByte() == 0 && TIMER2_GetMillis() < timeout);

  SD_CS_HIGH();
  SPI_TransmitByte(0xFF);

  return SD_OK;
}

uint8_t SD_IsSDHC(void)
{
  return sdhc;
}

uint8_t SD_IsInitialized(void)
{
  return card_initialized;
}

SD_Status SD_ReadBlocks(uint8_t *buff, uint32_t sector, uint32_t count)
{
  if(count == 1)
  {
    return SD_ReadSingleBlock(buff, sector);
  }
  else
  {
    return SD_ReadMultiBlocks(buff, sector, count);
  }
}

SD_Status SD_WriteBlocks(const uint8_t *buff, uint32_t sector, uint32_t count)
{
  if(count == 1)
  {
    return SD_WriteSingleBlock(buff, sector);
  }
  else
  {
    return SD_WriteMultiBlocks(buff, sector, count);
  }
}

