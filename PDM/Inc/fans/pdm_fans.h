#ifndef PDM_PDM_FANS_H
#define PDM_PDM_FANS_H
#include <stdbool.h>
#include "stm32_SMBUS_stack.h"
#include "stm32_PMBUS_stack.h"

extern SMBUS_HandleTypeDef hsmbus4;


typedef struct {
    uint8_t data;
    uint8_t controller_addr;
    st_command_t command;
    bool write;
} smbus_cmd_t;

void pdm_fans_init();
#endif // PDM_PDM_FANS_H
