#pragma once
#include "stm32_SMBUS_stack.h"

extern uint16_t MAX_FAN_SPEED;

typedef struct {
    uint8_t data;
    uint8_t controller_addr;
    st_command_t command;
    bool write;
} smbus_cmd_t;

uint8_t rpm_to_byte(uint16_t rpm);
uint16_t byte_to_rpm(uint8_t value);

typedef struct {
    uint16_t tach;
    uint8_t fan_num;
} fan_tach_t;
