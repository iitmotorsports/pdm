#include "stm32g4xx_hal_gpio.h"
#include "main.h"
#include "pdm.h"

#include "pdm_status_led.h"

#define PDM_STATUS_LED_STACK_SIZE 512
#define PDM_STATUS_LED_PRIORITY 20

static TX_THREAD pdm_status_led_thread;
static uint8_t pdm_status_led_stack[PDM_STATUS_LED_STACK_SIZE];

void pdm_init() {

}

void pdm_main_loop() {
    HAL_GPIO_WritePin(HSEN1_GPIO_Port, HSEN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HSEN2_GPIO_Port, HSEN2_Pin, GPIO_PIN_SET);
    HAL_Delay(250);
    HAL_GPIO_WritePin(HSEN1_GPIO_Port, HSEN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HSEN2_GPIO_Port, HSEN2_Pin, GPIO_PIN_RESET);
    HAL_Delay(250);
}

UINT pdm_threads_create(TX_BYTE_POOL* byte_pool) {
    UINT status = tx_thread_create(&pdm_status_led_thread, "pdm_status_led",
        pdm_status_led_thread_entry, 0, pdm_status_led_stack, PDM_STATUS_LED_STACK_SIZE,
        PDM_STATUS_LED_PRIORITY, PDM_STATUS_LED_PRIORITY, 1, TX_AUTO_START);

    if (status != TX_SUCCESS) {
        return TX_THREAD_ERROR;
    }
    return status;
};
