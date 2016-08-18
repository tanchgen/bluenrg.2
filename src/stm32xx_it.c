/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "eeprom.h"
#include "my_main.h"
#include "my_time.h"
#include "onewire.h"
#include "stm32xx_it.h"
#include "stm32_bluenrg_ble.h"
//#include "my_service.h"
//#include "my_stm32l0xx_nucleo.h"
//#include "stm32l0xx_nucleo_bluenrg.h"
#include "hci.h"

extern uint8_t ow_buf[];
extern uint8_t rxCount;
extern uint8_t txCount;

__IO uint32_t myTick = 0;

/** @defgroup INTERRUPT_HANDLER
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t ms_counter = 0;
volatile uint8_t button_event = 0;

/* SPI handler declared in "main.c" file */
//extern SPI_HandleTypeDef SpiHandle;
/* Private function prototypes -----------------------------------------------*/
//void myTimeOut( void );
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0+ Processor Exceptions Handlers                         */
/******************************************************************************/
void WWDG_IRQHandler( void)
{
  while(1);
}

void RTC_IRQHandler( void)
{
  while(1);
}

void FLASH_IRQHandler( void)
{
  while(1);
}

void RCC_IRQHandler( void)
{
  while(1);
}

void EXTI2_3_IRQHandler( void)
{
  while(1);
}

void DMA1_Channel1_IRQHandler( void)
{
  while(1);
}

void DMA1_Channel2_3_IRQHandler( void)
{
  while(1);
}

void DMA1_Channel4_5_IRQHandler( void)
{
  while(1);
}

void ADC1_IRQHandler( void)
{
  while(1);
}

void TIM1_BRK_UP_TRG_COM_IRQHandler( void)
{
  while(1);
}

void TIM1_CC_IRQHandler( void)
{
  while(1);
}

void TIM3_IRQHandler( void)
{
  while(1);
}

void TIM14_IRQHandler( void)
{
  while(1);
}

void TIM16_IRQHandler( void)
{
  while(1);
}

void TIM17_IRQHandler( void)
{
  while(1);
}

void SPI1_IRQHandler( void)
{
  while(1);
}

/**
  * @brief  NMI_Handler This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  HardFault_Handler This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  SVC_Handler This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  DebugMon_Handler This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  PendSV_Handler This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  SysTick_Handler This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//tBleStatus aci_gap_terminate(uint16_t conn_handle, uint8_t reason);
void SysTick_Handler(void)
{
//  HAL_IncTick();
  myTick++;
  timersHandler();
  // Если disconnCount > 0, значит таймер ожидания SHA-хэша включен.
}


/******************************************************************************/
/*                 STM32L0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l0xx.s).                                               */
/******************************************************************************/

/**
  * @brief  BNRG_SPI_EXTI_IRQHandler This function handles External line
  *         interrupt request for BlueNRG.
  * @param  None
  * @retval None
  */

void BNRG_SPI_EXTI_IRQHandler(void)
{
//  HAL_GPIO_EXTI_IRQHandler(BNRG_SPI_EXTI_PIN);
  if( (EXTI->PR & BNRG_SPI_EXTI_PIN) != RESET) {
    EXTI->PR = BNRG_SPI_EXTI_PIN;
    BNRG_RST_Callback(BNRG_SPI_EXTI_PIN);
  }
  NVIC_ClearPendingIRQ(BNRG_SPI_EXTI_IRQn);
}


/**
* @brief This function handles USART1 global interrupt.
*/

void I2C1_IRQHandler(void) {
	epprom_IRQHandler();
}

/* USART1 interrupt handler */
void USART1_IRQHandler(void) {
	NVIC_ClearPendingIRQ(USART1_IRQn);
	/* Data is ready */
	if (OW_USART->ISR & USART_ISR_RXNE) {
		/* Get data */
		ow_buf[rxCount] = USART1->RDR & 0xFF;
		rxCount++;
	}
	if (OW_USART->ISR & USART_ISR_TXE) {
		/* Get data */
//		USART1->TDR = ow_buf[txCount];
		txCount++;
	}

// Если применяем HAL-библиотеку:
//	HAL_UART_IRQHandler(&huart1);
}


/**
  * @brief  EXTI4_15_IRQHandler This function handles External lines 4 to 15 interrupt request.
  * @param  None
  * @retval None
  */
void PUSH_BUTTON_EXTI_IRQHandler(void)
{
//  HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_13);

  button_event = 1;
}

/******************************************************************************/
/*                 STM32L0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l0xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*
void PPP_IRQHandler(void)
{
}
*/

/**
 * @brief  EXTI line detection callback.
 * @param  uint16_t GPIO_Pin Specifies the pins connected EXTI line
 * @retval None
 */
void BNRG_RST_Callback(uint16_t GPIO_Pin)
{
	UNUSED(GPIO_Pin);
  HCI_Isr();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
