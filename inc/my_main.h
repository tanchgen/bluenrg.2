/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "my_bt01_def.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/** @addtogroup MAIN_Exported_Defines
 *  @{
 */
/* Exported defines --------------------------------------------------------*/
/** 
 * @brief User can use this section to tailor USARTx/UARTx instance used and 
 *        associated resources.
 */

#define  TICK_INT_PRIORITY            ((uint32_t)0)    /*!< tick interrupt priority (lowest by default)  */

/* Definition for USARTx clock resources */
#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __USART2_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE() 

#define USARTx_FORCE_RESET()             __USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __USART2_RELEASE_RESET()

/* Definition for USARTx Pins */
#ifdef USE_STM32F4XX_NUCLEO
  #define USARTx_TX_PIN                  GPIO_PIN_2
  #define USARTx_TX_GPIO_PORT            GPIOA  
  #define USARTx_TX_AF                   GPIO_AF7_USART2
  #define USARTx_RX_PIN                  GPIO_PIN_3
  #define USARTx_RX_GPIO_PORT            GPIOA 
  #define USARTx_RX_AF                   GPIO_AF7_USART2
#endif /* USE_STM32F4XX_NUCLEO */

#ifdef USE_STM32L0XX_NUCLEO  
  #define USARTx_TX_PIN                  GPIO_PIN_2
  #define USARTx_TX_GPIO_PORT            GPIOA  
  #define USARTx_TX_AF                   GPIO_AF4_USART2
  #define USARTx_RX_PIN                  GPIO_PIN_3
  #define USARTx_RX_GPIO_PORT            GPIOA 
  #define USARTx_RX_AF                   GPIO_AF4_USART2
#endif /* USE_STM32L0XX_NUCLEO */

/* Definition for USARTx's NVIC */
#define USARTx_IRQn                      USART2_IRQn
#define USARTx_IRQHandler                USART2_IRQHandler

/* Size of Reception buffer */
#define UARTHEADERSIZE 4
#define RXBUFFERSIZE 255
/**
 * @}
 */
 
/** @addtogroup MAIN_Exported_Macros
 *  @{
 */ 
/* Exported macros -----------------------------------------------------------*/
/* Size of Transmission buffer */
#define TXSTARTMESSAGESIZE               (COUNTOF(aTxStartMessage) - 1)
#define TXENDMESSAGESIZE                 (COUNTOF(aTxEndMessage) - 1)

#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

#define PLL_TIMEOUT		PLL_TIMEOUT_VALUE
#define HSE_TIMEOUT             HSE_TIMEOUT_VALUE

typedef enum {
	FALSE = 0,
	TRUE
} tBool;

/**
 * @}
 */
 
extern uint32_t myTick;

/* Exported functions ------------------------------------------------------- */

void getTokenStr( uint8_t ch[] );
void getShaHash( uint8_t *ch, uint8_t shaHash[] );

void Error_Handler( void );

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
