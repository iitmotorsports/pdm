#include "pdm_status_led.h"
#include "stm32g4xx_hal_gpio.h"

VOID pdm_status_led_thread_entry(ULONG thread_input) {
    while (1) {
        HAL_GPIO_WritePin(USER_R_GPIO_Port, USER_R_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(USER_G_GPIO_Port, USER_G_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(USER_B_GPIO_Port, USER_B_Pin, GPIO_PIN_SET);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
        HAL_GPIO_WritePin(USER_R_GPIO_Port, USER_R_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(USER_G_GPIO_Port, USER_G_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(USER_B_GPIO_Port, USER_B_Pin, GPIO_PIN_RESET);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
    }
}