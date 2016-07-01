#ifndef __BT01_H
#define __BT01_H
/* Здесь определяются значения, присущие плате "BT01" */

#include "stm32f0xx_hal.h"
#include "ble_status.h"
#include "stm32_bluenrg_ble.h"
#include "my_bt01_def.h"
#include "stm32f0xx_hal_tim.h"
   
#define  GPIO_SPEED_LOW         ((uint32_t)0x00000000)  /*!< Low speed     */
#define  GPIO_SPEED_MEDIUM      ((uint32_t)0x00000001)  /*!< Medium speed  */
#define  GPIO_SPEED_FAST        ((uint32_t)0x00000002)  /*!< Fast speed    */
#define  GPIO_SPEED_HIGH        ((uint32_t)0x00000003)  /*!< High speed    */

/*##################### Определения для реле #########################*/
#define RELAYn                             2

#define RELAY1_PIN                         GPIO_PIN_0
#define RELAY1_PORT                        GPIOB
#define RELAY1_GPIO_CLK_ENABLE()           __GPIOB_CLK_ENABLE()  
#define RELAY1_GPIO_CLK_DISABLE()          __GPIOB_CLK_DISABLE()
  
#define RELAY2_PIN                         GPIO_PIN_1
#define RELAY2_PORT                        GPIOB
#define RELAY2_GPIO_CLK_ENABLE()           __GPIOB_CLK_ENABLE()  
#define RELAY2_GPIO_CLK_DISABLE()          __GPIOB_CLK_DISABLE()
  
#define RELAYx_GPIO_CLK_ENABLE(__INDEX__)   if (__INDEX__) RELAY2_GPIO_CLK_ENABLE(); else RELAY1_GPIO_CLK_ENABLE()
#define RELAYx_GPIO_CLK_DISABLE(__INDEX__)  if (__INDEX__) RELAY2_GPIO_CLK_DISABLE(); else RELAY1_GPIO_CLK_DISABLE()

#define RELAY_OFF                          0x00
#define RELAY_ON                           0x01

/*##################### Определения для LEDs #########################*/
#define LEDn                              2

// Индикатор - Установлено ли соединение по Bluetooth (Вкл) или нет (Выкл)
#define LED1_PIN                          GPIO_PIN_2
#define LED1_PORT                         GPIOA
#define LED1_GPIO_CLK_ENABLE()            __GPIOA_CLK_ENABLE()  
#define LED1_GPIO_CLK_DISABLE()           __GPIOA_CLK_DISABLE()

// Индикатор - Включено ли какое-либо реле (Вкл) или нет (Выкл)
#define LED2_PIN                          GPIO_PIN_3	
#define LED2_PORT                         GPIOA
#define LED2_GPIO_CLK_ENABLE()            __GPIOA_CLK_ENABLE()  
#define LED2_GPIO_CLK_DISABLE()           __GPIOA_CLK_DISABLE()
  
#define LEDx_GPIO_CLK_ENABLE(__INDEX__)   if (__INDEX__) LED2_GPIO_CLK_ENABLE(); else LED1_GPIO_CLK_ENABLE()
#define LEDx_GPIO_CLK_DISABLE(__INDEX__)  if (__INDEX__) LED2_GPIO_CLK_DISABLE(); else LED1_GPIO_CLK_DISABLE()

#define LED_OFF                          0x00
#define LED_ON                           0x01



/*###################### SPI1 ###################################*/
#define SPIx                                 SPI1
#define SPIx_CLK_ENABLE()                  __SPI1_CLK_ENABLE()

#define SPIx_SCK_AF                          GPIO_AF0_SPI1
#define SPIx_SCK_GPIO_PORT                   GPIOA
#define SPIx_SCK_PIN                         GPIO_PIN_5
#define SPIx_SCK_GPIO_CLK_ENABLE()         __GPIOA_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_DISABLE()        __GPIOA_CLK_DISABLE()

#define SPIx_MISO_MOSI_AF                    GPIO_AF0_SPI1
#define SPIx_MISO_MOSI_GPIO_PORT             GPIOA
#define SPIx_MISO_MOSI_GPIO_CLK_ENABLE()   __GPIOA_CLK_ENABLE()
#define SPIx_MISO_MOSI_GPIO_CLK_DISABLE()  __GPIOA_CLK_DISABLE()
#define SPIx_MISO_PIN                        GPIO_PIN_6
#define SPIx_MOSI_PIN                        GPIO_PIN_7
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define SPIx_TIMEOUT_MAX                   1000

uint32_t    getVersion(void);  
void        timInit( TIM_HandleTypeDef *timHandle, TIM_TypeDef *rel ); 
void        relayInit( relayTypeDef rel );
uint8_t     relayStatus( relayTypeDef rel );
void        relayOn( relayTypeDef rel );
void        relayOff( relayTypeDef rel );
void        relayToggle( relayTypeDef rel );  

void        ledInit( ledTypeDef led );
uint8_t     ledStatus( ledTypeDef led );
void        ledOn( ledTypeDef led );
void        ledOff( ledTypeDef led );
void        ledToggle( ledTypeDef led );  

tBleStatus blueReset( uint16_t connection_handle, uint8_t reson );
tBleStatus BlueNRG_Init( void );

#endif /* __BT01_H */
