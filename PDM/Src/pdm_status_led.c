#include "pdm_status_led.h"
#include "stm32g4xx_hal_gpio.h"

VOID pdm_status_led_thread_entry(ULONG thread_input) {
    while (1) {
        HAL_GPIO_WritePin(HSEN1_GPIO_Port, HSEN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(HSEN2_GPIO_Port, HSEN2_Pin, GPIO_PIN_SET);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
        HAL_GPIO_WritePin(HSEN1_GPIO_Port, HSEN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(HSEN2_GPIO_Port, HSEN2_Pin, GPIO_PIN_RESET);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
    }
}