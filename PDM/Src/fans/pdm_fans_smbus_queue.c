#include "fans/pdm_fans_smbus_queue.h"
#include "fans/pdm_fans.h"
#include "pdm.h"

#define PDM_SMBUS_QUEUE_STACK_SIZE 2048
#define PDM_SMBUS_QUEUE_PRIORITY 20

static TX_THREAD pdm_smbus_queue_thread;
static uint8_t pdm_smbus_queue_stack[PDM_SMBUS_QUEUE_STACK_SIZE];

#define SMBUS_QUEUE_DEPTH 16
#define SMBUS_QUEUE_OBJ_SIZE                                                   \
    ((sizeof(smbus_cmd_t) + sizeof(ULONG) - 1U) / sizeof(ULONG))

TX_QUEUE smbus_queue;
static ULONG smbus_queue_storage[SMBUS_QUEUE_DEPTH * SMBUS_QUEUE_OBJ_SIZE];

#define READ_HB 0xEU
#define READ_LB 0xFU

#define TIMEOUT (TX_TIMER_TICKS_PER_SECOND / 10)

SMBUS_StackHandleTypeDef context1;

uint8_t fan_speeds[6];

void pdm_fans_smbus_queue_init() {
    HAL_GPIO_WritePin(THERM_EN_GPIO_Port, THERM_EN_Pin, GPIO_PIN_SET);
    context1.Device = &hsmbus4;
    context1.CMD_table = (st_command_t *)&PMBUS_COMMANDS_TAB[0];
    context1.CMD_tableSize = PMBUS_COMMANDS_TAB_SIZE;
    context1.SRByte = 0x55U;
    context1.CurrentCommand = NULL;
    STACK_SMBUS_Init(&context1);
}

VOID pdm_smbus_queue_thread_entry(ULONG thread_input) {
    smbus_cmd_t smbus_cmd;
    uint8_t *piobuf = NULL;
    while (1) {
        UINT status =
            tx_queue_receive(&smbus_queue, &smbus_cmd, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS) {
            Error_Handler();
        }
        piobuf = STACK_SMBUS_GetBuffer(&context1);
        piobuf[0] = smbus_cmd.data;
        STACK_SMBUS_HostCommand(&context1, &smbus_cmd.command,
                                smbus_cmd.controller_addr,
                                smbus_cmd.write ? WRITE : READ);
        uint8_t elapsed = 0;
        while (STACK_SMBUS_IsBusy(&context1) && elapsed < TIMEOUT) {
            elapsed++;
            tx_thread_sleep(1);
        }
        if (elapsed >= TIMEOUT) {
            can_frame_t error_can = {
                {'s', 'm', 'b', ' ', 'e', 'r', 'r'}, 7, SMBUS_MISC};
            tx_queue_send(&can_tx_queue, &error_can, TX_NO_WAIT);
            continue;
        }
        if (!smbus_cmd.write) {
            const uint8_t result = piobuf[0];
            // 1. Check if it's a fan write
            // 2. Determine fan #
            // 3. Write speed to corresponding array slot
            //  4. If not a fan read be very confused
            switch (smbus_cmd.command.cmnd_code & 0xF) {
            case READ_HB: {
                uint8_t fan_offset =
                    smbus_cmd.controller_addr == k_controller_addrs[0] ? 0 : 3;
                uint8_t nibble = (smbus_cmd.command.cmnd_code & 0xF0U) >> 4U;
                if (nibble < 0x3U) {
                    Error_Handler();
                }
                uint8_t fan_num = nibble - 0x3U;
                uint8_t fan_index = fan_offset + fan_num;
                if (fan_index < (sizeof(fan_speeds) / sizeof(fan_speeds[0]))) {
                    fan_speeds[fan_index] = result;
                } else {
                    Error_Handler();
                }
                break;
            }
            case READ_LB:
                // LB is always 0 in theory so we don't really care about LB
                // Just here in case logging is needed
                break;
            default:
                // Log any unimplemented
                {
                    can_frame_t smbus_response = {{0}, 8, SMBUS_MISC};
                    smbus_response.data[0] = smbus_cmd.controller_addr;
                    smbus_response.data[1] = smbus_cmd.command.cmnd_code;
                    smbus_response.data[2] = result;
                    tx_queue_send(&can_tx_queue, &smbus_response, TX_NO_WAIT);
                    break;
                }
            }
        }
    }
}

UINT pdm_smbus_queue_create(TX_BYTE_POOL *byte_pool) {
    UINT status = tx_thread_create(
        &pdm_smbus_queue_thread, "pdm_smbus", pdm_smbus_queue_thread_entry, 0,
        pdm_smbus_queue_stack, PDM_SMBUS_QUEUE_STACK_SIZE,
        PDM_SMBUS_QUEUE_PRIORITY, PDM_SMBUS_QUEUE_PRIORITY, 1, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        return status;
    }

    status = tx_queue_create(&smbus_queue, "smbus_queue", SMBUS_QUEUE_OBJ_SIZE,
                             smbus_queue_storage, sizeof(smbus_queue_storage));
    return status;
}
