/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stm32xx_it.h"
#include "debug.h"
#include "my_service.h"
#include "my_stm32l0xx_nucleo.h" //
#include "stm32l0xx_nucleo_bluenrg.h" //

/** @addtogroup X-CUBE-BLE1_Applications
 *  @{
 */

/** @addtogroup SampleApp
 *  @{
 */

/** @defgroup INTERRUPT_HANDLER
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t ms_counter = 0;
volatile uint8_t button_event = 0;
extern volatile uint32_t relayCount1, relayCount2;

extern volatile uint32_t disconnCount; // Таймаут аутентификации по SHA1-хэш 120 сек.
extern volatile uint32_t resetCount;
extern volatile uint16_t connection_handle;
/* SPI handler declared in "main.c" file */
extern SPI_HandleTypeDef SpiHandle;
/* Private function prototypes -----------------------------------------------*/
void myTimeOut( void );
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
  HAL_IncTick();

  ms_counter++;
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
  HAL_GPIO_EXTI_IRQHandler(BNRG_SPI_EXTI_PIN);
  NVIC_ClearPendingIRQ(BNRG_SPI_EXTI_IRQn);
}


/**
* @brief This function handles USART1 global interrupt.
*/

extern UART_HandleTypeDef huart1;

void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  // HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief  EXTI4_15_IRQHandler This function handles External lines 4 to 15 interrupt request.
  * @param  None
  * @retval None
  */
void PUSH_BUTTON_EXTI_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(KEY_BUTTON_PIN);

  button_event = 1;
}

/* ====================== Здесь мои прерывания от таймеров ================== */
void TIM3_IRQHandler(void)
{
  if ((TIM3->SR & TIM_IT_CC1) && (TIM3->DIER & TIM_IT_CC1))
  {
    TIM3->SR &= ~TIM_IT_CC1;
    TIM3->CCR1 = ( TIM3->CNT + CCR1_VAL );
    myTimeOut();
    /* Запрещаем прерывание */
//    TIM3->DIER &= ~TIM_DIER_CC1IE;
  }
  else if ((TIM3->SR & TIM_IT_CC3) && (TIM3->DIER & TIM_IT_CC3))
  {
    TIM3->SR &= ~TIM_IT_CC3;
    
    // Если реле1 - стартануло...
    if ( RELAY1_PORT->IDR & RELAY1_PIN) { 
      // Для индикации: включаем светодиод, если реле включилось
      HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_SET);

      TIM3->CCR3 = ( TIM3->CNT + CCR3_VAL ); // Запускаем счетчик на 1с
    }
    else { // Если выключено - выставляем время на CC3_VAL больше, чем в счетчике
      // Для индикации: выключаем светодиод, если реле выключилось
      HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_RESET);

      TIM3->DIER &= ~TIM_DIER_CC3IE; // Запрещаем прерывание
      TIM3->CCMR2 &= ~TIM_CCMR2_OC3M; 
      TIM3->CCMR2 |= TIM_OC3MODE_FORCED_INACTIVE; // Выставляем выход OС3 в 0
    }  
  }
  else if ((TIM3->SR & TIM_IT_CC4) && (TIM3->DIER & TIM_IT_CC4))
  {
    TIM3->SR &= ~TIM_IT_CC4;
    
    // Если реле1 - стартануло...
    if ( RELAY2_PORT->IDR & RELAY2_PIN) { 
      // Для индикации: включаем светодиод, если реле включилось
      HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_SET);

      TIM3->CCR4 = ( TIM3->CNT + CCR4_VAL ); // Запускаем счетчик на 1с
    }
    else { // Если выключено - выставляем время на CC3_VAL больше, чем в счетчике
      // Для индикации: выключаем светодиод, если реле выключилось
      HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_RESET);

      TIM3->DIER &= ~TIM_DIER_CC4IE; // Запрещаем прерывание
      TIM3->CCMR2 &= ~TIM_CCMR2_OC4M; 
      TIM3->CCMR2 |= TIM_OC4MODE_FORCED_INACTIVE; // Выставляем выход OС3 в 0
    }  
  }
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
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
