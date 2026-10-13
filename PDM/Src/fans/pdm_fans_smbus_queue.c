#include "fans/pdm_fans_smbus_queue.h"

#define SMBUS_QUEUE_DEPTH 12
#define SMBUS_QUEUE_OBJ_SIZE (sizeof(smbus_cmd_t)/sizeof(ULONG))

#define READ_HB 0xEU
#define READ_LB 0xFU

TX_QUEUE smbus_queue;
static ULONG smbus_queue_storage[SMBUS_QUEUE_DEPTH * SMBUS_QUEUE_OBJ_SIZE];

uint8_t fan_speeds[6];

void pdm_smbus_queue_init() {
    UINT status = tx_queue_create(&smbus_queue, "smbus_queue", SMBUS_QUEUE_OBJ_SIZE, smbus_queue_storage, sizeof(smbus_queue_storage));
    if (status != TX_SUCCESS) {
        Error_Handler();
    }
}

VOID pdm_smbus_queue_thread_entry(ULONG thread_input) {
    smbus_cmd_t* smbus_cmd = NULL;
    uint8_t *piobuf = NULL;
    while (1) {
        UINT status = tx_queue_receive(&smbus_queue, (smbus_cmd_t*) &smbus_cmd, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS) {
            Error_Handler();
        }
        piobuf = STACK_SMBUS_GetBuffer(&context1);
        piobuf[0] = smbus_cmd -> data;
        STACK_SMBUS_HostCommand(&context1, &smbus_cmd -> command, smbus_cmd -> controller_addr, smbus_cmd -> write ? WRITE : READ);
        while (STACK_SMBUS_IsBusy(&context1)) {tx_thread_sleep(1);}
        if (!smbus_cmd -> write) {
            const uint8_t result = piobuf[0];
            // 1. Check if its a fan write
            // 2. Determine fan #
            // 3. Write speed to corresponding array slot
            //  4. If not a fan read be very confused
            switch (smbus_cmd -> command.cmnd_code) {
                case READ_HB:
                    uint8_t fan_offset = smbus_cmd -> controller_addr == 0x5C ? 0 : 2;
                    uint8_t fan_num = ((smbus_cmd -> command.cmnd_code & 0xF0) >> 4) - 0x3U;
                    uint8_t fan_index = fan_offset + fan_num;
                    fan_speeds[fan_index] = result;
                    break;
                case READ_LB:
                    // LB is always 0 in theory so we don't really care about LB
                    // Just here in case logging is needed
                    break;
            }
        }
    }
}