#include "fans/pdm_fans_send.h"

#include "can/pdm_can_heartbeat.h"

VOID pdm_fans_tx_speed_thread_entry(ULONG thread_input) {
    can_frame_t fan_speed_frame = {{0}, 8, 0x01};
    // fan_speeds's size is explicitly 6 so should be okay
    memcpy(fan_speed_frame.data, fan_speeds, sizeof(fan_speeds));
    tx_queue_send(&can_tx_queue, &fan_speed_frame, TX_NO_WAIT);
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
}
