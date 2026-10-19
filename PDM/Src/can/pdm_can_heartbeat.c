#include "can/pdm_can_heartbeat.h"
VOID pdm_can_heartbeat_thread_entry(ULONG thread_input) {
    while (1) {
        can_frame_t heartbeat = {
            {0x49U, 0x49U, 0x54U, 0x53U, 0x41U, 0x45}, 6, 0x1};
        tx_queue_send(&can_tx_queue, &heartbeat, TX_WAIT_FOREVER);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}
