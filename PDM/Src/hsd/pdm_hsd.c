#include "hsd/pdm_hsd.h"

#define PDM_CAN_HSD_STACK_SIZE 512
#define PDM_CAN_HSD_PRIORITY 20
static TX_THREAD pdm_can_hsd_thread;
static uint8_t pdm_can_hsd_stack[PDM_CAN_HSD_STACK_SIZE];

const GPIO_Pin_t k_hsd_pins[4] = {
    {HSEN1_GPIO_Port, HSEN1_Pin},
    {HSEN2_GPIO_Port, HSEN2_Pin},
    {HSEN3_GPIO_Port, HSEN3_Pin},
    {HSEN4_GPIO_Port, HSEN4_Pin},
};

static void hsd_write(uint8_t i, uint8_t set, uint8_t state) {
    if (set) {
        HAL_GPIO_WritePin(k_hsd_pins[i].port, k_hsd_pins[i].pin,
                          state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

static void hsd_read(uint8_t i, struct hawkdrive_can_pdm_hsd_status_t *src_p) {
    uint8_t value =
        HAL_GPIO_ReadPin(k_hsd_pins[i].port, k_hsd_pins[i].pin) == GPIO_PIN_SET;
    switch (i) {
    case 0:
        src_p->hsd1_on = hawkdrive_can_pdm_hsd_status_hsd1_on_encode(value);
        break;
    case 1:
        src_p->hsd2_on = hawkdrive_can_pdm_hsd_status_hsd2_on_encode(value);
        break;
    case 2:
        src_p->hsd3_on = hawkdrive_can_pdm_hsd_status_hsd3_on_encode(value);
        break;
    case 3:
        src_p->hsd4_on = hawkdrive_can_pdm_hsd_status_hsd4_on_encode(value);
        break;
    default:
        Error_Handler();
        break; // Only happens if size of k_hsd_pins changes
    }
}

void pdm_can_hsd_callback_recv(uint8_t rx_data[], uint8_t rx_data_len) {
    struct hawkdrive_can_pcm_hsd_command_t hsd_cmd;
    hawkdrive_can_pcm_hsd_command_init(&hsd_cmd);
    hawkdrive_can_pcm_hsd_command_unpack(&hsd_cmd, rx_data, rx_data_len);

    hsd_write(0,
              (uint8_t)hawkdrive_can_pcm_hsd_command_hsd1_set_decode(
                  hsd_cmd.hsd1_set),
              (uint8_t)hawkdrive_can_pcm_hsd_command_hsd1_state_decode(
                  hsd_cmd.hsd1_state));
    hsd_write(1,
              (uint8_t)hawkdrive_can_pcm_hsd_command_hsd2_set_decode(
                  hsd_cmd.hsd2_set),
              (uint8_t)hawkdrive_can_pcm_hsd_command_hsd2_state_decode(
                  hsd_cmd.hsd2_state));
    hsd_write(2,
              (uint8_t)hawkdrive_can_pcm_hsd_command_hsd3_set_decode(
                  hsd_cmd.hsd3_set),
              (uint8_t)hawkdrive_can_pcm_hsd_command_hsd3_state_decode(
                  hsd_cmd.hsd3_state));
    hsd_write(3,
              (uint8_t)hawkdrive_can_pcm_hsd_command_hsd4_set_decode(
                  hsd_cmd.hsd4_set),
              (uint8_t)hawkdrive_can_pcm_hsd_command_hsd4_state_decode(
                  hsd_cmd.hsd4_state));
}

VOID pdm_can_hsd_thread_entry() {
    while (1) {
        uint8_t hsd_status_data[5];
        struct hawkdrive_can_pdm_hsd_status_t hsd_status;
        hawkdrive_can_pdm_hsd_status_init(&hsd_status);
        for (int i = 0; i < sizeof(k_hsd_pins) / sizeof(k_hsd_pins[0]); i++) {
            hsd_read(i, &hsd_status);
        }
        hawkdrive_can_pdm_hsd_status_pack(hsd_status_data, &hsd_status,
                                          sizeof(hsd_status_data) /
                                              sizeof(hsd_status_data[0]));
        can_frame_t can_frame = {{0}, HAWKDRIVE_CAN_PDM_HSD_STATUS_LENGTH, HAWKDRIVE_CAN_PDM_HSD_STATUS_FRAME_ID};
        memcpy(can_frame.data, hsd_status_data, sizeof(hsd_status_data));
        tx_queue_send(&can_tx_queue, &can_frame, TX_NO_WAIT);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}

UINT pdm_can_hsd_create(TX_BYTE_POOL *byte_pool) {
    UINT status = tx_thread_create(
        &pdm_can_hsd_thread, "pdm_can_hsd", pdm_can_hsd_thread_entry, 0,
        pdm_can_hsd_stack, PDM_CAN_HSD_STACK_SIZE, PDM_CAN_HSD_PRIORITY,
        PDM_CAN_HSD_PRIORITY, 1, TX_AUTO_START);
    return status;
}