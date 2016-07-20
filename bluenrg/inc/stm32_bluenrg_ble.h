/**
  ******************************************************************************
  * @file    stm32_bluenrg_ble.h
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_BLUENRG_BLE_H
#define __STM32_BLUENRG_BLE_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/ 
#include "stm32f0xx.h"
#include "my_error.h"
  //#include "my_stm32l0xx_nucleo.h"
  //#include "stm32l0xx_nucleo_bluenrg.h"
#define SYSCLK_FREQ 	24000000
#define SPI_TIMOUT			15
   
/** @addtogroup BSP
 *  @{
 */

/** @addtogroup X-NUCLEO-IDB04A1
 *  @{
 */
 
/** @addtogroup STM32_BLUENRG_BLE
 *  @{
 */

/** @defgroup STM32_BLUENRG_BLE_Exported_Functions 
 * @{
 */
  
 // SPI Instance
 #define BNRG_SPI		        					SPI1

 // SPI Configuration
 #define BNRG_SPI_MODE			        	SPI_MODE_MASTER
 #define BNRG_SPI_DIRECTION		        SPI_DIRECTION_2LINES
 #define BNRG_SPI_DATASIZE		        SPI_DATASIZE_8BIT
 #define BNRG_SPI_CLKPOLARITY		    	SPI_POLARITY_LOW
 #define BNRG_SPI_CLKPHASE	        	SPI_PHASE_1EDGE
 #define BNRG_SPI_NSS			        		SPI_NSS_SOFT
 #define BNRG_SPI_FIRSTBIT	        	SPI_FIRSTBIT_MSB
 #define BNRG_SPI_TIMODE		        	SPI_TIMODE_DISABLED
 #define BNRG_SPI_CRCPOLYNOMIAL	      7
 #define BNRG_SPI_BAUDRATEPRESCALER   SPI_BAUDRATEPRESCALER_4
 #define BNRG_SPI_CRCCALCULATION		  SPI_CRCCALCULATION_DISABLED

 // SPI Reset Pin: PA.1
#define BNRG_SPI_RESET_PORT		      	GPIOA
 #define BNRG_SPI_RESET_PIN	        	GPIO_Pin_1
 #define BNRG_SPI_RESET_PIN_NUM      	1

 // SCLK: PA.5
#define BNRG_SPI_SCLK_PORT		        GPIOA
 #define BNRG_SPI_SCLK_PIN		        GPIO_Pin_5
#define BNRG_SPI_SCLK_PIN_NUM		      5

 // MISO (Master Input Slave Output): PA.6
#define BNRG_SPI_MISO_PORT		        GPIOA
 #define BNRG_SPI_MISO_PIN		        GPIO_Pin_6
#define BNRG_SPI_MISO_PIN_NUM		      6

 // MOSI (Master Output Slave Input): PA.7
#define BNRG_SPI_MOSI_PORT						GPIOA
 #define BNRG_SPI_MOSI_PIN						GPIO_Pin_7
#define BNRG_SPI_MOSI_PIN_NUM					7

 // NSS/CSN/CS: PA.1
#define BNRG_SPI_CS_PORT							GPIOA
 #define BNRG_SPI_CS_PIN							GPIO_Pin_4
#define BNRG_SPI_CS_PIN_NUM						4

 // IRQ: PA.0
#define BNRG_SPI_IRQ_PORT							GPIOA
#define BNRG_SPI_IRQ_PIN							GPIO_Pin_0
#define BNRG_SPI_IRQ_PIN_NUM					0
#define  BNRG_SPI_IRQ_EXTI_LINE				EXTI_Line0

 // EXTI External Interrupt for SPI
 // NOTE: if you change the IRQ pin remember to implement a corresponding handler
 // function like EXTI0_1_IRQHandler() in the user project
 #define BNRG_SPI_EXTI_IRQn           EXTI0_1_IRQn
 #define BNRG_SPI_EXTI_IRQHandler     EXTI0_1_IRQHandler
 #define BNRG_SPI_EXTI_PIN            BNRG_SPI_IRQ_PIN
 #define BNRG_SPI_EXTI_PORT           BNRG_SPI_IRQ_PORT
 #define RTC_WAKEUP_IRQHandler        RTC_IRQHandler

 //EXTI External Interrupt for user button
 #define PUSH_BUTTON_EXTI_IRQHandler  EXTI4_15_IRQHandler


 eError SPI_TransmitReceive(SPI_TypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout);

 void Enable_SPI_IRQ(void);
 void Disable_SPI_IRQ(void);
 void Clear_SPI_IRQ(void);
 void Clear_SPI_EXTI_Flag(void);


// FIXME: add prototypes for BlueNRG here
void SpiMspInit(SPI_TypeDef* hspi);
void BNRG_SPI_Init(void);
void BlueNRG_RST(void);
uint8_t BlueNRG_DataPresent(void);
void    BlueNRG_HW_Bootloader(void);
int32_t BlueNRG_SPI_Read_All(SPI_TypeDef *hspi, uint8_t *buffer, uint8_t buff_size);
int32_t BlueNRG_SPI_Write(SPI_TypeDef *hspi, uint8_t* data1, uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2);

#ifdef __cplusplus
}
#endif

#endif /* __STM32_BLUENRG_BLE_H */
    
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

