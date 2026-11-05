#include "can/pdm_can_heartbeat.h"

#define PDM_CAN_HEARTBEAT_STACK_SIZE 512
#define PDM_CAN_HEARTBEAT_PRIORITY 20

static TX_THREAD pdm_can_heartbeat_thread;
static uint8_t pdm_can_heartbeat_stack[PDM_CAN_HEARTBEAT_STACK_SIZE];


UINT pdm_can_heartbeat_tx_create(TX_BYTE_POOL *byte_pool) {
    UINT status = tx_thread_create(
        &pdm_can_heartbeat_thread, "pdm_can_heartbeat",
        pdm_can_heartbeat_thread_entry, 0, pdm_can_heartbeat_stack,
        PDM_CAN_HEARTBEAT_STACK_SIZE, PDM_CAN_HEARTBEAT_PRIORITY,
        PDM_CAN_HEARTBEAT_PRIORITY, 1, TX_AUTO_START);
    return status;
}

VOID pdm_can_heartbeat_thread_entry(ULONG thread_input) {
    while (1) {
        const can_frame_t heartbeat = {
            {0x49U, 0x49U, 0x54U, 0x53U, 0x41U, 0x45}, 6, 0x1};
        tx_queue_send(&can_tx_queue, &heartbeat, TX_WAIT_FOREVER);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}
