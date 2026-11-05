#include "fans/pdm_fans_send.h"

#include "can/pdm_can_heartbeat.h"

#define PDM_FANS_TX_SPEED_STACK_SIZE 512
#define PDM_FANS_TX_SPEED_PRIORITY 20

static TX_THREAD pdm_fans_tx_speed_thread;
static uint8_t pdm_fans_tx_speed_stack[PDM_FANS_TX_SPEED_STACK_SIZE];

UINT pdm_fans_tx_speed_tx_create(TX_BYTE_POOL *byte_pool) {
    UINT status = tx_thread_create(
        &pdm_fans_tx_speed_thread, "pdm_can_tx_fan_speed",
        pdm_fans_tx_speed_thread_entry, 0, pdm_fans_tx_speed_stack,
        PDM_FANS_TX_SPEED_STACK_SIZE, PDM_FANS_TX_SPEED_PRIORITY,
        PDM_FANS_TX_SPEED_PRIORITY, 1, TX_AUTO_START);
    return status;
}

VOID pdm_fans_tx_speed_thread_entry(ULONG thread_input) {
    while (1) {
        pdm_fan_can_callback_send();
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 4); // Sleep to let fan_speeds update
        can_frame_t fan_speed_frame = {{0}, 8, 0x01};
        // fan_speeds's size is explicitly 6 so should be okay
        memcpy(fan_speed_frame.data, fan_speeds, sizeof(fan_speeds));
        tx_queue_send(&can_tx_queue, &fan_speed_frame, TX_NO_WAIT);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND * 3 / 4);
    }
}