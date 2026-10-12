#ifndef PDM_PDM_SMBUS_QUEUE_H
#define PDM_PDM_SMBUS_QUEUE_H
#include "tx_api.h"
#include <stdbool.h>
#include "stm32_SMBUS_stack.h"

typedef struct {
    uint8_t data;
    uint8_t controller_addr;
    st_command_t command;
    bool write;
} smbus_cmd_t;

extern SMBUS_StackHandleTypeDef context1;

void pdm_smbus_queue_init();

#endif //PDM_PDM_SMBUS_QUEUE_H