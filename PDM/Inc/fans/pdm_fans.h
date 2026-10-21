#ifndef PDM_PDM_FANS_H
#define PDM_PDM_FANS_H
#include "stm32_PMBUS_stack.h"
#include "stm32_SMBUS_stack.h"
#include "tx_api.h"
#include <stdbool.h>

extern SMBUS_HandleTypeDef hsmbus4;
extern TX_QUEUE smbus_queue;

typedef struct {
    uint8_t data;
    uint8_t controller_addr;
    st_command_t command;
    bool write;
} __attribute__((aligned(4))) smbus_cmd_t;

extern const uint8_t k_controller_addrs[2];
extern const uint8_t k_fan_settings_regs[3];

void pdm_fans_init();
void pdm_fans_write_speed(uint8_t tach, uint8_t fan_num);
void pdm_fans_read_speed(uint8_t fan_num);
void pdm_fans_set_push_pull(bool is_push_pull, uint8_t controller_addr);
void pdm_fans_set_fsc(bool is_fsc, uint8_t controller_addr,
                      uint8_t fan_reg_addr);
#endif // PDM_PDM_FANS_H
