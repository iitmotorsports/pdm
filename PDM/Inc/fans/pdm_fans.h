#ifndef PDM_PDM_FANS_H
#define PDM_PDM_FANS_H
#include "stm32_PMBUS_stack.h"
#include "stm32_SMBUS_stack.h"
#include "tx_api.h"
#include <stdbool.h>
#include "can/pdm_can.h"

typedef struct {
    uint8_t data;
    uint8_t controller_addr;
    st_command_t command;
    bool write;
} __attribute__((aligned(4))) smbus_cmd_t;

extern SMBUS_HandleTypeDef hsmbus4;
extern TX_QUEUE smbus_queue;
extern const uint8_t k_controller_addrs[2];
extern const uint8_t k_fan_settings_regs[3];
extern uint8_t fan_speeds[6];
extern TX_QUEUE can_tx_queue;

void pdm_fan_can_callback_recv(uint8_t rx_data[], uint8_t rx_data_len);
void pdm_fan_can_callback_send();
void pdm_fans_set_push_pull(bool is_push_pull, uint8_t controller_addr);
void pdm_fans_set_fsc(bool is_fsc, uint8_t controller_addr,
                      uint8_t fan_reg_addr);
UINT pdm_fans_tx_speed_tx_create(TX_BYTE_POOL *byte_pool);
VOID pdm_fans_tx_speed_thread_entry(ULONG thread_input);
#endif // PDM_PDM_FANS_H
