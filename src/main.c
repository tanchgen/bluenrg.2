/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32f0xx.h"
#include "stm32_bluenrg_ble.h"
//#include "hci.h"
#include "my_service.h"
#include "my_time.h"
#include "stm32xx_it.h"
#include "onewire.h"
#include "init.h"
#include "logger.h"
#include "my_main.h"

//#include "bt01.h"
//#include "my_bt01_def.h"

#define WATCHDOG	0

uint8_t crc;

uint8_t crcDS(uint8_t inp, uint8_t crc);

/* Private defines -----------------------------------------------------------*/
#define BDADDR_SIZE 6

// Private variables ---------------------------------------------------------


// SPI handler declaration 
#if BLUENRG

//uint8_t data[20]; //data for send to characteristic

/* Private function prototypes -----------------------------------------------*/
void User_Process(void);
#endif

static void SetSysClock(void);

#if WATCHDOG
void iwdgInit( void );
#endif

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
	myTick = 0;
  // Configure the system clock
  SetSysClock();
  timeInit();
  alrmInit();

#if ONE_WIRE
  toInit();
#else
  toLogCount = 0;
  toReadCount = 0;
#endif  // ONE_WIRE
  ddInit();
  // Установки логирования
  logInit();

#if BLUENRG
  // Initialize the BlueNRG SPI driver
  BNRG_SPI_Init();

  // Initialize the BlueNRG HCI
  HCI_Init();

  if(BlueNRG_Init() == BLE_STATUS_TIMEOUT){
 		blue.bleStatus = BLE_STATUS_TIMEOUT;
  }
#endif  // BLUENRG

#if WATCHDOG
  iwdgInit();
#endif // WATCHDOG

  while(1)
  {

#if BLUENRG
    HCI_Process();
  	timersProcess();
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
	if(blue.bleStatus == BLE_STATUS_TIMEOUT){
		bnrgFullRst();
	}
  if(blue.connectable){
    /* Establish connection with remote device */
    Make_Connection();
    blue.connectable = FALSE;
  }

  if ( blue.connected ) {
  		logSend( blue.logStatus.toReq, blue.logStatus.ddReq );
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
    
    // Select SYSCLK -> I2C clock source
    RCC->CFGR3 &= ~RCC_CFGR3_I2C1SW;

    /* PLL configuration */
    RCC->CFGR2 |= RCC_CFGR2_PREDIV_DIV1;

    /* Enable PLL */
    RCC->CR &= ~RCC_CR_PLLON;

    /* Wait till PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 1)
    {
    }
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL));
    RCC->CFGR |= (uint32_t)RCC_CFGR_PLLSRC_HSE_PREDIV;
//    RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE_PREDIV_DIV1;
    RCC->CFGR |= RCC_CFGR_PLLMUL3;

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
    FLASH->ACR |= FLASH_ACR_LATENCY;

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
  
#define SYSTICK_CLKSOURCE_HCLK         ((uint32_t)0x00000004)
//  SysTick->CTRL |= SYSTICK_CLKSOURCE_HCLK;

}

void Error_Handler( eErrStatus err ){

	switch ( err ){
		case OW_TO_DEV_ERR:
			alrmUpdate( ALARM_TO_FAULT );
			break;
		case OW_DD_DEV_ERR:
			alrmUpdate( ALARM_DD_FAULT );
			break;
		case OW_WIRE_ERR:
			for ( uint8_t i = 0; i < TO_DEV_NUM; i++ ) {
				if( owToDev[i].devStatus == OW_DEV_OK ){
					owToDev[i].devStatus = OW_TO_DEV_ERR;
					owToDev[i].newErr = TRUE;
				}
			}
#if OW_DD
			for ( uint8_t i = 0; i < OW_DD_DEV_NUM; i++ ) {
				if( owDdDev[i].devStatus == OW_DEV_OK ){
					owDdDev[i].devStatus = OW_DD_DEV_ERR;
					owDdDev[i].newErr = TRUE;
				}
			}
#endif
			alrmUpdate( ALARM_TO_FAULT );
			break;
		case 	LOG_ERR:
			alrmUpdate( ALARM_LOG_FAULT );
			break;
		default:
			while(1);
	}
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

#if WATCHDOG
void iwdgInit( void ){
  IWDG->KR = IWDG_START;          // Запуск WatchDog
  IWDG->KR = IWDG_WRITE_ACCESS;   // Доступ записи в регистры
  IWDG->PR = IWDG_PR_PR_2;        // Прескалер = 64;
  IWDG->RLR = 2047;               // Записываем регистр перезагрузки
  for ( uint32_t tickstart = myTick + HW_TIMEOUT; !(IWDG->SR ) ; ) {
    if ( myTick >
					tickstart ) {
      // Проблема с IWDG
      HardFault_Handler();
    }
  }
  IWDG->KR = IWDG_REFRESH;
}
#endif
