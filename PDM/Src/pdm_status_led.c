#include "pdm_status_led.h"

#define PDM_STATUS_LED_STACK_SIZE 512
#define PDM_STATUS_LED_PRIORITY 20

static TX_THREAD pdm_status_led_thread;
static uint8_t pdm_status_led_stack[PDM_STATUS_LED_STACK_SIZE];

VOID pdm_status_led_thread_entry(ULONG thread_input) {
    while (1) {
        HAL_GPIO_TogglePin(USER_R_GPIO_Port, USER_R_Pin);
        HAL_GPIO_TogglePin(USER_G_GPIO_Port, USER_G_Pin);
        HAL_GPIO_TogglePin(USER_B_GPIO_Port, USER_B_Pin);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
    }
}

UINT pdm_status_led_create(TX_BYTE_POOL *byte_pool) {
    UINT status = tx_thread_create(
        &pdm_status_led_thread, "pdm_status_led", pdm_status_led_thread_entry,
        0, pdm_status_led_stack, PDM_STATUS_LED_STACK_SIZE,
        PDM_STATUS_LED_PRIORITY, PDM_STATUS_LED_PRIORITY, 1, TX_AUTO_START);
    return status;
}