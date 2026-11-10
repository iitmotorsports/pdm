#include "can/pdm_can.h"

#define CAN_QUEUE_DEPTH 12
#define CAN_QUEUE_OBJ_SIZE (sizeof(can_frame_t) / sizeof(ULONG))

TX_QUEUE can_tx_queue;
static ULONG can_tx_queue_storage[CAN_QUEUE_DEPTH * CAN_QUEUE_OBJ_SIZE];

#define PDM_CAN_TX_STACK_SIZE 512
#define PDM_CAN_TX_PRIORITY 20
static TX_THREAD pdm_can_tx_thread;
static uint8_t pdm_can_tx_stack[PDM_CAN_TX_STACK_SIZE];

#define PDM_CAN_HSD_STACK_SIZE 512
#define PDM_CAN_HSD_PRIORITY 20
static TX_THREAD pdm_can_hsd_thread;
static uint8_t pdm_can_hsd_stack[PDM_CAN_TX_STACK_SIZE];

const GPIO_Pin_t k_hsd_pins[4] = {
    {HSEN1_GPIO_Port, HSEN1_Pin},
    {HSEN2_GPIO_Port, HSEN2_Pin},
    {HSEN3_GPIO_Port, HSEN3_Pin},
    {HSEN4_GPIO_Port, HSEN4_Pin},
};

void pdm_can_init() {
    HAL_GPIO_WritePin(TERM_EN_GPIO_Port, TERM_EN_Pin, GPIO_PIN_SET);

    FDCAN_FilterTypeDef sFilterConfig;

    sFilterConfig.IdType = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1 = 0x000;
    sFilterConfig.FilterID2 = 0x000; // mask 0 = accept everything

    HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig);

    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
        Error_Handler();
    }
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}

void pdm_can_hsd_callback_recv(uint8_t rx_data[], uint8_t rx_data_len) {
    for (uint8_t i = 0; i < rx_data_len; i++) {
        if ((rx_data[i] & 0x2U) ==
            0x2) { // Check 2nd bit. 1 -> acknowledge | 0 -> ignore
            HAL_GPIO_WritePin(k_hsd_pins[i].port, k_hsd_pins[i].pin,
                              rx_data[i] & 0x1F ? GPIO_PIN_SET
                                                : GPIO_PIN_RESET);
        }
    }
}

void pdm_can_tx(const uint8_t tx_data[], uint8_t tx_data_len, uint16_t tx_id) {
    uint8_t payload[8] = {0}; // zero-init 8 bytes
    // Can't transmit more than 8 bytes
    if (tx_data_len > 8) {
        Error_Handler();
    }
    // Always 8 bytes, pad if too small
    memcpy(payload, tx_data, tx_data_len);

    FDCAN_TxHeaderTypeDef TxHeader;
    TxHeader.Identifier = tx_id;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF; // must be OFF for regular CAN
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, tx_data) != HAL_OK) {
        Error_Handler();
    }
}

void pdm_can_rx(uint8_t rx_data[], uint8_t rx_data_len, uint16_t rx_id) {
    switch (rx_id) {
    // Handler based on rx_id
    case FAN_RECEIVE: // ENUM Declared in pdm_can.h
        pdm_fan_can_callback_recv(rx_data, rx_data_len);
    case HSD_RECEIVE:
        pdm_can_hsd_callback_recv(rx_data, rx_data_len);
    default:
        // Do nothing for unrecognized CAN messages
        break;
    }
}

VOID pdm_can_tx_thread_entry() {
    while (1) {
        can_frame_t *can_frame = NULL;
        UINT status = tx_queue_receive(&can_tx_queue, (can_frame_t *)&can_frame,
                                       TX_WAIT_FOREVER);
        if (status != TX_SUCCESS) {
            Error_Handler();
        }
        pdm_can_tx(can_frame->data, can_frame->data_len, can_frame->id);
    }
}

UINT pdm_can_tx_create(TX_BYTE_POOL *byte_pool) {
    UINT status =
        tx_queue_create(&can_tx_queue, "can_tx_queue", CAN_QUEUE_OBJ_SIZE,
                        can_tx_queue_storage, sizeof(can_tx_queue_storage));
    if (status != TX_SUCCESS) {
        return status;
    }
    status = tx_thread_create(&pdm_can_tx_thread, "pdm_can_tx",
                              pdm_can_tx_thread_entry, 0, pdm_can_tx_stack,
                              PDM_CAN_TX_STACK_SIZE, PDM_CAN_TX_PRIORITY,
                              PDM_CAN_TX_PRIORITY, 1, TX_AUTO_START);
    return status;
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0) {
        FDCAN_RxHeaderTypeDef RxHeader;
        uint8_t RxData[8];

        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) ==
            HAL_OK) {
            pdm_can_rx(RxData, RxHeader.DataLength, RxHeader.Identifier);
        }
        HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                       0);
    }
}

VOID pdm_can_hsd_thread_entry() {
    while (1) {
        uint8_t hsd_data[sizeof(k_hsd_pins) / sizeof(k_hsd_pins[0])] = {0};
        for (int i = 0; i < sizeof(k_hsd_pins) / sizeof(k_hsd_pins[0]); i++) {
            hsd_data[i] = HAL_GPIO_ReadPin(k_hsd_pins[i].port,
                                           k_hsd_pins[i].pin) == GPIO_PIN_SET
                              ? 1
                              : 0;
        }
        can_frame_t can_frame = {{0}, 8, HSD_SEND};
        memcpy(can_frame.data, hsd_data, sizeof(hsd_data));
        tx_queue_send(&can_tx_queue, &can_frame, TX_NO_WAIT);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}

UINT pdm_can_hsd_create(TX_BYTE_POOL *byte_pool) {
    UINT status = tx_thread_create(
        &pdm_can_hsd_thread, "pdm_can_hsd", pdm_can_hsd_thread_entry, 0,
        pdm_can_hsd_stack, PDM_CAN_HSD_STACK_SIZE, PDM_CAN_HSD_PRIORITY,
        PDM_CAN_HSD_PRIORITY, 1, TX_AUTO_START);
    return status;
}