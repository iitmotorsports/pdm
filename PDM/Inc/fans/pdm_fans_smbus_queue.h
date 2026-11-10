#ifndef PDM_PDM_SMBUS_QUEUE_H
#define PDM_PDM_SMBUS_QUEUE_H
#include "can/pdm_can_heartbeat.h"
#include "fans/pdm_fans_emc2303.h"
#include "stm32_SMBUS_stack.h"
#include "tx_api.h"
#include <stdbool.h>

UINT pdm_smbus_queue_create(TX_BYTE_POOL *byte_pool);
void pdm_fans_smbus_queue_init();
extern const uint8_t k_controller_addrs[2];
extern SMBUS_HandleTypeDef hsmbus4;

#endif // PDM_PDM_SMBUS_QUEUE_H