/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stm32xx_it.h"
//#include "my_service.h"
#include "my_stm32l0xx_nucleo.h" //
//#include "stm32l0xx_nucleo_bluenrg.h" //
//#include "hci.h"


uint32_t myTick = 0;

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
/*
void BNRG_SPI_EXTI_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(BNRG_SPI_EXTI_PIN);
  NVIC_ClearPendingIRQ(BNRG_SPI_EXTI_IRQn);
}
*/

/**
* @brief This function handles USART1 global interrupt.
*/


void USART1_IRQHandler(void)
{
  // HAL_UART_IRQHandler(&huart1);
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

/* ====================== Здесь мои прерывания от таймеров ================== */
void TIM3_IRQHandler(void)
{
}

void TIM16_IRQHandler(void)
{
}

void TIM17_IRQHandler(void)
{
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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  HCI_Isr();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/