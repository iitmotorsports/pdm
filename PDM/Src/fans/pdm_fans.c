#include "fans/pdm_fans.h"

const uint8_t k_controller_addrs[] = {0x5C, 0x5E};
const uint8_t k_fan_settings_regs[] = {0x32U, 0x42U, 0x52U};
const uint8_t k_fan_target_lb_regs[] = {0x3CU, 0x4CU, 0x5CU};
const uint8_t k_fan_target_hb_regs[] = {0x3DU, 0x4DU, 0x5DU};
const uint8_t k_fan_read_hb_regs[] = {0x3EU, 0x4EU, 0x5EU};
const uint8_t k_fan_read_lb_regs[] = {0x3FU, 0x4FU, 0x5FU};

SMBUS_StackHandleTypeDef context1;

// Assumes conversion from rpm to tach has already happened
// Assumes 1 byte with lost resolution, therefore only set HB and always set LB to 0
// Assumes FSC
// SHOULD ONLY BE CALLED IN A THREAD
void pdm_fans_write_speed(uint8_t tach, uint8_t fan_num) {
    uint8_t fan_ctrler_num = (fan_num - 1) % 3;
    uint8_t ctrler_num = fan_num / 3;
    uint8_t fan_ctrler_addrs = k_controller_addrs[ctrler_num];
    uint8_t fan_address = k_fan_target_lb_regs[fan_ctrler_num];
    smbus_cmd_t cmd_lb = {0U, fan_ctrler_addrs, {fan_address, WRITE, 2, 0}, true};
    fan_address = k_fan_target_hb_regs[fan_ctrler_num];
    smbus_cmd_t cmd_hb = {tach, fan_ctrler_addrs, {fan_address, WRITE, 2, 0}, true};
    tx_queue_send(&smbus_queue, &cmd_lb, TX_WAIT_FOREVER); // FIXME: WAIT_FOREVER is probably not a good idea for prod
    tx_queue_send(&smbus_queue, &cmd_hb, TX_WAIT_FOREVER);
}

// SHOULD ONLY BE CALLED IN A THREAD
void pdm_fans_read_speed(uint8_t fan_num) {
    uint8_t fan_ctrler_num = (fan_num - 1) % 3;
    uint8_t ctrler_num = (fan_num - 1) / 3;
    uint8_t fan_ctrler_addr = k_controller_addrs[ctrler_num];
    uint8_t fan_address = k_fan_read_lb_regs[fan_ctrler_num];
    //Read HB before LB (in theory lb is always 0 though)
    smbus_cmd_t cmd_lb = {0U, fan_ctrler_addr, { fan_address, READ, 1, 1} , false};
    fan_address = k_fan_target_hb_regs[fan_ctrler_num];
    smbus_cmd_t cmd_hb = {0U, fan_ctrler_addr, {fan_address, READ, 1, 1} , false};
    tx_queue_send(&smbus_queue, &cmd_hb, TX_WAIT_FOREVER);
    tx_queue_send(&smbus_queue, &cmd_lb, TX_WAIT_FOREVER); // FIXME: WAIT_FOREVER is probably not a good idea for prod
}

// DO NOT CALL IN A THREAD
void pdm_fans_set_push_pull(bool is_push_pull, uint8_t controller_addr) {
    uint8_t *piobuf = NULL;
    piobuf = STACK_SMBUS_GetBuffer(&context1);
    if (is_push_pull) {
        st_command_t WRITE_PUSH_PULL = {0x2B, WRITE, 2, 0};
        piobuf[0] = 0x7; // Data that you're writing Table 5.5
        STACK_SMBUS_HostCommand(&context1, &WRITE_PUSH_PULL, controller_addr, WRITE);
        while (STACK_SMBUS_IsBusy(&context1)) {HAL_Delay(1);}
    } // open drain is default so no need to change
}

// DO NOT CALL IN A THREAD
void pdm_fans_set_fsc(bool is_fsc, uint8_t controller_addr, uint8_t fan_reg_addr) {
    uint8_t *piobuf = NULL;
    piobuf = STACK_SMBUS_GetBuffer(&context1);
    if (is_fsc) {
        piobuf = STACK_SMBUS_GetBuffer(&context1);
        // FSC is the closed loop mode
        st_command_t WRITE_FSC_SETTINGS = {fan_reg_addr, WRITE, 2, 0};
        piobuf[0] = 0xBB; // Table 5.10
        STACK_SMBUS_HostCommand(&context1, &WRITE_FSC_SETTINGS, controller_addr, WRITE);
        while (STACK_SMBUS_IsBusy(&context1)) {HAL_Delay(1);}
    }
}

void pdm_fans_init() {
    HAL_GPIO_WritePin(THERM_EN_GPIO_Port, THERM_EN_Pin, GPIO_PIN_SET);
    context1.Device = &hsmbus4;
    context1.CMD_table = (st_command_t *) &PMBUS_COMMANDS_TAB[0];
    context1.CMD_tableSize = PMBUS_COMMANDS_TAB_SIZE;
    context1.SRByte = 0x55U;
    context1.CurrentCommand = NULL;
    STACK_SMBUS_Init(&context1);

    for (uint8_t i = 0; i < (uint8_t)(sizeof(k_controller_addrs)/sizeof(k_controller_addrs[0])); i++) {
        pdm_fans_set_push_pull(true, k_controller_addrs[i]);
        for (uint8_t j = 0; j < (uint8_t)(sizeof(k_fan_settings_regs)/sizeof(k_fan_settings_regs[0])); j++) {
            pdm_fans_set_fsc(true, k_controller_addrs[i], k_fan_settings_regs[j]);
        }
    }
}