#include "pdm.h"
#include "can/pdm_can.h"
#include "can/pdm_can_heartbeat.h"
#include "fans/pdm_fans_smbus_queue.h"
#include "main.h"
#include "pdm_status_led.h"

#define PDM_STATUS_LED_STACK_SIZE 512
#define PDM_STATUS_LED_PRIORITY 20

static TX_THREAD pdm_status_led_thread;
static uint8_t pdm_status_led_stack[PDM_STATUS_LED_STACK_SIZE];

#define PDM_CAN_HEARTBEAT_STACK_SIZE 512
#define PDM_CAN_HEARTBEAT_PRIORITY 20

static TX_THREAD pdm_can_heartbeat_thread;
static uint8_t pdm_can_heartbeat_stack[PDM_CAN_HEARTBEAT_STACK_SIZE];

#define PDM_CAN_TX_STACK_SIZE 512
#define PDM_CAN_TX_PRIORITY 20

static TX_THREAD pdm_can_tx_thread;
static uint8_t pdm_can_tx_stack[PDM_CAN_TX_STACK_SIZE];

#define PDM_SMBUS_QUEUE_STACK_SIZE 512
#define PDM_SMBUS_QUEUE_PRIORITY 20

static TX_THREAD pdm_smbus_queue_thread;
static uint8_t pdm_smbus_queue_stack[PDM_SMBUS_QUEUE_STACK_SIZE];

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
    pdm_smbus_queue_init();
    // pdm_fans_init();
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
    UINT status = tx_thread_create(
        &pdm_status_led_thread, "pdm_status_led", pdm_status_led_thread_entry,
        0, pdm_status_led_stack, PDM_STATUS_LED_STACK_SIZE,
        PDM_STATUS_LED_PRIORITY, PDM_STATUS_LED_PRIORITY, 1, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        return TX_THREAD_ERROR;
    }

    status = tx_thread_create(
        &pdm_can_heartbeat_thread, "pdm_can_heartbeat",
        pdm_can_heartbeat_thread_entry, 0, pdm_can_heartbeat_stack,
        PDM_CAN_HEARTBEAT_STACK_SIZE, PDM_CAN_HEARTBEAT_PRIORITY,
        PDM_CAN_HEARTBEAT_PRIORITY, 1, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        return TX_THREAD_ERROR;
    }

    status = tx_thread_create(
        &pdm_can_tx_thread, "pdm_can_tx",
        pdm_can_tx_thread_entry, 0, pdm_can_tx_stack,
        PDM_CAN_TX_STACK_SIZE, PDM_CAN_TX_PRIORITY,
        PDM_CAN_TX_PRIORITY, 1, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        return TX_THREAD_ERROR;
    }

    status = tx_thread_create(
        &pdm_smbus_queue_thread, "pdm_smbus",
        pdm_smbus_queue_thread_entry, 0, pdm_smbus_queue_stack,
        PDM_SMBUS_QUEUE_STACK_SIZE, PDM_SMBUS_QUEUE_PRIORITY,
        PDM_SMBUS_QUEUE_PRIORITY, 1, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        return TX_THREAD_ERROR;
    }

    return status;
};