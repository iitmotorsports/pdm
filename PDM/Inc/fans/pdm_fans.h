#ifndef PDM_PDM_FANS_H
#define PDM_PDM_FANS_H
#include "stm32_PMBUS_stack.h"
#include "stm32_SMBUS_stack.h"
#include "tx_api.h"
#include <stdbool.h>
#include "can/pdm_can.h"
#include "fans/pdm_fans_emc2303.h"


extern TX_QUEUE smbus_queue;
extern uint8_t fan_speeds[6];
extern TX_QUEUE can_tx_queue;

void pdm_fans_init();
void pdm_fan_can_callback_recv(uint8_t rx_data[], uint8_t rx_data_len);
void pdm_fan_can_callback_send();
UINT pdm_fans_tx_speed_tx_create(TX_BYTE_POOL *byte_pool);
VOID pdm_fans_tx_speed_thread_entry(ULONG thread_input);
#endif // PDM_PDM_FANS_H
