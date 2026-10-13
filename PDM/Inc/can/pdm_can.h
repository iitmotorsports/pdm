#ifndef PDM_PDM_CAN_H
#define PDM_PDM_CAN_H

#include <string.h>
#include "tx_api.h"
#include "main.h"

extern FDCAN_HandleTypeDef hfdcan1;

typedef struct {
    uint8_t data[8];
    uint8_t data_len;
    uint16_t id;
} __attribute__((aligned(4))) can_frame_t;

void pdm_can_init();
VOID pdm_can_tx_thread_entry();

#endif // PDM_PDM_CAN_H
