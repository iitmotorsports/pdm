
#ifndef PDM_PDM_FANS_EMC2303_H
#define PDM_PDM_FANS_EMC2303_H
#include "stm32_SMBUS_stack.h"
#include "tx_api.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t data;
    uint8_t controller_addr;
    st_command_t command;
    bool write;
} __attribute__((aligned(4))) smbus_cmd_t;

typedef struct emc2303 emc2303_t;
struct emc2303 {
    uint8_t ctrler_addr;
    TX_QUEUE *smbus_queue;
    void (*write_speed)(emc2303_t *self, uint8_t tach, uint8_t fan_num);
    void (*read_speed)(emc2303_t *self, uint8_t fan_num);
    void (*set_fsc)(emc2303_t *self, bool is_fsc, uint8_t fan_num);
    void (*set_push_pull)(emc2303_t *self, bool is_push_pull);
};

void pdm_fans_emc2303_init(emc2303_t *dev, uint8_t ctrler_addr,
                           TX_QUEUE *smbus_queue);
#endif // PDM_PDM_FANS_EMC2303_H
