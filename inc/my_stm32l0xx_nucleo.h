/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MY_STM32L0XX_NUCLEO_H
#define __MY_STM32L0XX_NUCLEO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
   
#define  GPIO_SPEED_LOW         ((uint32_t)0x00000000)  /*!< Low speed     */
#define  GPIO_SPEED_MEDIUM      ((uint32_t)0x00000001)  /*!< Medium speed  */
#define  GPIO_SPEED_FAST        ((uint32_t)0x00000002)  /*!< Fast speed    */
#define  GPIO_SPEED_HIGH        ((uint32_t)0x00000003)  /*!< High speed    */

typedef enum 
{  
  BUTTON_KEY = 0
} Button_TypeDef;

typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef; 

typedef enum 
{ 
  JOY_NONE  = 0,
  JOY_SEL   = 1,
  JOY_DOWN  = 2,
  JOY_LEFT  = 3,
  JOY_RIGHT = 4,
  JOY_UP    = 5
} JOYState_TypeDef;

/**
  * @}
  */ 

/** @defgroup STM32L0XX_NUCLEO_LOW_LEVEL_Exported_Constants 
  * @{
  */ 

/** 
* @brief	Define for STM32L0XX_NUCLEO board  
  */ 
#if !defined (USE_STM32L0XX_NUCLEO)
 #define USE_STM32L0XX_NUCLEO
#endif

#define BUTTONn                            1

/**
  * @brief Key push-button
  */
#define KEY_BUTTON_PIN                         GPIO_PIN_13
#define KEY_BUTTON_GPIO_PORT                   GPIOC
#define KEY_BUTTON_GPIO_CLK_ENABLE()         __GPIOC_CLK_ENABLE()   
#define KEY_BUTTON_GPIO_CLK_DISABLE()        __GPIOC_CLK_DISABLE()  
#define KEY_BUTTON_EXTI_LINE                   GPIO_PIN_13
#define KEY_BUTTON_EXTI_IRQn                   EXTI4_15_IRQn

#define BUTTONx_GPIO_CLK_ENABLE(__INDEX__)    (((__INDEX__) == 0) ? KEY_BUTTON_GPIO_CLK_ENABLE() : 0)
#define BUTTONx_GPIO_CLK_DISABLE(__INDEX__)   (((__INDEX__) == 0) ? KEY_BUTTON_GPIO_CLK_DISABLE() : 0)
/**
  * @}
  */ 

/** @addtogroup STM32L0XX_NUCLEO_LOW_LEVEL_BUS
  * @{
  */ 
/*###################### SPI1 ###################################*/
#define NUCLEO_SPIx                                 SPI1
#define NUCLEO_SPIx_CLK_ENABLE()                  __SPI1_CLK_ENABLE()

#define NUCLEO_SPIx_SCK_AF                          GPIO_AF0_SPI1
#define NUCLEO_SPIx_SCK_GPIO_PORT                   GPIOA
#define NUCLEO_SPIx_SCK_PIN                         GPIO_PIN_5
#define NUCLEO_SPIx_SCK_GPIO_CLK_ENABLE()         __GPIOA_CLK_ENABLE()
#define NUCLEO_SPIx_SCK_GPIO_CLK_DISABLE()        __GPIOA_CLK_DISABLE()

#define NUCLEO_SPIx_MISO_MOSI_AF                    GPIO_AF0_SPI1
#define NUCLEO_SPIx_MISO_MOSI_GPIO_PORT             GPIOA
#define NUCLEO_SPIx_MISO_MOSI_GPIO_CLK_ENABLE()   __GPIOA_CLK_ENABLE()
#define NUCLEO_SPIx_MISO_MOSI_GPIO_CLK_DISABLE()  __GPIOA_CLK_DISABLE()
#define NUCLEO_SPIx_MISO_PIN                        GPIO_PIN_6
#define NUCLEO_SPIx_MOSI_PIN                        GPIO_PIN_7
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define NUCLEO_SPIx_TIMEOUT_MAX                   1000

/** @addtogroup STM32L0XX_NUCLEO_LOW_LEVEL_COMPONENT
  * @{
  */

/**
  * @brief  SD Control Lines management
  */
#define SD_CS_LOW()       HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_PIN_RESET)
#define SD_CS_HIGH()      HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_PIN_SET)

/**
  * @brief  LCD Control Lines management
  */
#define LCD_CS_LOW()      HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET)
#define LCD_CS_HIGH()     HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET)
#define LCD_DC_LOW()      HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_RESET)
#define LCD_DC_HIGH()     HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET)
     
/**
  * @brief  SD Control Interface pins
  */
#define SD_CS_PIN                                 GPIO_PIN_5
#define SD_CS_GPIO_PORT                           GPIOB
#define SD_CS_GPIO_CLK_ENABLE()                 __GPIOB_CLK_ENABLE()
#define SD_CS_GPIO_CLK_DISABLE()                __GPIOB_CLK_DISABLE()

/**
  * @brief  LCD Control Interface pins
  */
#define LCD_CS_PIN                                 GPIO_PIN_6
#define LCD_CS_GPIO_PORT                           GPIOB
#define LCD_CS_GPIO_CLK_ENABLE()                 __GPIOB_CLK_ENABLE()
#define LCD_CS_GPIO_CLK_DISABLE()                __GPIOB_CLK_DISABLE()
    
/**
  * @brief  LCD Data/Command Interface pins
  */
#define LCD_DC_PIN                                 GPIO_PIN_9
#define LCD_DC_GPIO_PORT                           GPIOA
#define LCD_DC_GPIO_CLK_ENABLE()                 __GPIOA_CLK_ENABLE()
#define LCD_DC_GPIO_CLK_DISABLE()                __GPIOA_CLK_DISABLE()

/*##################### ADC1 ###################################*/
/**
  * @brief  ADC Interface pins
  *         used to detect motion of Joystick available on Adafruit 1.8" TFT shield
  */
#define NUCLEO_ADCx                                 ADC1
#define NUCLEO_ADCx_CLK_ENABLE()                  __ADC1_CLK_ENABLE()

#define NUCLEO_ADCx_GPIO_PORT                       GPIOB
#define NUCLEO_ADCx_GPIO_PIN                        GPIO_PIN_0
#define NUCLEO_ADCx_GPIO_CLK_ENABLE()             __GPIOB_CLK_ENABLE()
#define NUCLEO_ADCx_GPIO_CLK_DISABLE()            __GPIOB_CLK_DISABLE()

/** @defgroup STM32L0XX_NUCLEO_LOW_LEVEL_Exported_Macros 
  * @{
  */  
/**
  * @}
  */ 

/** @defgroup STM32L0XX_NUCLEO_LOW_LEVEL_Exported_Functions 
  * @{
  */
uint32_t         BSP_GetVersion(void);  
void             BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode);
uint32_t         BSP_PB_GetState(Button_TypeDef Button);                
uint8_t          BSP_JOY_Init(void);
JOYState_TypeDef BSP_JOY_GetState(void);
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

#ifdef __cplusplus
}
#endif

#endif /* __STM32L0XX_NUCLEO_H */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

