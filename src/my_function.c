#include "stm32f0xx.h"
#include "my_main.h"
//#include "bt01.h"
//#include "my_service.h"
#include "deviceid.h"
#include "my_service.h"

#ifndef HAL_TIM_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#endif


//void HardFault_Handler( void ); // Прерывание ошибки железа
//static void WakeupBlueNRG(void);

/*
char * itoa(int n, char * s) {
  char * dest = s;

  if(n / 10 != 0) {
    dest = itoa(n/10, dest);
  }
  else if(n < 0) {
    *dest++ = '-';
    n = -n;
  }

  *dest++ = (n % 10) + '0';
  *dest = '\0';

  return dest;
}
*/
/*
 *  Define the circular shift macro
 */
#define RoL(bits,word) \
                ((((word) << (bits)) & 0xFFFFFFFF) | \
                ((word) >> (32-(bits))))

extern volatile int connected;                  // Флаг действующего соединения
/*
static const uint8_t ch[] = {'0','1','2','3','4','5','6','7','8','9',
                       'A','B','C','D','E','F','G','H','I','J','K','L','M',
                       'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
                       'a','b','c','d','e','f','g','h','i','j','k','l','m',
                       'n','o','p','q','r','s','t','u','v','w','x','y','z' };
static const char Device_UUID[] = { DEVICE_UUID };
*/
uint32_t Pin = 123456;

volatile uint32_t disconnCount; // Таймаут аутентификации по SHA1-хэш 120 сек.
extern volatile uint16_t myWD;

uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  
void myTimeOut( void ){
  // Таймаут 2 мин для токена
  if ( disconnCount ) { 
    if ( (--disconnCount) == 0) {
      aci_gap_terminate( blue.connHandle, SHA_TIMEOUT_REASON);
//      aci_gap_set_non_connectable( ADV_NONCONN_IND );
    }
  }
  if ( myWD ) {
    if ( ((--myWD) == 0) && !connected ){
      NVIC_SystemReset();
    }
  }
  
}  

/*
static void us50Delay(void)
{
#if SYSCLK_FREQ == 8000000
  for(volatile int i = 0; i < 18; i++)__NOP();
#elif SYSCLK_FREQ == 16000000
  for(volatile int i = 0; i < 55; i++)__NOP();
#elif SYSCLK_FREQ == 24000000
  for(volatile int i = 0; i < 91; i++)__NOP();
#elif SYSCLK_FREQ == 32000000
  for(volatile int i = 0; i < 127; i++)__NOP();
#else // SYSCLK_FREQ == 48000000
  for(volatile int i = 0; i < 200; i++)__NOP();
//#error Implement delay function.
#endif    
}

static void WakeupBlueNRG(void)
{
  HAL_NVIC_DisableIRQ(BNRG_SPI_EXTI_IRQn);
  set_irq_as_output();
  us50Delay();
  us50Delay();
  us50Delay();
  LPM_Mode_Request(eLPM_SPI_TX, eLPM_Mode_LP_Stop);
  set_irq_as_input();
  HAL_NVIC_EnableIRQ(BNRG_SPI_EXTI_IRQn);
  return;
}
*/

uint8_t hlToStr(uint32_t l, uint8_t **str){
  int i;
  uint8_t nb = 0;

  if ((i = l/16) != 0)
     nb += hlToStr( i, str );
  if( (i = (l % 16 )) < 10) {
    i += '0';
  }
  else {
    i += 'A'-10;
  }
  *((*str)++) = i;
  *(*str) = '\0';
  nb++;

  return nb;

}
