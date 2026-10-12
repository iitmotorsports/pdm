#include "fans/pdm_fans_smbus_queue.h"

#define SMBUS_QUEUE_DEPTH 12
#define READ_HB 0xEU
#define TACH_RPM_CONST 3932160
#define TACH_MULT 2 // Table 5.12

static TX_QUEUE smbus_queue;
static ULONG smbus_queue_storage[SMBUS_QUEUE_DEPTH];

uint8_t fan_speeds[6];

void pdm_smbus_queue_init() {
    UINT status = tx_queue_create(&smbus_queue, "smbus_queue", TX_1_ULONG, smbus_queue_storage, sizeof(smbus_queue_storage));
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
            const uint8_t read = piobuf[0];
            // Obtain the lb if the read was for fan tach
            if ((0x0FU & smbus_cmd -> command.cmnd_code) == READ_HB) {
                smbus_cmd -> command.cmnd_code += 0x01U; // Increment by 1 to get lb address
                piobuf = STACK_SMBUS_GetBuffer(&context1);
                piobuf[0] = smbus_cmd -> data;
                STACK_SMBUS_HostCommand(&context1, &smbus_cmd -> command, smbus_cmd -> controller_addr, smbus_cmd -> write ? WRITE : READ);
                while (STACK_SMBUS_IsBusy(&context1)) {tx_thread_sleep(1);}
                const uint8_t lb = piobuf[0];
                const uint16_t tach = ((uint16_t)read << 8 | lb) >> 3;
                const uint16_t rpm = TACH_RPM_CONST * TACH_MULT / tach;

                // Figure out fan number
                const uint8_t  ctrler_fan_num = (smbus_cmd -> command.cmnd_code >> 4) - 0x2U; // EXAMPLE: 0x30U >> 4 = 0x3U; 0x3U - 0x2U = 0x1U or fan1
                const uint8_t fan_num = (smbus_cmd -> controller_addr == 0x5CU ? 0 : 3) + (ctrler_fan_num);
                fan_speeds[fan_num - 1] = (uint8_t) (rpm >> 8);
            }
            else {
            }
        }
    }
}