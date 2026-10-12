#ifndef PDM_PDM_SMBUS_QUEUE_H
#define PDM_PDM_SMBUS_QUEUE_H
#include "tx_api.h"
#include <stdbool.h>
#include "stm32_SMBUS_stack.h"
#include "pdm_fans.h"


extern SMBUS_StackHandleTypeDef context1;


void pdm_smbus_queue_init();

#endif //PDM_PDM_SMBUS_QUEUE_H