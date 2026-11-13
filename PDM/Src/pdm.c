#include "pdm.h"

void pdm_init() {
    // Show PDM is working before entering threads
    uint16_t i = 0;
    while (i < 5000) {
        HAL_GPIO_TogglePin(USER_R_GPIO_Port, USER_R_Pin);
        HAL_GPIO_TogglePin(USER_G_GPIO_Port, USER_G_Pin);
        HAL_GPIO_TogglePin(USER_B_GPIO_Port, USER_B_Pin);
        HAL_Delay(50);
        i += 50;
    }
    pdm_can_init();
    pdm_fans_init();
    pdm_fans_smbus_queue_init();
}

void pdm_main_loop() {
    HAL_GPIO_WritePin(USER_R_GPIO_Port, USER_R_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(USER_G_GPIO_Port, USER_G_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(USER_B_GPIO_Port, USER_B_Pin, GPIO_PIN_SET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(USER_R_GPIO_Port, USER_R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(USER_G_GPIO_Port, USER_G_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(USER_B_GPIO_Port, USER_B_Pin, GPIO_PIN_RESET);
}

UINT pdm_threads_create(TX_BYTE_POOL *byte_pool) {
    UINT status = pdm_status_led_create(byte_pool);
    if (status != TX_SUCCESS) {
        return status;
    }

    status = pdm_can_tx_create(byte_pool);
    if (status != TX_SUCCESS) {
        return status;
    }

    status = pdm_can_hsd_create(byte_pool);
    if (status != TX_SUCCESS) {
        return status;
    }

    status = pdm_can_heartbeat_create(byte_pool);
    if (status != TX_SUCCESS) {
        return status;
    }

    status = pdm_smbus_queue_create(byte_pool);
    if (status != TX_SUCCESS) {
        return status;
    }

    status = pdm_fans_tx_speed_create(byte_pool);

    return status;
};
