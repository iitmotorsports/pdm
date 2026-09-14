#include "stm32g4xx_hal_gpio.h"
#include "main.h"

void pdm_init() {

}

void pdm_main_loop() {
    HAL_GPIO_WritePin(HSEN1_GPIO_Port, HSEN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HSEN2_GPIO_Port, HSEN2_Pin, GPIO_PIN_SET);
    HAL_Delay(250);
    HAL_GPIO_WritePin(HSEN1_GPIO_Port, HSEN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HSEN2_GPIO_Port, HSEN2_Pin, GPIO_PIN_RESET);
    HAL_Delay(250);
}