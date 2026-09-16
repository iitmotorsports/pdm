#include "pdm_status_led.h"

VOID pdm_status_led_thread_entry(ULONG thread_input) {
    while (1) {
        HAL_GPIO_TogglePin(USER_R_GPIO_Port, USER_R_Pin);
        HAL_GPIO_TogglePin(USER_G_GPIO_Port, USER_G_Pin);
        HAL_GPIO_TogglePin(USER_B_GPIO_Port, USER_B_Pin);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
    }
}