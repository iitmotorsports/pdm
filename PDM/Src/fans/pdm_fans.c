#include "fans/pdm_fans.h"

const uint8_t k_controller_addrs[] = {0x5C, 0x5E};

#define PDM_FANS_TX_SPEED_STACK_SIZE 512
#define PDM_FANS_TX_SPEED_PRIORITY 20

#define NUM_FANS 3

static TX_THREAD pdm_fans_tx_speed_thread;
static uint8_t pdm_fans_tx_speed_stack[PDM_FANS_TX_SPEED_STACK_SIZE];

emc2303_t ctrler_1;
emc2303_t ctrler_2;

void pdm_fan_can_callback_recv(uint8_t rx_data[], uint8_t rx_data_len) {
    for (uint8_t i = 0; i < rx_data_len; i++) {
        if (rx_data[i] != 0) {
            if (i / 3 < 1) {
                ctrler_1.write_speed(&ctrler_1, rx_data[i], (i % 3) + 1);
            } else {
                ctrler_2.write_speed(&ctrler_1, rx_data[i], (i % 3) + 1);
            }
        }
    }
}

void pdm_fan_can_callback_send() {
    for (uint8_t i = 0;
         i < (uint8_t)(sizeof(fan_speeds) / sizeof(fan_speeds[0])); i++) {
        if (i / 3 < 1) {
            ctrler_1.read_speed(&ctrler_1, (i % 3) + 1);
        } else {
            ctrler_2.read_speed(&ctrler_1, (i % 3) + 1);
        }
    }
}

UINT pdm_fans_tx_speed_tx_create(TX_BYTE_POOL *byte_pool) {
    UINT status = tx_thread_create(
        &pdm_fans_tx_speed_thread, "pdm_can_tx_fan_speed",
        pdm_fans_tx_speed_thread_entry, 0, pdm_fans_tx_speed_stack,
        PDM_FANS_TX_SPEED_STACK_SIZE, PDM_FANS_TX_SPEED_PRIORITY,
        PDM_FANS_TX_SPEED_PRIORITY, 1, TX_AUTO_START);
    return status;
}

VOID pdm_fans_tx_speed_thread_entry(ULONG thread_input) {
    // On each controller enable push_pull to match how our fan controllers are
    // configured
    ctrler_1.set_push_pull(&ctrler_1, true);
    ctrler_2.set_push_pull(&ctrler_2, true);
    // On each fan
    for (uint8_t i = 0; i < NUM_FANS; i++) {
        // Enable FSC mode to control the fan through RPM
        ctrler_1.set_fsc(&ctrler_1, true, i + 1);
        ctrler_2.set_fsc(&ctrler_2, true, i + 1);
    }
    while (1) {
        pdm_fan_can_callback_send();
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND /
                        4); // Sleep to let fan_speeds update
        can_frame_t fan_speed_frame = {{0}, 8, 0x01};
        // fan_speeds's size is explicitly 6 so should be okay
        memcpy(fan_speed_frame.data, fan_speeds, sizeof(fan_speeds));
        tx_queue_send(&can_tx_queue, &fan_speed_frame, TX_NO_WAIT);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND * 3 / 4);
    }
}

void pdm_fans_init() {
    pdm_fans_emc2303_init(&ctrler_1, k_controller_addrs[0], &smbus_queue);
    pdm_fans_emc2303_init(&ctrler_2, k_controller_addrs[1], &smbus_queue);
}