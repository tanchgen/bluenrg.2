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
 *  Define the circular shift macro
 */
#define RoL(bits,word) \
                ((((word) << (bits)) & 0xFFFFFFFF) | \
                ((word) >> (32-(bits))))

extern volatile int connected;                  // Флаг действующего соединения
extern volatile uint16_t connection_handle; 
extern uint8_t tknStr[TOKEN_LEN+1];     // строка для вычесления токена
extern uint8_t shaHash[41];             // Длина SHA-hash - 40 байт
extern uint8_t tokenPart;


static const uint8_t ch[] = {'0','1','2','3','4','5','6','7','8','9',
                       'A','B','C','D','E','F','G','H','I','J','K','L','M',
                       'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
                       'a','b','c','d','e','f','g','h','i','j','k','l','m',
                       'n','o','p','q','r','s','t','u','v','w','x','y','z' };
static const char Device_UUID[] = { DEVICE_UUID };
uint32_t Pin = 123456;

volatile uint32_t disconnCount; // Таймаут аутентификации по SHA1-хэш 120 сек.
extern volatile uint16_t myWD;

uint16_t service_handle, dev_name_char_handle, appearance_char_handle;

/* Получение случайной строки для Токена */
void getTokenStr(uint8_t str[])
{
  uint32_t rand0;
  uint8_t i, b;
  uint32_t m, k;

  rand0 = myTick;
  m = 0x3FFFFFFF;                 // 2^31-1 - Модуль
  k = 1220703125;              // Множитель
  b = 7;                          // Прироащение
  for ( i=0; i < TOKEN_LEN-32; i++ )
  {
    rand0 = (( k * rand0 + b ) % m);
    str[i] = ch[(rand0 % 62)];     // 62 - Количество символов в наборе "ch[]"
  }
  for ( ; i < TOKEN_LEN; i++ ) {
    str[i] = Device_UUID[i-(TOKEN_LEN-32)];
  }
  str[i]='\0';
}

/* Получение SHA-хэш для токена. Строка выровнена побайтно */
void getShaHash(uint8_t * str, uint8_t token[] )
{
   uint8_t hash[320];           //Буфер хэша - 80x32 бита
   uint32_t h[] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
   uint32_t a = 0x67452301;
   uint32_t b = 0xEFCDAB89;
   uint32_t c = 0x98BADCFE;
   uint32_t d = 0x10325476;
   uint32_t e = 0xC3D2E1F0;

   uint16_t i;
   uint32_t w[80];
   uint32_t k, f;

   for ( i=0; i < TOKEN_LEN; i++)
   {
      hash[i] = *(str+i);
   }
   hash[i++] = 0x80;
   for ( ;i < 62; i++ )
   {
      hash[i] = 0x00;
   }
  f = TOKEN_LEN*8;
  // hash[i++] = (uint8_t)(( (uint32_t)(TOKEN_LEN*8) >> 24) & 0xFF);
  // hash[i++] = (uint8_t)(( (uint32_t)(TOKEN_LEN * 8 ) >> 16) & 0xFF);
   hash[i++] = (uint8_t)(( f >> 8) & 0xFF);
   hash[i] = (uint8_t)( f & 0xFF);

   //w = (uint32_t *)hash; // Оперируем 32-битными словами
   /* разбиваем этот кусок на 16 частей, слов по 32-бита w[i], 0 <= i <= 15
      16 слов по 32-бита дополняются до 80 32-битовых слов: */

    /*
     *  Initialize the first 16 words in the array W
     */
    for(i = 0; i < 16; i++)
    {
        w[i] = (hash[i * 4]) << 24;
        w[i] |= (hash[i * 4 + 1]) << 16;
        w[i] |= (hash[i * 4 + 2]) << 8;
        w[i] |= (hash[i * 4 + 3]);
    }


   //for i from 16 to 79
   //     w[i] = (w[i-3] xor w[i-8] xor w[i-14] xor w[i-16]) циклический сдвиг влево 1


   for ( i=16; i < 80; i++ )
   {
      w[i] = RoL(1, w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]);
   }
   /*   Основной цикл:
    for i from 0 to 79
        if 0 ? i ? 19 then
            f = (b and c) or ((not b) and d)
            k = 0x5A827999
        else if 20 ? i ? 39 then
            f = b xor c xor d
            k = 0x6ED9EBA1
        else if 40 ? i ? 59 then
            f = (b and c) or (b and d) or (c and d)
            k = 0x8F1BBCDC
        else if 60 ? i ? 79 then
            f = b xor c xor d
            k = 0xCA62C1D6

        temp = (a leftrotate 5) + f + e + k + w[i]
        e = d
        d = c
        c = b leftrotate 30
        b = a
        a = temp
   */

   for ( i=0; i<80; i++ )
   {
      if ( i<20 ) {
         f = (b & c) | (~b & d);
         k = 0x5A827999;
      }
      else if ( i<40 ) {
         f = b ^ c ^ d;
         k = 0x6ED9EBA1;
      }
      else if ( i<60 ) {
         f = (b & c) | (b & d) | (c & d);
         k = 0x8F1BBCDC;
      }
      else {
         f = b ^ c ^ d;
         k = 0xCA62C1D6;
      }

      //   temp = (a leftrotate 5) + f + e + k + w[i]
      f += RoL( 5, a ) + e + k + w[i];
      e = d;
      d = c;
      c = RoL( 30, b );
      b = a;
      a = f;
   }

   /*    Добавляем хеш-значение этой части к результату:
         h0 = h0 + a
         h1 = h1 + b
         h2 = h2 + c
         h3 = h3 + d
         h4 = h4 + e
     */
      h[0] += a;
      h[1] += b;
      h[2] += c;
      h[3] += d;
      h[4] += e;
      /* Итоговое хеш-значение:
         digest = hash = h0 append h1 append h2 append h3 append h4
      */
      for (i=0;i<5;i++){
        uint8_t j,k;
        for ( k=0; k<8; k++) {
          j=(uint8_t)((h[i] >> (28-k*4)) & 0xF);
          token[i*8+k] = j<0xA ? (j+'0') : ( j+0x57 );
        }
      }

   return;
}

int8_t setTokenStr( void ){
  int8_t err;
  // uint8_t tempStr[] = { "Vm21CmA2LAV3uRyMCT2u" };  // Постоянная строка для теста

  tokenPart = 0;          // Сбрасываем счетчик-"какая часть токена передана" 
  getTokenStr(tknStr);    // Получаем случайную строку для токена
  /* ============ Тестовый вариант : "Постоянная строка" ================
    for (uint8_t i=0; i<20; i++ ) tknStr[i]=tempStr[i];
  ==================================================================== */
  getShaHash(tknStr, shaHash);    // Получаем SHA-hash строки tknStr (Token)
  err = sendData(tknStr, (sizeof(tknStr)-33) );
  if ( !err ){
    disconnCount = 120; // Таймаут аутентификации по SHA1-хэш в мсек - 120 сек.
  }
  else {
    BlueNRG_Init();
//    aci_gap_terminate(connection_handle, SHA_TIMEOUT_REASON);
//    GAP_DisconnectionComplete_CB();
  }
  return err;
}

void myTimeOut( void ){
  // Таймаут 2 мин для токена
  if ( disconnCount ) { 
    if ( (--disconnCount) == 0) {
      aci_gap_terminate(connection_handle, SHA_TIMEOUT_REASON);
//      aci_gap_set_non_connectable( ADV_NONCONN_IND );
    }
  }
  if ( myWD ) {
    if ( ((--myWD) == 0) && !connected ){
      NVIC_SystemReset();
    }
  }
  
}  

#if WATCHDOG
void iwdgInit( void ){
  IWDG->KR = IWDG_START;          // Запуск WatchDog 
  IWDG->KR = IWDG_WRITE_ACCESS;   // Доступ записи в регистры
  IWDG->PR = IWDG_PR_PR_2;        // Прескалер = 64;
  IWDG->RLR = 2047;               // Записываем регистр перезагрузки
  for ( uint32_t tout = myTick+HW_TIMEOUT; !(IWDG->SR ) ; ) {
    if ( myTick  > tout) {
      // Проблема с IWDG
      HardFault_Handler();
    }
  }
  IWDG->KR = IWDG_REFRESH;
}
#endif // WATCHDOG

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

