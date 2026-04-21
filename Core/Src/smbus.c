#include "stm32_SMBUS_stack.h"

#include <stdbool.h>
#include <stdint.h>

// Constants
uint16_t MAX_FAN_SPEED = 5500; // FIXME: Replace with actual fan speed

//Helper Functions
// Encode RPM to byte for CAN transmission
uint8_t rpm_to_byte(const uint16_t rpm)
{
    return (uint8_t)((rpm * 255U) / MAX_FAN_SPEED);
}

// Decode byte back to RPM
uint16_t byte_to_rpm(const uint8_t value)
{
    return (uint16_t)((value * MAX_FAN_SPEED) / 255U);
}
