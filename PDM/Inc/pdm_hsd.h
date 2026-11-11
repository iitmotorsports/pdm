
#ifndef PDM_PDM_HSD_H
#define PDM_PDM_HSD_H
#include "hawkdrive_can.h"
#include "can/pdm_can.h"
#include "tx_api.h"

UINT pdm_can_hsd_create(TX_BYTE_POOL *byte_pool);
void pdm_can_hsd_callback_recv(uint8_t rx_data[], uint8_t rx_data_len);

#endif //PDM_PDM_HSD_H
