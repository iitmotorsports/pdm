#include "fans/pdm_fans.h"

const uint8_t k_controller_addrs[] = {0x5C, 0x5E};
const uint8_t k_fan_settings_regs[] = {0x32U, 0x42U, 0x52U};
const uint8_t k_fan_target_lb_regs[] = {0x3CU, 0x4CU, 0x5CU};
const uint8_t k_fan_target_hb_regs[] = {0x3DU, 0x4DU, 0x5DU};
const uint8_t k_fan_read_hb_regs[] = {0x3EU, 0x4EU, 0x5EU};
const uint8_t k_fan_read_lb_regs[] = {0x3FU, 0x4FU, 0x5FU};

SMBUS_StackHandleTypeDef context1;

#define TIMEOUT (TX_TIMER_TICKS_PER_SECOND / 10)

// Assumes conversion from rpm to tach has already happened
// Assumes 1 byte with lost resolution, therefore only set HB and always set LB to 0
// Assumes FSC
// SHOULD ONLY BE CALLED IN A THREAD
// Fans are indexed at 1 to maintain parity with EMC2303 docs
void pdm_fans_write_speed(uint8_t tach, uint8_t fan_num) {
    uint8_t fan_ctrler_num = (fan_num - 1) % 3;
    uint8_t ctrler_num = fan_num / 3;
    uint8_t fan_ctrler_addrs = k_controller_addrs[ctrler_num];
    uint8_t fan_address = k_fan_target_lb_regs[fan_ctrler_num];
    smbus_cmd_t cmd_lb = {0U, fan_ctrler_addrs, {fan_address, WRITE, 2, 0}, true};
    fan_address = k_fan_target_hb_regs[fan_ctrler_num];
    smbus_cmd_t cmd_hb = {tach, fan_ctrler_addrs, {fan_address, WRITE, 2, 0}, true};
    tx_queue_send(&smbus_queue, &cmd_lb, TX_NO_WAIT);
    tx_queue_send(&smbus_queue, &cmd_hb, TX_NO_WAIT);
}

// SHOULD ONLY BE CALLED IN A THREAD
// Fans are indexed at 1 to maintain parity with EMC2303 docs
void pdm_fans_read_speed(uint8_t fan_num) {
    uint8_t fan_ctrler_num = (fan_num - 1) % 3;
    uint8_t ctrler_num = (fan_num - 1) / 3;
    uint8_t fan_ctrler_addr = k_controller_addrs[ctrler_num];
    uint8_t fan_address = k_fan_read_lb_regs[fan_ctrler_num];
    //Read HB before LB (in theory lb is always 0 though)
    smbus_cmd_t cmd_lb = {0U, fan_ctrler_addr, { fan_address, READ, 1, 1} , false};
    fan_address = k_fan_target_hb_regs[fan_ctrler_num];
    smbus_cmd_t cmd_hb = {0U, fan_ctrler_addr, {fan_address, READ, 1, 1} , false};
    tx_queue_send(&smbus_queue, &cmd_hb, TX_NO_WAIT);
    tx_queue_send(&smbus_queue, &cmd_lb, TX_NO_WAIT);
}

// ONLY CALL IN A THREAD
void pdm_fans_set_push_pull(bool is_push_pull, uint8_t controller_addr) {
    uint8_t *piobuf = NULL;
    piobuf = STACK_SMBUS_GetBuffer(&context1);
    st_command_t WRITE_PUSH_PULL = {0x2B, WRITE, 2, 0};
    piobuf[0] = 0x0U; // Data that you're writing Table 5.5
    if (is_push_pull) {
        piobuf[0] = 0x7U; // Data that you're writing Table 5.5
    }
    STACK_SMBUS_HostCommand(&context1, &WRITE_PUSH_PULL, controller_addr, WRITE);
    uint8_t elapsed = 0;
    while (STACK_SMBUS_IsBusy(&context1) && elapsed < TIMEOUT) {
        tx_thread_sleep(1);
        elapsed++;
    }
    if (elapsed >= TIMEOUT) {
        Error_Handler(); //TODO: SMBUS timeout
    }
}

// ONLY CALL IN A THREAD
void pdm_fans_set_fsc(bool is_fsc, uint8_t controller_addr, uint8_t fan_reg_addr) {
    uint8_t *piobuf = NULL;
    piobuf = STACK_SMBUS_GetBuffer(&context1);
    st_command_t WRITE_FSC_SETTINGS = {fan_reg_addr, WRITE, 2, 0};
    // Likely will change since PDM will power pumps instead
    piobuf[0] = 0x3BU; // Default is 2B but for fans we're using 3B is correct see Table 5.11
    if (is_fsc) {
        // FSC is the closed loop mode
        piobuf[0] = 0xBBU; // Table 5.10
    }
    STACK_SMBUS_HostCommand(&context1, &WRITE_FSC_SETTINGS, controller_addr, WRITE);
    uint8_t elapsed = 0;
    while (STACK_SMBUS_IsBusy(&context1) && elapsed < TIMEOUT) {
        tx_thread_sleep(1);
        elapsed++;
    }
    if (elapsed >= TIMEOUT) {
        Error_Handler(); //TODO: SMBUS never recieved a respo
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
}