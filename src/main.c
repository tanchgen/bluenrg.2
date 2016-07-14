/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32f0xx.h"
#include "stm32f0xx_conf.h"
//#include "stm32_bluenrg_ble.h"
#include "my_main.h"
//#include "cube_hal.h"

//#include "sample_service.h"
#include "role_type.h"
#include "stm32xx_it.h"
#include "onewire.h"
#include "init.h"
//#include "bt01.h"
//#include "my_bt01_def.h"

//#include "logger.h"


/* Private defines -----------------------------------------------------------*/
#define BDADDR_SIZE 6

// Private variables ---------------------------------------------------------


// SPI handler declaration 
#if BLUENRG
// Uncomment the line corresponding to the role you want to have 
BLE_RoleTypeDef BLE_Role = SERVER;

extern volatile uint8_t set_connectable;
extern volatile uint16_t connection_handle;
extern volatile uint32_t resetCount;

//uint8_t data[20]; //data for send to characteristic

/* Private function prototypes -----------------------------------------------*/
void User_Process(void);
#endif

static void SetSysClock(void);



//void iwdgInit( void );

/* Private functions ---------------------------------------------------------*/

// Определение моего watchdog 
uint16_t myWD;

/**
 * @brief  Main function to show how to use the BlueNRG based device 
 * @param  None
 * @retval None
 */
int main(void)
{ 
  // Configure the system clock
  SetSysClock();
//  owInit();
  // Установки логирования
// 	logInit();

  uint8_t sendBuf[10] = { 0xcc, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  uint8_t buf[16*8];

 // Тестирование 1-Wire
  OW_Init();

#if BLUENRG
  // Initialize the BlueNRG SPI driver
  BNRG_SPI_Init();

  // Initialize the BlueNRG HCI
  HCI_Init();

  BlueNRG_Init();
#endif  // BLUENRG

#if WATCHDOG
  iwdgInit();
#endif // WATCHDOG
  
  while(1)
  {
    for ( uint8_t i=0; i<16*8; i += 9){
    	sendBuf[1] = 0x44;
    	OW_Send(OW_SEND_RESET, (uint8_t *)sendBuf, 2, NULL, 0, OW_NO_READ);
    	myDelay(1000);

    	sendBuf[1] = 0xbe;
    	OW_Send(OW_SEND_RESET, (uint8_t *)sendBuf, 10, &buf[i], 8, 2);
    }

    myDelay(1000);

#if BLUENRG
    HCI_Process();
    User_Process();
#endif
#if WATCHDOG
    IWDG->KR = IWDG_REFRESH;
#endif // WATCHDOG

  }
}

#if BLUENRG
void User_Process(void)
{
  // Проверяем на програмный сброс
  if ( resetCount == 1 ) {
    aci_gap_terminate(connection_handle, SHA_TIMEOUT_REASON);
//    GAP_DisconnectionComplete_CB();
    HAL_Delay(2000);
    NVIC_SystemReset();
  }

  if(set_connectable){
    /* Establish connection with remote device */
    Make_Connection();
    set_connectable = FALSE;
  }
}
#endif

/**
  * @brief  Configures the System clock frequency, AHB/APBx prescalers and Flash
  *         settings.
  * @note   This function should be called only once the RCC clock configuration
  *         is reset to the default reset state (done in SystemInit() function).
  * @param  None
  * @retval None
  */
static void SetSysClock(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;
  RCC_ClocksTypeDef RCC_Clocks;
/******************************************************************************/
/*            PLL (clocked by HSE) used as System clock source                */
/******************************************************************************/

  /* SYSCLK, HCLK, PCLK configuration ----------------------------------------*/
  /* Enable HSE */
  RCC->CR |= ((uint32_t)RCC_CR_HSEON);

  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

  if ((RCC->CR & RCC_CR_HSERDY) != RESET)
  {
    HSEStatus = (uint32_t)0x01;
  }
  else
  {
    HSEStatus = (uint32_t)0x00;
  }

  if (HSEStatus == (uint32_t)0x01)
  {
    /* Enable Prefetch Buffer and set Flash Latency */
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

    /* Select PLL as system clock source */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));

    /* HCLK = SYSCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

    /* PCLK = HCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE_DIV1;

    /* PLL configuration */
    RCC->CFGR2 |= RCC_CFGR2_PREDIV1_DIV4;

    /* Enable PLL */
    RCC->CR &= ~RCC_CR_PLLON;

    /* Wait till PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 1)
    {
    }
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSE_PREDIV | RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLMULL3);

    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
    }

    RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL)
    {
    }
  }
  else
  { /* If HSE fails to start-up, the application will have wrong clock
         configuration. User can add here some code to deal with this error */
  	HardFault_Handler();
  }
  RCC_GetClocksFreq(&RCC_Clocks);

  SysTick_Config(RCC_Clocks.HCLK_Frequency/1000);
  NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY);
  NVIC_EnableIRQ(SysTick_IRQn);
  NVIC_SetPriority(SysTick_IRQn, 0xF);
//  NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY);
#define SYSTICK_CLKSOURCE_HCLK         ((uint32_t)0x00000004)
  SysTick->CTRL |= SYSTICK_CLKSOURCE_HCLK;

}

void Error_Handler( void ) {
	while(1);
}
