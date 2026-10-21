
#ifndef PDM_PDM_CAN_HEARTBEAT_H
#define PDM_PDM_CAN_HEARTBEAT_H

#include "can/pdm_can.h"
#include "main.h"
#include "tx_api.h"

extern TX_QUEUE can_tx_queue;
VOID pdm_can_heartbeat_thread_entry(ULONG thread_input);

#endif // PDM_PDM_CAN_HEARTBEAT_H
