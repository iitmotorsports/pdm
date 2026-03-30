#pragma once
#include "cmsis_os.h"

extern osMessageQueueId_t cmd_queue;
extern osMessageQueueId_t result_queue;

extern uint16_t MAX_FAN_SPEED;

typedef enum { CMD_FAN_WRITE, CMD_FAN_READ } smbus_cmd_type_t;

typedef struct {
    smbus_cmd_type_t type;
    uint8_t          fan_num;         // 0-5
    uint16_t         rpm;             // for writes
} smbus_cmd_t;

typedef struct {
    uint8_t  fan_num;
    uint16_t rpm;
} smbus_result_t;

typedef struct {
    uint8_t fan_num;
    uint8_t address;
    uint8_t low_addr;
    uint8_t high_addr;
    uint8_t tach_high_addr;
    uint8_t *od_value;
} fan_config_t;

extern fan_config_t fan_configs[6];

uint8_t rpm_to_byte(uint16_t rpm);
uint16_t byte_to_rpm(uint8_t value);