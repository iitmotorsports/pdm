#ifndef PDM_PDM_CAN_H
#define PDM_PDM_CAN_H

#include "fans/pdm_fans.h"
#include "hawkdrive_can.h"
#include "main.h"
#include "tx_api.h"
#include "pdm_hsd.h"
#include <string.h>

extern FDCAN_HandleTypeDef hfdcan1;

typedef struct {
    uint8_t data[8];
    uint8_t data_len;
    uint16_t id;
} __attribute__((aligned(4))) can_frame_t;

enum canbus_ids {
    PDM_HEARTBEAT = 0x001,
    FAN_SEND = 0x02,
    FAN_RECEIVE = 0x102,
    SMBUS_MISC = 0x44,
};

void pdm_can_init();
UINT pdm_can_tx_create(TX_BYTE_POOL *byte_pool);

#endif // PDM_PDM_CAN_H
