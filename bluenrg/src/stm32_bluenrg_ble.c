/**
  ******************************************************************************
  * @file    stm32_bluenrg_ble.c
  * @author  CL
  * @version V1.0.0
  * @date    04-July-2014
  * @brief   
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 
  
/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include "stm32f0xx.h"
#include "stm32_bluenrg_ble.h"
#include "my_time.h"
#include "gp_timer.h"
#include "my_error.h"
#include "my_main.h"
#include "my_service.h"

extern __IO uint32_t myTick;
extern volatile uint32_t resetCount; // Таймаут отсутствия активности на bluetoth.
extern struct _blue blue;
eError bnrgErr;

/** @addtogroup BSP
 *  @{
 */

#define HEADER_SIZE 5
#define MAX_BUFFER_SIZE 255
#define TIMEOUT_DURATION 15

/* Private function prototypes -----------------------------------------------*/
static void us150Delay(void);
void set_irq_as_output(void);
void set_irq_as_input(void);

/**
 * @brief  This function is used for low level initialization of the SPI 
 *         communication with the BlueNRG Expansion Board.
 * @param  hspi: SPI handle.
 * @retval None
 */
void SpiMspInit(SPI_TypeDef* hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  EXTI_InitTypeDef EXTI_InitStruct;
  if(hspi == BNRG_SPI) {
    /* Enable peripherals clock */

  	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  		// Clock GPIOA enable
 		RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
 		// Clock DMA enable
 		RCC->AHBENR |= RCC_AHBENR_DMAEN;
 		RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;               // включить тактирование SYSCFG

    /* Reset */
    GPIO_InitStruct.GPIO_Pin = BNRG_SPI_RESET_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(BNRG_SPI_RESET_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_1;
    BNRG_SPI_RESET_PORT->BRR |= BNRG_SPI_RESET_PIN;			// Added to avoid spurious interrupt from the BlueNRG

    /* SCLK */
    GPIO_InitStruct.GPIO_Pin = BNRG_SPI_SCLK_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
    GPIO_Init(BNRG_SPI_SCLK_PORT, &GPIO_InitStruct);
    BNRG_SPI_SCLK_PORT->AFR[BNRG_SPI_SCLK_PIN >> 0x3] &= ~(0xF << ((BNRG_SPI_SCLK_PIN_NUM & 0x7) * 4));		// AF0

    /* MISO */
    GPIO_InitStruct.GPIO_Pin = BNRG_SPI_MISO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
    GPIO_Init(BNRG_SPI_SCLK_PORT, &GPIO_InitStruct);
    BNRG_SPI_MISO_PORT->AFR[BNRG_SPI_MISO_PIN >> 0x3] &= ~(0xF << ((BNRG_SPI_MISO_PIN_NUM & 0x7) * 4));		// AF0

    /* MOSI */
    GPIO_InitStruct.GPIO_Pin = BNRG_SPI_MOSI_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
    GPIO_Init(BNRG_SPI_SCLK_PORT, &GPIO_InitStruct);
    BNRG_SPI_MOSI_PORT->AFR[BNRG_SPI_MOSI_PIN >> 0x3] &= ~(0xF << ((BNRG_SPI_MOSI_PIN_NUM & 0x7) * 4));		// AF0

    /* NSS/CSN/CS */
    GPIO_InitStruct.GPIO_Pin = BNRG_SPI_CS_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
    GPIO_Init(BNRG_SPI_SCLK_PORT, &GPIO_InitStruct);
    BNRG_SPI_SCLK_PORT->AFR[BNRG_SPI_SCLK_PIN >> 0x3] &= ~(0xF << ((BNRG_SPI_SCLK_PIN_NUM & 0x7) * 4));		// AF0

    /* IRQ -- INPUT */

    GPIO_InitStruct.GPIO_Pin = BNRG_SPI_IRQ_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;

    GPIO_Init(BNRG_SPI_IRQ_PORT, &GPIO_InitStruct);

    EXTI_InitStruct.EXTI_Line = BNRG_SPI_IRQ_EXTI_LINE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init( &EXTI_InitStruct );

    /* Configure the NVIC for SPI */  
    NVIC_SetPriority(BNRG_SPI_EXTI_IRQn, 3);
    NVIC_EnableIRQ(BNRG_SPI_EXTI_IRQn);
  }
}

/**
 * @brief  Initializes the SPI communication with the BlueNRG
 *         Expansion Board.
 * @param  None
 * @retval None
 */
void BNRG_SPI_Init(void)
{
	SPI_InitTypeDef SPI_InitStruct;
	// --------------- Set SPI init structure parameters values -----------------
	  /* Initialize the SPI_Direction member */
	  SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	  /* Initialize the SPI_Mode member */
	  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	  /* Initialize the SPI_DataSize member */
	  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	  /* Initialize the SPI_CPOL member */
	  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	  /* Initialize the SPI_CPHA member */
	  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	  /* Initialize the SPI_NSS member */
	  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	  /* Initialize the SPI_BaudRatePrescaler member */
	  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	  /* Initialize the SPI_FirstBit member */
	  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	  /* Initialize the SPI_CRCPolynomial member */
	  SPI_InitStruct.SPI_CRCPolynomial = 7;

	  SpiMspInit( BNRG_SPI );
	  SPI_Init( BNRG_SPI, &SPI_InitStruct );

	  SPI_TIModeCmd( BNRG_SPI, DISABLE );
	  SPI_CalculateCRC( BNRG_SPI, DISABLE );
// Configures the FIFO reception threshold for the selected SPI.
	  /* Clear FRXTH bit */
	  BNRG_SPI->CR2 &= (uint16_t)~((uint16_t)SPI_CR2_FRXTH);
	  /* Set new FRXTH bit value */
	  BNRG_SPI->CR2 |= SPI_CR2_FRXTH;

	  SPI_Cmd( BNRG_SPI, ENABLE );
}

/**
 * @brief  Resets the BlueNRG.
 * @param  None
 * @retval None
 */
void BlueNRG_RST(void)
{
  BNRG_SPI_RESET_PORT->BRR |= BNRG_SPI_RESET_PIN;
  myDelay(5);
  BNRG_SPI_RESET_PORT->BSRR |= BNRG_SPI_RESET_PIN;
  myDelay(5);
}

void bnrgFullRst( void ){
	// BlueNRG завис - перезапускаем
		RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
		for(uint8_t i = 0; i < 2; i++){
			__NOP();
		}
		RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
		BNRG_SPI_Init();
		if(BlueNRG_Init() != BLE_STATUS_SUCCESS){
			blue.bleStatus = BLE_STATUS_TIMEOUT;
			return;
		}
		resetCount = 120;
		toCurCharUpdate();
		ddCurCharUpdate();
		rtcCharUpdate(  uxTime );
		minMaxCharUpdate();
		GAP_DisconnectionComplete_CB();
}

/**
 * @brief  Reports if the BlueNRG has data for the host micro.
 * @param  None
 * @retval 1 if data are present, 0 otherwise
 */
// FIXME: find a better way to handle this return value (bool type? TRUE and FALSE)
uint8_t BlueNRG_DataPresent(void)
{
  if ( (BNRG_SPI_EXTI_PORT->IDR & BNRG_SPI_EXTI_PIN) == BNRG_SPI_EXTI_PIN )
      return TRUE;
  else  
      return FALSE;
} /* end BlueNRG_DataPresent() */

/**
 * @brief  Activate internal bootloader using pin.
 * @param  None
 * @retval None
 */
void BlueNRG_HW_Bootloader(void)
{
  set_irq_as_output();
  BlueNRG_RST();
  set_irq_as_input();
}

/**
 * @brief  Reads from BlueNRG SPI buffer and store data into local buffer.
 * @param  hspi     : SPI handle
 * @param  buffer   : Buffer where data from SPI are stored
 * @param  buff_size: Buffer size
 * @retval int32_t  : Number of read bytes
 */
int32_t BlueNRG_SPI_Read_All(SPI_TypeDef *hspi, uint8_t *buffer,
                             uint8_t buff_size)
{
  uint16_t byte_count;
  uint8_t len = 0;
  uint8_t char_ff = 0xff;
  volatile uint8_t read_char;


  uint8_t header_master[HEADER_SIZE] = {0x0b, 0x00, 0x00, 0x00, 0x00};
  uint8_t header_slave[HEADER_SIZE];

  /* CS reset */
  BNRG_SPI_CS_PORT->BRR |= BNRG_SPI_CS_PIN;

  /* Read the header */  
  SPI_TransmitReceive(hspi, header_master, header_slave, HEADER_SIZE, TIMEOUT_DURATION);
  
  if (header_slave[0] == 0x02) {
    /* device is ready */
    byte_count = (header_slave[4]<<8)|header_slave[3];
  
    if (byte_count > 0) {
  
      /* avoid to read more data that size of the buffer */
      if (byte_count > buff_size){
        byte_count = buff_size;
      }
  
      for (len = 0; len < byte_count; len++){
        SPI_TransmitReceive(hspi, &char_ff, (uint8_t*)&read_char, 1, TIMEOUT_DURATION);
        buffer[len] = read_char;
      }
    }    
  }
  /* Release CS line */
  BNRG_SPI_CS_PORT->BSRR |= BNRG_SPI_CS_PIN;
  
  // Add a small delay to give time to the BlueNRG to set the IRQ pin low
  // to avoid a useless SPI read at the end of the transaction
  for(volatile int i = 0; i < 2; i++)__NOP();
  
  return len;   
}

/**
 * @brief  Writes data from local buffer to SPI.
 * @param  hspi     : SPI handle
 * @param  data1    : First data buffer to be written
 * @param  data2    : Second data buffer to be written
 * @param  Nb_bytes1: Size of first data buffer to be written
 * @param  Nb_bytes2: Size of second data buffer to be written
 * @retval Number of read bytes
 */
int32_t BlueNRG_SPI_Write(SPI_TypeDef *hspi, uint8_t* data1,
                          uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2)
{  
  int32_t result = 0;
  
  int32_t spi_fix_enabled = 0;

#define ENABLE_SPI_FIX
  
#ifdef ENABLE_SPI_FIX
  spi_fix_enabled = 1;
#endif //ENABLE_SPI_FIX
  
  unsigned char header_master[HEADER_SIZE] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  unsigned char header_slave[HEADER_SIZE]  = {0xaa, 0x00, 0x00, 0x00, 0x00};
  
  unsigned char read_char_buf[MAX_BUFFER_SIZE];

  Disable_SPI_IRQ(); 

  /*
   If the SPI_FIX is enabled the IRQ is set in Output mode, then it is pulled
   high and, after a delay of at least 112us, the CS line is asserted and the
   header transmit/receive operations are started.
   After these transmit/receive operations the IRQ is reset in input mode.
 */
  if (spi_fix_enabled) {
    set_irq_as_output();

    /* Assert CS line after at least 112us */
    us150Delay();
}

  /* CS reset */
  BNRG_SPI_CS_PORT->BRR |= BNRG_SPI_CS_PIN;

  /* Exchange header */  
  SPI_TransmitReceive(hspi, header_master, header_slave, HEADER_SIZE, TIMEOUT_DURATION);
  
  if (spi_fix_enabled) {
    set_irq_as_input();
}

  if (header_slave[0] == 0x02) {
    /* SPI is ready */
    if (header_slave[1] >= (Nb_bytes1+Nb_bytes2)) {
  
      /*  Buffer is big enough */
      if (Nb_bytes1 > 0) {
        SPI_TransmitReceive(hspi, data1, read_char_buf, Nb_bytes1, TIMEOUT_DURATION);
      }
      if (Nb_bytes2 > 0) {
        SPI_TransmitReceive(hspi, data2, read_char_buf, Nb_bytes2, TIMEOUT_DURATION);
}

    } else {
      /* Buffer is too small */
      result = -2;
      }
  } else {
    /* SPI is not ready */
    result = -1;
      }
    
    /* Release CS line */
  BNRG_SPI_CS_PORT->BSRR |= BNRG_SPI_CS_PIN;
    
  Enable_SPI_IRQ();
    
  return result;
}
      
/**
 * @brief  Set in Output mode the IRQ.
 * @param  None
 * @retval None
 */
void set_irq_as_output(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Pull IRQ high */
  GPIO_InitStructure.GPIO_Pin = BNRG_SPI_IRQ_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(BNRG_SPI_IRQ_PORT, &GPIO_InitStructure);
  BNRG_SPI_IRQ_PORT->BSRR |= BNRG_SPI_IRQ_PIN;			// Added to avoid spurious interrupt from the BlueNRG

}

/**
 * @brief  Set the IRQ in input mode.
 * @param  None
 * @retval None
 */
void set_irq_as_input(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* IRQ input */  
  GPIO_InitStructure.GPIO_Pin = BNRG_SPI_IRQ_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(BNRG_SPI_IRQ_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(BNRG_SPI_IRQ_PORT, &GPIO_InitStructure);
}

/**
 * @brief  Utility function for delay
 * @param  None
 * @retval None
 * NOTE: TODO: implement with clock-independent function.
 */
static void us150Delay(void)
{
#if SYSCLK_FREQ == 4000000
  for(volatile int i = 0; i < 35; i++)__NOP();
#elif SYSCLK_FREQ == 32000000
  for(volatile int i = 0; i < 420; i++)__NOP();
#elif SYSCLK_FREQ == 84000000
  for(volatile int i = 0; i < 1125; i++)__NOP();
#else
  for(volatile int i = 0; i < 1125; i++)__NOP();
//#error Implement delay function.
#endif    
}

/**
 * @brief  Enable SPI IRQ.
 * @param  None
 * @retval None
 */
void Enable_SPI_IRQ(void)
{
  NVIC_EnableIRQ(BNRG_SPI_EXTI_IRQn);
}

/**
 * @brief  Disable SPI IRQ.
 * @param  None
 * @retval None
 */
void Disable_SPI_IRQ(void)
{ 
  NVIC_DisableIRQ(BNRG_SPI_EXTI_IRQn);
}

/**
 * @brief  Clear Pending SPI IRQ.
 * @param  None
 * @retval None
 */
void Clear_SPI_IRQ(void)
{
  NVIC_ClearPendingIRQ(BNRG_SPI_EXTI_IRQn);
}

/**
 * @brief  Clear EXTI (External Interrupt) line for SPI IRQ.
 * @param  None
 * @retval None
 */
void Clear_SPI_EXTI_Flag(void)
{  
  EXTI->PR = BNRG_SPI_EXTI_PIN;
}

eError SPI_TransmitReceive(SPI_TypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout)
{
__IO uint16_t tmpreg;
  uint32_t tout = myTick + Timeout;

  assert_param(IS_SPI_DIRECTION_2LINES(hspi->Init.Direction));


  if((pTxData == NULL) || (pRxData == NULL) || (Size == 0))
  {
    return ERR_ERROR;
  }

  uint8_t *pRxBuffPtr  = pRxData;
  uint8_t RxXferCount = Size;
  uint8_t *pTxBuffPtr  = pTxData;
  uint8_t TxXferCount = Size;

  /* Set the Rx Fido threshold */
  if( RxXferCount > 1 )
  {
    /* set fiforxthreshold according the reception data length: 16bit */
    hspi->CR2 &= ~SPI_CR2_FRXTH;
  }
  else
  {
    /* set fiforxthreshold according the reception data length: 8bit */
    hspi->CR2 |= SPI_CR2_FRXTH;
  }

  /* Check if the SPI is already enabled */
  if((hspi->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
  {
    /* Enable SPI peripheral */
  	hspi->CR1 |= SPI_CR1_SPE;
  }

  // Transmit and Receive data in 8 Bit mode
  while((TxXferCount > 0) || (RxXferCount > 0)) {
    // check TXE flag
    if((TxXferCount > 0) && ((hspi->SR & SPI_SR_TXE) == SPI_SR_TXE)) {
      if(TxXferCount > 1) {
        hspi->DR = *((uint16_t*)pTxBuffPtr);
        pTxBuffPtr += sizeof(uint16_t);
        TxXferCount -= 2;
      }
      else {
        *(__IO uint8_t *)&hspi->DR = (*pTxBuffPtr++);
        TxXferCount--;
      }
    }

    // Wait until RXNE flag is reset
    if( (RxXferCount > 0) && ((hspi->SR & SPI_SR_RXNE) == SPI_SR_RXNE) ) {
      if( RxXferCount > 1 ) {
        *((uint16_t*)pRxBuffPtr) = hspi->DR;
        pRxBuffPtr += sizeof(uint16_t);
        RxXferCount -= 2;
        if( RxXferCount <= 1 ) {
          // set fiforxthresold before to switch on 8 bit data size
          hspi->CR2 |= SPI_CR2_FRXTH;
        }
      }
      else {
      	*(pRxBuffPtr++) =  *(__IO uint8_t *)&hspi->DR;
          RxXferCount--;
      }
    }
    if( Timeout && ( myTick >= tout ) ) {
      return ERR_TIMEOUT;
    }
  }

  /* Check if CRC error occurred */
  if( (hspi->SR = SPI_SR_CRCERR) != RESET)
  {
    /* Clear CRC Flag */
    hspi->SR &= SPI_SR_CRCERR;

    return ERR_ERROR;
  }

  // Check the end of the transaction
  tout = myTick + Timeout;
  while( (hspi->SR & SPI_SR_FTLVL) ) {
    tmpreg = *((__IO uint8_t*)&hspi->DR);
    /* To avoid GCC warning */
    UNUSED(tmpreg);
  	if ( myTick > tout) {
  		return ERR_TIMEOUT;
  	}
  }
  tout = myTick + Timeout;
  while( (hspi->SR & SPI_SR_BSY) ) {
  	if ( myTick > tout) {
  		return ERR_TIMEOUT;
  	}
  }

  return ERR_OK;
}


/**
* @}
*/

/**
 * @}
 */

/**
 * @}
 */
   
/**
 * @}
 */
 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
