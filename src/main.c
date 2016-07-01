/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "stm32_bluenrg_ble.h"
#include "my_main.h"
//#include "cube_hal.h"

#include "sample_service.h"
//#include "role_type.h"
#include "debug.h"
#include "stm32xx_it.h"
#include "bt01.h"
#include "my_bt01_def.h"

//#include "logger.h"

#include <stdio.h>

/* Private defines -----------------------------------------------------------*/
#define BDADDR_SIZE 6

// Private variables ---------------------------------------------------------
// UART handler declaration
UART_HandleTypeDef UartHandle;

// Buffer used for transmission 
// Buffer used for reception 
uint8_t uart_header[UARTHEADERSIZE];
uint8_t aRxBuffer[RXBUFFERSIZE];

// SPI handler declaration 

// Uncomment the line corresponding to the role you want to have 
BLE_RoleTypeDef BLE_Role = SERVER;
TIM_Base_InitTypeDef  TIM_TimeBaseStructure;
TIM_OC_InitTypeDef  TIM_OCInitStructure;

extern volatile uint8_t set_connectable;
extern volatile uint16_t connection_handle;
extern volatile uint32_t resetCount;

//uint8_t data[20]; //data for send to characteristic

/* Private function prototypes -----------------------------------------------*/
void User_Process(void);
void tim3Init( void );
void iwdgInit( void );

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
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
  
  tim3Init();

  /* Configure RELAY1 (GPIO1) PB1*/
  relayInit(RELAY1);
   /* Configure RELAY2 (GPIO1) PB2*/
  relayInit(RELAY2);
 
  /* Инициация Светодиодов (PA2 и PA3) */
  ledInit(LED1);
  ledInit(LED2); 
  /* Initialize the BlueNRG SPI driver */
  BNRG_SPI_Init();

  /* Initialize the BlueNRG HCI */
  HCI_Init();

  BlueNRG_Init();
  
  iwdgInit();
  
  while(1)
  {
    HCI_Process();
    User_Process();
    IWDG->KR = IWDG_REFRESH;
  }
}

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


/** System Clock Configuration
*/
void SystemClock_Config(void)
{
/*
	uint8_t err = 0;
  uint32_t temp;
 
  // Запускаем внешний генератор
  RCC->CR |= RCC_CR_HSEON;
  for ( uint32_t tickstart = HAL_GetTick(); !(RCC->CR & RCC_CR_HSERDY); ) {
    if ( (HAL_GetTick() - tickstart) > HSE_TIMEOUT) {
      // Внешний кварц не запустился
      err = TRUE;
    }
  }
  // Частота <= 24МГц. Поэтому включаем буфер предвыборки флеш и LATENCY=0
  FLASH->ACR &= ~FLASH_ACR_LATENCY; // Предочистка.
  FLASH->ACR |= FLASH_LATENCY_0; // Если SystemCoreClock <= 24 МГц, без пропусков.
  //FLASH->ACR |= FLASH_LATENCY_1; // Если 24< SystemCoreClock <= 48, пропускать 1 такт.
  // FLASH->ACR |= FLASH_LATENCY_2; // Если 48< SystemCoreClock <= 72, пропускать 2 такта.

  FLASH->ACR |= FLASH_ACR_PRFTBE; // Enable Prefetch Buffer.
  
  // Запускаем-настраиваем PLL
  temp = RCC->CFGR;
  temp &= ~RCC_CFGR_SW_PLL;
  temp |= RCC_CFGR_SW_HSI;
  RCC->CFGR = temp;
  RCC->CR &= ~RCC_CR_PLLON;
  for ( uint32_t tickstart = HAL_GetTick(); RCC->CR & RCC_CR_PLLRDY ; ) {
    if ( (HAL_GetTick() - tickstart) > PLL_TIMEOUT) {
      // Внешний кварц не запустился
      err = TRUE;
      HardFault_Handler();
    }
  }

  RCC->CFGR &= ~RCC_CFGR_PLLMUL;
  RCC->CFGR &= ~RCC_CFGR_PLLSRC;
  if ( !err ) {         //HSE запустился и настраиваем под него
    // Настраиваем PLL 
    RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE_PREDIV_DIV2;
    RCC->CFGR |= RCC_CFGR_PLLMUL3;      // Внешний кварц 16МГц
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV;
  }
  else {
    // Настраиваем PLL
    RCC->CFGR |= RCC_CFGR_PLLMUL6;
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_DIV2;
  }
 
  // Запускаем-настраиваем PLL
  RCC->CR |= RCC_CR_PLLON;
  for ( uint32_t tickstart = HAL_GetTick(); !(RCC->CR & RCC_CR_PLLRDY) ; ) {
    if ( (HAL_GetTick() - tickstart) > PLL_TIMEOUT) {
      // PLL Не запустился
      err = TRUE;
      HardFault_Handler();
    }
  }

  RCC->CFGR &= ~RCC_CFGR_SW;
  if ( !err ) {  
    RCC->CFGR |= RCC_CFGR_SW_PLL;
  }
  else {
    RCC->CFGR |= RCC_CFGR_SW_HSI;
  }
  for ( uint32_t tickstart = HAL_GetTick(); !(RCC->CFGR & RCC_CFGR_SWS_PLL) ; ) {
    if ( (HAL_GetTick() - tickstart) > PLL_TIMEOUT) {
      // PLL Не запустился
      err = TRUE;
      HardFault_Handler();
    }
  }
*/
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  SysTick_Config(HAL_RCC_GetHCLKFreq()/1000);
  NVIC_SetPriority(SysTick_IRQn, 4);
//  NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY);
  SysTick->CTRL |= SYSTICK_CLKSOURCE_HCLK;
}

/* ========== Инициализация таймера ===========
* Канал 1 - прерывание по 1с (для disConn, myWD, ban0..ban9)
* Канал 3 - управление Реле1
* Канал 4 - управление реле2
*/

void tim3Init(void)
{
  //Разрешение прерывания от таймера 3 и установка приоритета
  NVIC_SetPriority(TIM3_IRQn, 2);
  NVIC_EnableIRQ(TIM3_IRQn);
  
  /*Инициализация таймера TIM3.
  Для измерения периода входного сигнала используется канал 1 (TIM3_CH1)*/
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;     //Включаем тактирование TIM3
  TIM3->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS);
  TIM3->ARR = 0xFFFF;
  TIM3->PSC = 47999;      // Выставляем счетчик на 2мс

  /* Разрешаем работу */
  TIM3->CCER |= TIM_CCER_CC1E;
  TIM3->CCER |= TIM_CCER_CC3E;
  TIM3->CCER |= TIM_CCER_CC4E;

  TIM3->CCER &= ~TIM_CCER_CC3P;
  TIM3->CCER &= ~TIM_CCER_CC4P;
  
  /* Отключаем выход сигнала OCxREF */
  TIM3->CCMR1 &= ~TIM_CCMR1_OC1M;
  TIM3->CCMR2 &= ~TIM_CCMR2_OC3M;
  TIM3->CCMR2 &= ~TIM_CCMR2_OC4M;
  
  TIM3->CCMR2 |= TIM_OC3MODE_FORCED_INACTIVE;
  TIM3->CCMR2 |= TIM_OC4MODE_FORCED_INACTIVE;
  
  TIM3->CCMR2 &= ~TIM_CCMR2_OC3PE;
  TIM3->CCMR2 &= ~TIM_CCMR2_OC4PE;
  
  /* Выставляем на "CC in output" */
  TIM3->CCMR1 &= ~TIM_CCMR1_CC1S;
  TIM3->CCMR2 &= ~TIM_CCMR2_CC3S;
  TIM3->CCMR2 &= ~TIM_CCMR2_CC4S;

  /* Загружаем предустановку */
  TIM3->CCR1 = CCR1_VAL;
  TIM3->CCR3 = CCR3_VAL;
  TIM3->CCR4 = CCR4_VAL;
  
  /* Запрещаем прерывание */
  TIM3->DIER &= ~TIM_DIER_CC1IE;
  TIM3->DIER &= ~TIM_DIER_CC3IE;
  TIM3->DIER &= ~TIM_DIER_CC4IE;

  TIM3->DIER |= TIM_DIER_CC1IE;
  TIM3->CR1 |= TIM_CR1_CEN;
}
