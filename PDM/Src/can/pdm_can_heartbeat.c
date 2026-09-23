#include "can/pdm_can_heartbeat.h"
VOID pdm_can_heartbeat_thread_entry(ULONG thread_input) {
    while (1) {
        uint8_t tx_data[8] = {0x49U, 0x49U, 0x54U, 0x53U, 0x41U, 0x45};
        pdm_can_tx(tx_data, sizeof(tx_data), 0x001U);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}
