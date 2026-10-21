#include "fans/pdm_fans_smbus_queue.h"

#include "can/pdm_can_heartbeat.h"

#define SMBUS_QUEUE_DEPTH 12
#define SMBUS_QUEUE_OBJ_SIZE (sizeof(smbus_cmd_t) / sizeof(ULONG))

#define READ_HB 0xEU
#define READ_LB 0xFU

#define TIMEOUT (TX_TIMER_TICKS_PER_SECOND / 10)

TX_QUEUE smbus_queue;
static ULONG smbus_queue_storage[SMBUS_QUEUE_DEPTH * SMBUS_QUEUE_OBJ_SIZE];

uint8_t fan_speeds[6];

void pdm_smbus_queue_init() {
    UINT status =
        tx_queue_create(&smbus_queue, "smbus_queue", SMBUS_QUEUE_OBJ_SIZE,
                        smbus_queue_storage, sizeof(smbus_queue_storage));
    if (status != TX_SUCCESS) {
        Error_Handler();
    }
}

VOID pdm_smbus_queue_thread_entry(ULONG thread_input) {
    smbus_cmd_t *smbus_cmd = NULL;
    uint8_t *piobuf = NULL;
    for (uint8_t i = 0; i < (uint8_t)(sizeof(k_controller_addrs) /
                                      sizeof(k_controller_addrs[0]));
         i++) {
        pdm_fans_set_push_pull(true, k_controller_addrs[i]);
        for (uint8_t j = 0; j < (uint8_t)(sizeof(k_fan_settings_regs) /
                                          sizeof(k_fan_settings_regs[0]));
             j++) {
            pdm_fans_set_fsc(true, k_controller_addrs[i],
                             k_fan_settings_regs[j]);
        }
    }
    while (1) {
        UINT status = tx_queue_receive(&smbus_queue, (smbus_cmd_t *)&smbus_cmd,
                                       TX_WAIT_FOREVER);
        if (status != TX_SUCCESS) {
            Error_Handler();
        }
        piobuf = STACK_SMBUS_GetBuffer(&context1);
        piobuf[0] = smbus_cmd->data;
        STACK_SMBUS_HostCommand(&context1, &smbus_cmd->command,
                                smbus_cmd->controller_addr,
                                smbus_cmd->write ? WRITE : READ);
        uint8_t elapsed = 0;
        while (STACK_SMBUS_IsBusy(&context1) && elapsed < TIMEOUT) {
            elapsed++;
            tx_thread_sleep(1);
        }
        if (elapsed >= TIMEOUT) {
            Error_Handler(); // TODO: SMBUS never recieved a respo
        }
        if (!smbus_cmd->write) {
            const uint8_t result = piobuf[0];
            // 1. Check if it's a fan write
            // 2. Determine fan #
            // 3. Write speed to corresponding array slot
            //  4. If not a fan read be very confused
            switch (smbus_cmd->command.cmnd_code) {
            case READ_HB: {
                uint8_t fan_offset =
                    smbus_cmd->controller_addr == k_controller_addrs[0] ? 0 : 2;
                uint8_t fan_num =
                    ((smbus_cmd->command.cmnd_code & 0xF0) >> 4) - 0x3U;
                uint8_t fan_index = fan_offset + fan_num;
                fan_speeds[fan_index] = result;
                break;
            }
            case READ_LB:
                // LB is always 0 in theory so we don't really care about LB
                // Just here in case logging is needed
                break;
            default:
                // Log any unimplemented
                {
                    can_frame_t smbus_response = {{0}, 8, 0x044};
                    smbus_response.data[0] = smbus_cmd->controller_addr;
                    smbus_response.data[1] = smbus_cmd->command.cmnd_code;
                    smbus_response.data[2] = result;
                    tx_queue_send(&can_tx_queue, &smbus_response, TX_NO_WAIT);
                    break;
                }
            }
        }
    }
}