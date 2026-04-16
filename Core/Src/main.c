/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "CO_app_STM32.h"
#include "OD.h"
#include "smbus.h"
#include "stm32_PMBUS_stack.h"
#include "stm32_SMBUS_stack.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

FDCAN_HandleTypeDef hfdcan1;

I2C_HandleTypeDef hi2c3;
SMBUS_HandleTypeDef hsmbus4;

TIM_HandleTypeDef htim17;

/* Definitions for smbus_task */
osThreadId_t smbus_taskHandle;
const osThreadAttr_t smbus_task_attributes = {
  .name = "smbus_task",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for read_fans */
osThreadId_t read_fansHandle;
const osThreadAttr_t read_fans_attributes = {
  .name = "read_fans",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for canopen_task */
osThreadId_t canopen_taskHandle;
const osThreadAttr_t canopen_task_attributes = {
  .name = "canopen_task",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 4
};
/* USER CODE BEGIN PV */
// Need to add each pin as they're created won't update automatically
HSEN_Pin_t hsen_pins[7] = {
    {HSEN1_GPIO_Port, HSEN1_Pin},
    {HSEN2_GPIO_Port, HSEN2_Pin},
    {HSEN3_GPIO_Port, HSEN3_Pin},
    {HSEN4_GPIO_Port, HSEN4_Pin},
    {USER_R_GPIO_Port, USER_R_Pin},
    {USER_G_GPIO_Port, USER_G_Pin},
    {USER_B_GPIO_Port, USER_B_Pin},
};
SMBUS_StackHandleTypeDef context1;
osMessageQueueId_t cmd_queue;
osMessageQueueId_t result_queue;
osSemaphoreId_t    smbus_done;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_I2C3_Init(void);
static void MX_I2C4_SMBUS_Init(void);
static void MX_TIM17_Init(void);
void smbus_task_start(void *argument);
void read_fan_start(void *argument);
void canopen_task_start(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void write_to_pin(int hsen_num, bool state)
{
    int index = hsen_num - 1;
    if (index < sizeof(hsen_pins) / sizeof(hsen_pins[0]) && index >= 0) {
        HAL_GPIO_WritePin(hsen_pins[index].port, hsen_pins[index].pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_FDCAN1_Init();
  MX_I2C3_Init();
  MX_I2C4_SMBUS_Init();
  MX_TIM17_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of smbus_task */
  smbus_taskHandle = osThreadNew(smbus_task_start, NULL, &smbus_task_attributes);

  /* creation of read_fans */
  // read_fansHandle = osThreadNew(read_fan_start, NULL, &read_fans_attributes);

  /* creation of canopen_task */
  canopen_taskHandle = osThreadNew(canopen_task_start, NULL, &canopen_task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // blank because FREERTOS
      // canopen_app_process();
      // OD_set_u32(OD_find(OD, 0x6000), 0x00, 123, false); // The correct way
      // OD_PERSIST_COMM.x2000_fan_1_r = 1; // The simple way

     // high-side driver tPDOs
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.GainCompensation = 0;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = ENABLE;
  hfdcan1.Init.TransmitPause = ENABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 32;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 13;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 32;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 13;
  hfdcan1.Init.DataTimeSeg2 = 2;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x10B17DB5;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_SMBUS_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hsmbus4.Instance = I2C4;
  hsmbus4.Init.Timing = 0x10B17DB5;
  hsmbus4.Init.AnalogFilter = SMBUS_ANALOGFILTER_ENABLE;
  hsmbus4.Init.OwnAddress1 = 2;
  hsmbus4.Init.AddressingMode = SMBUS_ADDRESSINGMODE_7BIT;
  hsmbus4.Init.DualAddressMode = SMBUS_DUALADDRESS_DISABLE;
  hsmbus4.Init.OwnAddress2 = 0;
  hsmbus4.Init.OwnAddress2Masks = SMBUS_OA2_NOMASK;
  hsmbus4.Init.GeneralCallMode = SMBUS_GENERALCALL_DISABLE;
  hsmbus4.Init.NoStretchMode = SMBUS_NOSTRETCH_DISABLE;
  hsmbus4.Init.PacketErrorCheckMode = SMBUS_PEC_DISABLE;
  hsmbus4.Init.PeripheralMode = SMBUS_PERIPHERAL_MODE_SMBUS_HOST;
  hsmbus4.Init.SMBusTimeout = 0x0000830D;
  if (HAL_SMBUS_Init(&hsmbus4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */
    // Context config
    context1.Device = &hsmbus4;
    context1.CMD_table = (st_command_t *) &PMBUS_COMMANDS_TAB[0];
    context1.CMD_tableSize = PMBUS_COMMANDS_TAB_SIZE;
    context1.SRByte = 0x55U;
    context1.CurrentCommand = NULL;
    STACK_SMBUS_Init(&context1);
  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(void)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 63;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 1000;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, USER_R_Pin|USER_G_Pin|USER_B_Pin|HSEN3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, HSEN1_Pin|HSDIAG1_Pin|HSDIAG2_Pin|HSEN2_Pin
                          |TERM_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, HSDIAG3_Pin|HSDIAG4_Pin|HSEN4_Pin|I_AUX2_Pin
                          |I_AUX1_Pin|I_AUX2B15_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : USER_R_Pin USER_G_Pin USER_B_Pin HSEN3_Pin */
  GPIO_InitStruct.Pin = USER_R_Pin|USER_G_Pin|USER_B_Pin|HSEN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : HSEN1_Pin HSDIAG1_Pin HSDIAG2_Pin HSEN2_Pin
                           TERM_EN_Pin */
  GPIO_InitStruct.Pin = HSEN1_Pin|HSDIAG1_Pin|HSDIAG2_Pin|HSEN2_Pin
                          |TERM_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : HSDIAG3_Pin HSDIAG4_Pin HSEN4_Pin I_AUX2_Pin
                           I_AUX1_Pin I_AUX2B15_Pin */
  GPIO_InitStruct.Pin = HSDIAG3_Pin|HSDIAG4_Pin|HSEN4_Pin|I_AUX2_Pin
                          |I_AUX1_Pin|I_AUX2B15_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : F_ALERT2_Pin F_ALERT1_Pin */
  GPIO_InitStruct.Pin = F_ALERT2_Pin|F_ALERT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : CAN_FLT_Pin */
  GPIO_InitStruct.Pin = CAN_FLT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CAN_FLT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/* USER CODE BEGIN Header_smbus_task_start */
/**
  * @brief  Function implementing the smbus_task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_smbus_task_start */
void smbus_task_start(void *argument)
{
  /* USER CODE BEGIN 5 */
    // smbus_cmd_t cmd;
    for (;;) {
        // osMessageQueueGet(cmd_queue, &cmd, NULL, osWaitForever);
        //
        // if (cmd.type == CMD_FAN_WRITE) {
        //     const fan_config_t *cfg = &fan_configs[cmd.fan_num];
        //     const uint16_t count = 7864320U / cmd.rpm;
        //     uint8_t buf_tx[] = {
        //         cfg->low_addr,
        //         (uint8_t)((count & 0x1FU) << 3U),
        //         cfg->high_addr,
        //         (uint8_t)(count >> 5U),
        //     };
        //     HAL_SMBUS_Master_Transmit_IT(&hsmbus4, cfg->address << 1U, buf_tx, 4U, SMBUS_LAST_FRAME_NO_PEC);
        //     while (HAL_SMBUS_GetState(&hsmbus4) != HAL_SMBUS_STATE_READY)
        //     {
        //         osDelay(1);
        //     }
        //     osSemaphoreAcquire(smbus_done, 50); // timeout guards against hung bus
        // }
        // else { // CMD_FAN_READ
        //     const fan_config_t *cfg = &fan_configs[cmd.fan_num];
        //
        //     // Send hb register address first and ctrler will get matching lb
        //     uint8_t reg = cfg->tach_high_addr;
        //     HAL_SMBUS_Master_Transmit_IT(&hsmbus4, cfg->address << 1U, &reg, 1U, SMBUS_LAST_FRAME_NO_PEC);
        //     while (HAL_SMBUS_GetState(&hsmbus4) != HAL_SMBUS_STATE_READY)
        //     {
        //         osDelay(1);
        //     }
        //
        //     // Read hb and lb
        //     uint8_t rx[2] = {0U, 0U};
        //     HAL_SMBUS_Master_Receive_IT(&hsmbus4, cfg->address << 1U, rx, 2U, SMBUS_LAST_FRAME_NO_PEC);
        //     while (HAL_SMBUS_GetState(&hsmbus4) != HAL_SMBUS_STATE_READY)
        //     {
        //         osDelay(1);
        //     }
        //
        //     // Convert and send result
        //     const uint16_t count = (uint16_t)(((uint16_t)rx[0] << 5U) | (rx[1] >> 3U));
        //     const smbus_result_t result = {
        //         .fan_num = cmd.fan_num,
        //         .rpm     = (uint16_t)(7864320U / count),
        //     };
        //     osMessageQueuePut(result_queue, &result, 0U, 0U);
        // }
        HAL_GPIO_WritePin(USER_R_GPIO_Port, USER_R_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(USER_G_GPIO_Port, USER_G_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(USER_B_GPIO_Port, USER_B_Pin, GPIO_PIN_RESET);
        osDelay(200);
        HAL_GPIO_WritePin(USER_R_GPIO_Port, USER_R_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(USER_G_GPIO_Port, USER_G_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(USER_B_GPIO_Port, USER_B_Pin, GPIO_PIN_SET);
        osDelay(200);
    }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_read_fan_start */
/**
* @brief Function implementing the read_fans thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_read_fan_start */
void read_fan_start(void *argument)
{
  /* USER CODE BEGIN read_fan_start */
    /* Infinite loop */
    for(;;) {
    }
  /* USER CODE END read_fan_start */
}

/* USER CODE BEGIN Header_canopen_task_start */
/**
* @brief Function implementing the canopen_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_canopen_task_start */
void canopen_task_start(void *argument)
{
  /* USER CODE BEGIN canopen_task_start */
    uint8_t *piobuf = NULL;

    CANopenNodeSTM32 canopenNodeSTM32;
    canopenNodeSTM32.CANHandle = &hfdcan1;
    canopenNodeSTM32.HWInitFunction = MX_FDCAN1_Init;
    canopenNodeSTM32.timerHandle = &htim17;
    canopenNodeSTM32.desiredNodeID = 29; // ADD THIS TO COMID TO GET TPDO ID
    canopenNodeSTM32.baudrate = 125;

    const uint16_t k_controller_addrs[] = {0x5C, 0x5E};
    const uint8_t k_fan_settings_regs[] = {0x32U, 0x42U, 0x52U};
    const uint8_t k_fan_target_lb_regs[] = {0x3CU, 0x4CU, 0x5CU};
    const uint8_t k_fan_target_hb_regs[] = {0x3DU, 0x4DU, 0x5DU};

    for (uint8_t i = 0; i < (uint8_t)(sizeof(k_controller_addrs)/sizeof(k_controller_addrs[0])); i++) {
        // context1 is defined after hsmbus4 init
        piobuf = STACK_SMBUS_GetBuffer(&context1);
        st_command_t WRITE_PUSH_PULL = {0x2B, WRITE, 2, 0};
        piobuf[0] = 0x7; // Data that you're writing
        STACK_SMBUS_HostCommand(&context1, &WRITE_PUSH_PULL, k_controller_addrs[i], WRITE);
        while (STACK_SMBUS_IsBusy(&context1)) {osDelay(1);}
        for (uint8_t j = 0; j < sizeof(k_fan_settings_regs)/sizeof(k_fan_settings_regs[0]); j++) {
            piobuf = STACK_SMBUS_GetBuffer(&context1);
            // FSC is the closed loop mode
            st_command_t WRITE_FSC_SETTINGS = {k_fan_settings_regs[j], WRITE, 2, 0};
            piobuf[0] = 0x80U | 0x2BU;
            STACK_SMBUS_HostCommand(&context1, &WRITE_FSC_SETTINGS, k_controller_addrs[i], WRITE);
            while (STACK_SMBUS_IsBusy(&context1)) {osDelay(1);}
        }
        const uint16_t rpm = 2000U;
        const uint16_t tach = 3932160U * 2 / rpm;
        for (uint8_t j = 0; j < sizeof(k_fan_settings_regs)/sizeof(k_fan_settings_regs[0]); j++) {
            piobuf = STACK_SMBUS_GetBuffer(&context1);
            st_command_t TEST_FAN_SPEED = {k_fan_target_lb_regs[j], WRITE, 2, 0};
            piobuf[0] = (uint8_t)((tach & 0x1FU) << 3);
            STACK_SMBUS_HostCommand(&context1, &TEST_FAN_SPEED, k_controller_addrs[i], WRITE);
            while (STACK_SMBUS_IsBusy(&context1)) {osDelay(1);}
        }
        for (uint8_t j = 0; j < sizeof(k_fan_settings_regs)/sizeof(k_fan_settings_regs[0]); j++) {
            piobuf = STACK_SMBUS_GetBuffer(&context1);
            st_command_t TEST_FAN_SPEED = {k_fan_target_hb_regs[j], WRITE, 2, 0};
            piobuf[0] = (uint8_t)(tach >> 5);
            STACK_SMBUS_HostCommand(&context1, &TEST_FAN_SPEED, k_controller_addrs[i], WRITE);
            while (STACK_SMBUS_IsBusy(&context1)) {osDelay(1);}
        }
    }



    canopen_app_init(&canopenNodeSTM32);
    HAL_GPIO_WritePin(TERM_EN_GPIO_Port, TERM_EN_Pin, GPIO_PIN_SET);
    /* Infinite loop */
  for(;;)
  {
      canopen_app_process();
      // HAL_GPIO_WritePin(TERM_EN_GPIO_Port, TERM_EN_Pin, OD_PERSIST_COMM.x2301_thermal_en ? GPIO_PIN_SET : GPIO_PIN_RESET );
      vTaskDelay(pdMS_TO_TICKS(1));
  }
  /* USER CODE END canopen_task_start */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

    if (htim->Instance == TIM17) {
        canopen_app_interrupt();
    }
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
