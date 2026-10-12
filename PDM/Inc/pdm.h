//
// Created by Jacob on 9/14/2026.
//

#ifndef PDM_PDM_H
#define PDM_PDM_H
#include "main.h"
#include "tx_api.h"

typedef struct {
     uint16_t STACK_SIZE;
    const int PRIORITY;
    TX_THREAD thread;
    void *stack;
} tx_thread_props;

UINT pdm_threads_create(TX_BYTE_POOL *byte_pool);
void pdm_main_loop();
void pdm_init();
#endif // PDM_PDM_H