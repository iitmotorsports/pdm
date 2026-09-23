#ifndef PDM_PDM_CAN_H
#define PDM_PDM_CAN_H

#include "main.h"
#include <string.h>

void pdm_can_init();

void pdm_can_tx(const uint8_t tx_data[], uint8_t tx_data_len, uint16_t tx_id);
void pdm_can_rx(uint8_t rx_data[], uint8_t rx_data_len, uint16_t rx_id);


#endif //PDM_PDM_CAN_H
