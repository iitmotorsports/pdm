#include "fans/pdm_fans_emc2303.h"

const uint8_t k_fan_settings_regs[] = {0x32U, 0x42U, 0x52U};
const uint8_t k_fan_target_lb_regs[] = {0x3CU, 0x4CU, 0x5CU};
const uint8_t k_fan_target_hb_regs[] = {0x3DU, 0x4DU, 0x5DU};
const uint8_t k_fan_read_hb_regs[] = {0x3EU, 0x4EU, 0x5EU};
const uint8_t k_fan_read_lb_regs[] = {0x3FU, 0x4FU, 0x5FU};

// Assumes conversion from rpm to tach has already happened
// Assumes 1 byte with lost resolution, therefore only set HB and always set LB
// to 0 Assumes FSC SHOULD ONLY BE CALLED IN A THREAD Fans are indexed at 1 to
// maintain parity with EMC2303 docs
void pdm_fans_emc2303_write_speed(emc2303_t *dev, uint8_t tach,
                                  uint8_t fan_num) {
    uint8_t fan_ctrler_num = (fan_num - 1) % 3;
    uint8_t fan_ctrler_addrs = dev->ctrler_addr;
    uint8_t fan_address = k_fan_target_lb_regs[fan_ctrler_num];
    smbus_cmd_t cmd_lb = {
        0U, fan_ctrler_addrs, {fan_address, WRITE, 2, 0}, true};
    fan_address = k_fan_target_hb_regs[fan_ctrler_num];
    smbus_cmd_t cmd_hb = {
        tach, fan_ctrler_addrs, {fan_address, WRITE, 2, 0}, true};
    tx_queue_send(dev->smbus_queue, &cmd_lb, TX_NO_WAIT);
    tx_queue_send(dev->smbus_queue, &cmd_hb, TX_NO_WAIT);
}

// SHOULD ONLY BE CALLED IN A THREAD
// Fans are indexed at 1 to maintain parity with EMC2303 docs
void pdm_fans_emc2303_read_speed(emc2303_t *dev, uint8_t fan_num) {
    uint8_t fan_ctrler_num = (fan_num - 1) % 3;
    uint8_t fan_ctrler_addr = dev->ctrler_addr;
    uint8_t fan_address = k_fan_read_lb_regs[fan_ctrler_num];
    // Read HB before LB (in theory lb is always 0 though)
    smbus_cmd_t cmd_lb = {
        0U, fan_ctrler_addr, {fan_address, READ, 1, 1}, false};
    fan_address = k_fan_read_hb_regs[fan_ctrler_num];
    smbus_cmd_t cmd_hb = {
        0U, fan_ctrler_addr, {fan_address, READ, 1, 1}, false};
    tx_queue_send(dev->smbus_queue, &cmd_hb, TX_NO_WAIT);
    tx_queue_send(dev->smbus_queue, &cmd_lb, TX_NO_WAIT);
}

// ONLY CALL IN A THREAD
void pdm_fans_emc2303_set_push_pull(emc2303_t *dev, bool is_push_pull) {
    st_command_t WRITE_PUSH_PULL = {0x2B, WRITE, 2, 0};
    uint8_t data = 0x0U; // Data that you're writing Table 5.5
    if (is_push_pull) {
        data = 0x7U; // Data that you're writing Table 5.5
    }
    smbus_cmd_t cmd = {data, dev->ctrler_addr, WRITE_PUSH_PULL, true};
    tx_queue_send(dev->smbus_queue, &cmd, TX_NO_WAIT);
}

// ONLY CALL IN A THREAD
// Fans are indexed at 1 to maintain parity with EMC2303 docs
void pdm_fans_emc2303_set_fsc(emc2303_t *dev, bool is_fsc, uint8_t fan_num) {
    st_command_t WRITE_FSC_SETTINGS = {k_fan_settings_regs[fan_num - 1], WRITE,
                                       2, 0};
    // Likely will change since PDM will power pumps instead
    uint8_t data =
        0x3BU; // Default is 2B but for fans we're using 3B is correct
    // see Table 5.11
    if (is_fsc) {
        // FSC is the closed loop mode
        data = 0xBBU; // Table 5.10
    }
    smbus_cmd_t cmd = {data, dev->ctrler_addr, WRITE_FSC_SETTINGS, true};
    tx_queue_send(dev->smbus_queue, &cmd, TX_NO_WAIT);
}

void pdm_fans_emc2303_init(emc2303_t *dev, uint8_t ctrler_addr,
                           TX_QUEUE *smbus_queue) {
    dev->ctrler_addr = ctrler_addr;
    dev->smbus_queue = smbus_queue;
    dev->write_speed = pdm_fans_emc2303_write_speed;
    dev->read_speed = pdm_fans_emc2303_read_speed;
    dev->set_fsc = pdm_fans_emc2303_set_fsc;
    dev->set_push_pull = pdm_fans_emc2303_set_push_pull;
}
