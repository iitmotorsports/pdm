#ifndef PDM_PDM_SMBUS_QUEUE_H
#define PDM_PDM_SMBUS_QUEUE_H
#include "pdm_fans.h"
#include "stm32_SMBUS_stack.h"
#include "tx_api.h"
#include <stdbool.h>

extern SMBUS_StackHandleTypeDef context1;

UINT pdm_smbus_queue_tx_create(TX_BYTE_POOL *byte_pool);
void pdm_smbus_queue_init();
VOID pdm_smbus_queue_thread_entry(ULONG thread_input);

#endif // PDM_PDM_SMBUS_QUEUE_H