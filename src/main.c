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

uint8_t crc;

uint8_t crcDS(uint8_t inp, uint8_t crc);

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
  owInit();
  // Установки логирования
 	logInit();

  uint8_t sendBuf[11];
  uint8_t readBuf[16*8];
  uint64_t addr;


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

  memset( sendBuf, 0xFF, 11);

	sendBuf[0] = READ_ROM;
	OW_Send(OW_SEND_RESET, sendBuf, 9, readBuf, 8, 1);
	addr = *((uint64_t *)readBuf);

	memcpy( &sendBuf[1], (void*)&addr, 8);
 	sendBuf[0] = MATCH_ROM;
 	OW_Send(OW_SEND_RESET, sendBuf, 9, NULL, 0, OW_NO_READ);
  memset( sendBuf, 0xFF, 9);

  sendBuf[0] = MEM_READ;
 	OW_Send(OW_NO_RESET, sendBuf, 11, readBuf, 8, 2);

  while(1)
  {
  	timersProcess();

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

uint8_t crcDS(uint8_t inp, uint8_t crc) {
 inp ^= crc;
 crc = 0;
 if(inp & 0x1)   crc ^= 0x5e;
 if(inp & 0x2)   crc ^= 0xbc;
 if(inp & 0x4)   crc ^= 0x61;
 if(inp & 0x8)   crc ^= 0xc2;
 if(inp & 0x10)  crc ^= 0x9d;
 if(inp & 0x20)  crc ^= 0x23;
 if(inp & 0x40)  crc ^= 0x46;
 if(inp & 0x80)  crc ^= 0x8c;
 return (crc);
}
