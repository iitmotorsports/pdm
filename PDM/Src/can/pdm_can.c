#include "can/pdm_can.h"


#define SMBUS_QUEUE_DEPTH 12
#define SMBUS_QUEUE_OBJ_SIZE (sizeof(can_frame_t) / sizeof(ULONG))

TX_QUEUE can_tx_queue;
static ULONG can_tx_queue_storage[SMBUS_QUEUE_DEPTH * SMBUS_QUEUE_OBJ_SIZE];

void pdm_can_init() {
    UINT status =
        tx_queue_create(&can_tx_queue, "can_tx_queue", SMBUS_QUEUE_OBJ_SIZE,
                        can_tx_queue_storage, sizeof(can_tx_queue_storage));
    if (status != TX_SUCCESS) {
        Error_Handler();
    }

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
    case 0x102:
        for (uint8_t i = 0; i < rx_data_len; i++) {
            pdm_fans_write_speed(rx_data[i], i+1);
        }
    default:
        // TODO: This works for debugging but obviously don't want to just send
        // back unrecognized CAN msgs
        pdm_can_tx(rx_data, rx_data_len, rx_id);
        break;
    }
}

VOID pdm_can_tx_thread_entry() {
    can_frame_t *can_frame = NULL;
    UINT status = tx_queue_receive(&can_tx_queue, (can_frame_t *)&can_frame,
                                   TX_WAIT_FOREVER);
    if (status != TX_SUCCESS) {
        Error_Handler();
    }
    pdm_can_tx(can_frame->data, can_frame->data_len, can_frame->id);
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
