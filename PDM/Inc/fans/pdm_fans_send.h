
#ifndef PDM_PDM_FANS_SEND_H
#define PDM_PDM_FANS_SEND_H
#include "can/pdm_can.h"
#include "tx_api.h"
#include <stdint.h>

extern uint8_t fan_speeds[6];

UINT pdm_fans_tx_speed_tx_create(TX_BYTE_POOL *byte_pool);
VOID pdm_fans_tx_speed_thread_entry(ULONG thread_input);
#endif // PDM_PDM_FANS_SEND_H