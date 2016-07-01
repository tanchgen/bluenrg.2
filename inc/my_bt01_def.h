#ifndef _MY_BT10_DEF_H
#define _MY_BT10_DEF_H

/* Мои определения */
#define RELAY_OFF                          0x00
#define RELAY_ON                           0x01

/* ============== Отклики на принятые данные/команды ==================== */
#define TOKEN_SUCCESS       0x00 // Очередная часть токена передана без ошибок

/* Ошибки приема и выполнения комманд */
#define RECEIVE_ERR     0xFE // Ошибка приема команды
#define TOKEN_ERR       0xFD // Ошибка токена

#define REL_NUM_ERR     0xF0 // Номер ошибки приема номера реле
#define RELAY1_ON_ERR   0xF1 //Номер ошибки включения реле №1
#define RELAY2_ON_ERR   0xF2 //Номер ошибки включения реле №2
#define RELAY_STATUS_ERR    0xF4 //Ошибка получения статуса
#define RELAYx_ON_ERR(__INDEX__)  (__INDEX__) ? (RELAY1_ON_ERR) : (RELAY2_ON_ERR)

#define SHA_ERR_REASON  0x0E // Ошибка: принят неправильный sha-хэш ("Host Rejected due to security reasons")
#define SHA_TIMEOUT_REASON  0x13 // Ошибка: таймаут ожидания SHA-хэша

#define UNKNOWN_ERR     0xFF // Неопределенная ошибка

// ===================== Команды от клиента к серверу ==================

#define STRING_REQ_CMD      0x00 // Запрос строки для формирования токена
#define STRING_IS_READ	    0x01 // Подтверждение получения строки

#define RELAY_STATUS_CMD    0x10 // Запрос состояния реле
#define RELAY_ON_CMD        0x11 // команда включения реле

#define AUTH_REQ           0x03 // Клиенту отправлена "строка"

// ============== Прочие определения =============================
typedef enum
{
  RELAY1 = 0,
  RELAY2 = 1
} relayTypeDef;

typedef enum
{
  LED1 = 0,
  LED2 = 1
} ledTypeDef;

#define BDADDR_SIZE 6

typedef struct {
  uint8_t bdaddr[BDADDR_SIZE];    // Адрес забаненного клиента
  uint8_t banCount;               // Счетчик подключений без авторизации
  uint8_t banTime;                // Оставшееся время бана
} banTypeDef;


#define BAN_TIMEOUT         30    // Максимальное время бана
#define BANADDR_MAX         10    // Максимальное количество забаненых клиентов
/* ========== ТАЙМАУТЫ ================== */
#define MY_WD_TIME          600    // Таймаут (с) перезагрузки прибора, если нет подключения.
#define HW_TIMEOUT          100     // Общий таймаут (мс) для реакции железа контроллера

#define IWDG_START          0x0000CCCC      // Запуск WatchDog 
#define IWDG_WRITE_ACCESS   0x00005555      // Доступ записи в регистры
#define IWDG_RELOAD         2047            // Записываем регистр перезагрузки
#define IWDG_REFRESH        0x0000AAAA      // Перезагружаем счеттчик IWDG

#define TIM_2M                          0
#define TIM_REL1                        1
#define TIM_REL2                        2

#define BDADDR_SIZE 6   // Длина MAC-адреса bluetooth-устройства
#define TOKEN_LEN  52 // (Длина строки + длина UUID)  < 56 байт

/* Режимы вывода для канала 1 */
#define TIM_OC1MODE_TIMING                   ((uint32_t)0x0000)
#define TIM_OC1MODE_ACTIVE                   (TIM_CCMR1_OC1M_0)
#define TIM_OC1MODE_INACTIVE                 (TIM_CCMR1_OC1M_1)
#define TIM_OC1MODE_TOGGLE                   (TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1)
#define TIM_OC1MODE_PWM1                     (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2)
#define TIM_OC1MODE_PWM2                     (TIM_CCMR1_OC1M)
#define TIM_OC1MODE_FORCED_ACTIVE            (TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_2)
#define TIM_OC1MODE_FORCED_INACTIVE          (TIM_CCMR1_OC1M_2)

/* Режимы вывода для канала 2 */
#define TIM_OC2MODE_TIMING                   ((uint32_t)0x0000)
#define TIM_OC2MODE_ACTIVE                   (TIM_CCMR1_OC2M_0)
#define TIM_OC2MODE_INACTIVE                 (TIM_CCMR1_OC2M_1)
#define TIM_OC2MODE_TOGGLE                   (TIM_CCMR1_OC2M_0 | TIM_CCMR1_OC2M_1)
#define TIM_OC2MODE_PWM1                     (TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2)
#define TIM_OC2MODE_PWM2                     (TIM_CCMR1_OC2M)
#define TIM_OC2MODE_FORCED_ACTIVE            (TIM_CCMR1_OC2M_0 | TIM_CCMR1_OC2M_2)
#define TIM_OC2MODE_FORCED_INACTIVE          (TIM_CCMR1_OC2M_2)

/* Режимы вывода для канала 3 */
#define TIM_OC3MODE_TIMING                   ((uint32_t)0x0000)
#define TIM_OC3MODE_ACTIVE                   (TIM_CCMR2_OC3M_0)
#define TIM_OC3MODE_INACTIVE                 (TIM_CCMR2_OC3M_1)
#define TIM_OC3MODE_TOGGLE                   (TIM_CCMR2_OC3M_0 | TIM_CCMR2_OC3M_1)
#define TIM_OC3MODE_PWM1                     (TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2)
#define TIM_OC3MODE_PWM2                     (TIM_CCMR2_OC3M)
#define TIM_OC3MODE_FORCED_ACTIVE            (TIM_CCMR2_OC3M_0 | TIM_CCMR2_OC3M_2)
#define TIM_OC3MODE_FORCED_INACTIVE          (TIM_CCMR2_OC3M_2)

/* Режимы вывода для канала 4 */
#define TIM_OC4MODE_TIMING                   ((uint32_t)0x0000)
#define TIM_OC4MODE_ACTIVE                   (TIM_CCMR2_OC4M_0)
#define TIM_OC4MODE_INACTIVE                 (TIM_CCMR2_OC4M_1)
#define TIM_OC4MODE_TOGGLE                   (TIM_CCMR2_OC4M_0 | TIM_CCMR2_OC4M_1)
#define TIM_OC4MODE_PWM1                     (TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2)
#define TIM_OC4MODE_PWM2                     (TIM_CCMR2_OC4M)
#define TIM_OC4MODE_FORCED_ACTIVE            (TIM_CCMR2_OC4M_0 | TIM_CCMR2_OC4M_2)
#define TIM_OC4MODE_FORCED_INACTIVE          (TIM_CCMR2_OC4M_2)

#define CCR1_VAL          ((__IO uint16_t)500)
#define CCR3_VAL          ((__IO uint16_t)500)
#define CCR4_VAL          ((__IO uint16_t)500)

#endif //_MY_BT10_DEF_H
