#include "fans/pdm_fans.h"

#include "stm32_PMBUS_stack.h"


const uint8_t k_controller_addrs[] = {0x5C, 0x5E}; // Assumes writing to device | If reading needs to be ored with 0x1 3.1.2
const uint8_t k_fan_settings_regs[] = {0x32U, 0x42U, 0x52U};
const uint8_t k_fan_target_lb_regs[] = {0x3CU, 0x4CU, 0x5CU};
const uint8_t k_fan_target_hb_regs[] = {0x3DU, 0x4DU, 0x5DU};

SMBUS_HandleTypeDef context1;
context1.CMD_table = (st_command_t *) &PMBUS_COMMANDS_TAB[0];
context1.CMD_tableSize = PMBUS_COMMANDS_TAB_SIZE;
context1.SRByte = 0x55U;
context1.CurrentCommand = NULL;
STACK_SMBUS_Init(&context1);

void pdm_fans_init() {
    HAL_GPIO_WritePin(THERM_EN_GPIO_Port, THERM_EN_Pin, GPIO_PIN_SET);
    uint8_t *piobuf = NULL;
    piobuf = STACK_SMBUS_GetBuffer(&context1);

    for (uint8_t i = 0; i < (uint8_t)(sizeof(k_controller_addrs)/sizeof(k_controller_addrs[0])); i++) {
        st_command_t WRITE_PUSH_PULL = {0x2B, WRITE, 2, 0};
        piobuf[0] = 0x7; // Data that you're writing Table 5.5
        STACK_SMBUS_HostCommand(&context1, &WRITE_PUSH_PULL, k_controller_addrs[i], WRITE);
        while (STACK_SMBUS_IsBusy(&context1)) {HAL_Delay(1);}
        for (uint8_t j = 0; j < (uint8_t)(sizeof(k_fan_settings_regs)/sizeof(k_fan_settings_regs[0])); j++) {
            piobuf = STACK_SMBUS_GetBuffer(&context1);
            // FSC is the closed loop mode
            st_command_t WRITE_FSC_SETTINGS = {k_fan_settings_regs[j], WRITE, 2, 0};
            piobuf[0] = 0xBB; // Table 5.10
            STACK_SMBUS_HostCommand(&context1, &WRITE_FSC_SETTINGS, k_controller_addrs[i], WRITE);
            while (STACK_SMBUS_IsBusy(&context1)) {HAL_Delay(1);}
        }
    }
}