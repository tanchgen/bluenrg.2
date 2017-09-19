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

#define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
#define FLASH_USER_START_ADDR   ((uint32_t)0x08002000)   /* Start @ of user Flash area */
#define DATA_TO_PROG            ((uint32_t)0xAA55CC33)   /* 32-bits value to be programmed */

/* Error codes used to make the orange led blinking */
#define ERROR_ERASE 0x01
#define ERROR_PROG  0x02
#define ERROR_PROG_FLAG 0x04
#define ERROR_WRITE_PROTECTION 0x08
#define ERROR_UNKNOWN 0x10

uint8_t error;

/**
  * @brief  This function programs a 16-bit word.
  *         The Programming bit (PG) is set at the beginning and reset at the end
  *         of the function, in case of successive programming, these two operations
  *         could be performed outside the function.
  *         This function waits the end of programming, clears the appropriate bit in
  *         the Status register and eventually reports an error.
  * @param  flash_addr is the address to be programmed
  *         data is the 16-bit word to program
  * @retval None
  */
__INLINE void FlashWord16Write(uint32_t flash_addr, uint16_t data) {
  /* (1) Set the PG bit in the FLASH_CR register to enable programming */
  /* (2) Perform the data write (half-word) at the desired address */
  /* (3) Wait until the BSY bit is reset in the FLASH_SR register */
  /* (4) Check the EOP flag in the FLASH_SR register */
  /* (5) clear it by software by writing it at 1 */
  /* (6) Reset the PG Bit to disable programming */
  FLASH->CR |= FLASH_CR_PG; /* (1) */
  *(__IO uint16_t*)(flash_addr) = data; /* (2) */
  while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (3) */
  {
    /* For robust implementation, add here time-out management */
  }
  if ((FLASH->SR & FLASH_SR_EOP) != 0)  /* (4) */
  {
    FLASH->SR |= FLASH_SR_EOP; /* (5) */
  }
  /* Manage the error cases */
  else if ((FLASH->SR & FLASH_SR_PGERR) != 0) /* Check Programming error */
  {
    error = ERROR_PROG_FLAG;
    FLASH->SR |= FLASH_SR_PGERR; /* Clear it by software by writing EOP at 1*/
  }
  else if ((FLASH->SR & FLASH_SR_WRPERR) != 0) /* Check write protection */
  {
    error = ERROR_WRITE_PROTECTION;
    FLASH->SR |= FLASH_SR_WRPERR; /* Clear it by software by writing it at 1*/
  }
  else
  {
    error = ERROR_UNKNOWN;
  }
  FLASH->CR &= ~FLASH_CR_PG; /* (6) */
}

void flashBdaddrSave( uint8_t * bdaddr ){
  // Начальный адрес сохранения
  uint32_t faddr = BDADDR_FLASH_START;

  // Unlock FLASH
  while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
  {
    /* For robust implementation, add here time-out management */
  }
  if ((FLASH->CR & FLASH_CR_LOCK) != 0) /* (2) */
  {
    FLASH->KEYR = 0x45670123; /* (3) */
    FLASH->KEYR = 0xCDEF89AB;
  }

  // Save BDADDR
  for( uint8_t i = 0; i < 6; i++ ){
    uint16_t addr16 = bdaddr[i++];
    addr16 |= bdaddr[i] << 8;
    FlashWord16Write( faddr, addr16 );
    faddr += 2;
  }
}


