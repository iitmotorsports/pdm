#pragma once
#include "cmsis_os.h"

extern osMessageQueueId_t cmd_queue;
extern osMessageQueueId_t result_queue;

extern uint16_t MAX_FAN_SPEED;

typedef enum { CMD_FAN_WRITE, CMD_FAN_READ } smbus_cmd_type_t;


uint8_t rpm_to_byte(uint16_t rpm);
uint16_t byte_to_rpm(uint8_t value);