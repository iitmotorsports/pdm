/*
 * CANopen main program file.
 *
 * This file is a template for other microcontrollers.
 *
 * @file        main_generic.c
 * @author      Hamed Jafarzadeh 	2022
 * 				Janez Paternoster	2021
 * @copyright   2021 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "CO_app_STM32.h"
#include "CANopen.h"
#include "main.h"
#include <stdio.h>
#include <inttypes.h>

#include "CO_storageBlank.h"
#include "OD.h"

CANopenNodeSTM32*
    canopenNodeSTM32; // It will be set by canopen_app_init and will be used across app to get access to CANOpen objects

/* Printf function of CanOpen app */
#define log_printf(macropar_message, ...) printf(macropar_message, ## __VA_ARGS__)

/* default values for CO_CANopenInit() */
#define NMT_CONTROL                                                                                                    \
    (CO_NMT_STARTUP_TO_OPERATIONAL                                                                                      \
    | CO_NMT_ERR_ON_ERR_REG | CO_ERR_REG_GENERIC_ERR | CO_ERR_REG_COMMUNICATION)
#define FIRST_HB_TIME        500
#define SDO_SRV_TIMEOUT_TIME 1000
#define SDO_CLI_TIMEOUT_TIME 500
#define SDO_CLI_BLOCK        false
#define OD_STATUS_BITS       NULL

/* Global variables and objects */
CO_t* CO = NULL; /* CANopen object */

// Global variables
uint32_t time_old, time_current;
CO_ReturnError_t err;

// External references
extern SMBUS_HandleTypeDef hsmbus4;

// Constants
static const uint16_t MAX_FAN_SPEED = 5000; // FIXME: Replace with actual fan speed

// Helper functions

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

typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint8_t      *od_value;
} gpio_od_config_t;

static  gpio_od_config_t gpio_configs[4];

static ODR_t gpio_callback(OD_stream_t *stream, const void *buf, const OD_size_t size, OD_size_t *countWritten) {
    const ODR_t result = OD_writeOriginal(stream, buf, size, countWritten);
    if (result == ODR_OK) {
        const gpio_od_config_t *cfg = stream -> object;
        HAL_GPIO_WritePin(cfg -> port, cfg -> pin, *cfg -> od_value ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    return result;
}


static OD_extension_t gpio_extensions[sizeof(gpio_configs) / sizeof(gpio_configs[0])];

typedef struct {
    uint16_t controller_addr;
    uint8_t  tx_buf[4];
    bool pending;
} smbus_fan_job_t;

static smbus_fan_job_t pending_fan_jobs[6];

// FIXME: For now this is how I'm going to do this but there has to be a better way
void smbus_queue_process(void)
{
    if (HAL_SMBUS_GetState(&hsmbus4) != HAL_SMBUS_STATE_READY)
    {
        return;
    }

    for (size_t i = 0U; i < 6U; i++)
    {
        if (pending_fan_jobs[i].pending)
        {
            pending_fan_jobs[i].pending = false;
            HAL_SMBUS_Master_Transmit_IT(&hsmbus4, pending_fan_jobs[i].controller_addr, pending_fan_jobs[i].tx_buf, 4U,SMBUS_LAST_FRAME_NO_PEC);
            return;
        }
    }
}

typedef struct {
    uint8_t fan_num;
    uint8_t address;
    uint8_t low_addr;
    uint8_t high_addr;
    uint8_t *od_value;
} fan_config_t;

static fan_config_t fan_configs[6];

static ODR_t fan_callback(OD_stream_t *stream, const void *buf, const OD_size_t size, OD_size_t *countWritten) {
    const ODR_t result = OD_writeOriginal(stream, buf, size, countWritten);
    if (result == ODR_OK) {
        const fan_config_t *cfg = stream->object;
        const uint16_t rpm = byte_to_rpm(*cfg->od_value);
        const uint16_t count = (uint16_t)(7864320U / rpm);
        const uint8_t low  = (uint8_t)((count & 0x1FU) << 3U); // Lower 5 bits [4:0] shifted in accordance with 5.17 in datasheet
        const uint8_t high = (uint8_t)(count >> 5U); // Upper 8 bits [12:5]

        pending_fan_jobs[cfg->fan_num-1] = (smbus_fan_job_t) {
            .controller_addr = cfg->address,
            .tx_buf = {
                cfg -> low_addr,
                low,
                cfg -> high_addr,
                high,
            },
            .pending = true
        };
    }
    return result;
}

static OD_extension_t fan_extensions[sizeof(fan_configs) / sizeof(fan_configs[0])];

/* This function will basically setup the CANopen node */
int
canopen_app_init(CANopenNodeSTM32* _canopenNodeSTM32) {
    // Keep a copy global reference of canOpenSTM32 Object
    canopenNodeSTM32 = _canopenNodeSTM32;

    OD_entry_t *gpio_entries[] = {
        OD_ENTRY_H2200,
        OD_ENTRY_H2201,
        OD_ENTRY_H2202,
        OD_ENTRY_H2203,
    };

    gpio_configs[0] = (gpio_od_config_t){HSEN1_GPIO_Port, HSEN1_Pin, &OD_PERSIST_COMM.x2200_hsd_1_w};
    gpio_configs[1] = (gpio_od_config_t){HSEN2_GPIO_Port, HSEN2_Pin, &OD_PERSIST_COMM.x2201_hsd_2_w};
    gpio_configs[2] = (gpio_od_config_t){HSEN3_GPIO_Port, HSEN3_Pin, &OD_PERSIST_COMM.x2202_hsd_3_w};
    gpio_configs[3] = (gpio_od_config_t){HSEN4_GPIO_Port, HSEN4_Pin, &OD_PERSIST_COMM.x2203_hsd_4_w};

    for(size_t i = 0; i < sizeof(gpio_entries) / sizeof(gpio_entries[0]); i++) {
        gpio_extensions[i].object = &gpio_configs[i];
        gpio_extensions[i].read = NULL;
        gpio_extensions[i].write = gpio_callback;
        OD_extension_init(gpio_entries[i], &gpio_extensions[i]);
    }


    // Enable RPM control for all fans on both controllers
    const uint8_t k_fan_config = 0x80U | 0x2BU;
    const uint8_t k_fan_config_regs[] = {0x32U, 0x42U, 0x52U};
    const uint16_t k_controller_addrs[] = {0x2EU << 1, 0x2FU << 1};

    for (size_t ctrl = 0U; ctrl < 2U; ctrl++)
    {
        for (size_t fan = 0U; fan < 3U; fan++)
        {
            uint8_t tx_buf[] = {k_fan_config_regs[fan], k_fan_config};
            HAL_SMBUS_Master_Transmit_IT(&hsmbus4, k_controller_addrs[ctrl], tx_buf,2U, SMBUS_LAST_FRAME_NO_PEC);
            while (HAL_SMBUS_GetState(&hsmbus4) != HAL_SMBUS_STATE_READY) {}
        }
    }

    OD_entry_t *fan_entries[] = {
        OD_ENTRY_H2100,
        OD_ENTRY_H2101,
        OD_ENTRY_H2102,
        OD_ENTRY_H2103,
        OD_ENTRY_H2104,
        OD_ENTRY_H2105,
    };

    fan_configs[0] = (fan_config_t) {1, 0x2EU, 0x3CU, 0x3DU, &OD_PERSIST_COMM.x2100_fan_1_w};
    fan_configs[1] = (fan_config_t) {2, 0x2EU, 0x4CU, 0x4DU, &OD_PERSIST_COMM.x2101_fan_2_w};
    fan_configs[2] = (fan_config_t) {3, 0x2EU, 0x5CU, 0x5DU, &OD_PERSIST_COMM.x2102_fan_3_w};
    fan_configs[3] = (fan_config_t) {4, 0x2FU, 0x3CU, 0x3DU, &OD_PERSIST_COMM.x2103_fan_4_w};
    fan_configs[4] = (fan_config_t) {5, 0x2FU, 0x4CU, 0x4DU, &OD_PERSIST_COMM.x2104_fan_5_w};
    fan_configs[5] = (fan_config_t) {6, 0x2FU, 0x5CU, 0x5DU, &OD_PERSIST_COMM.x2105_fan_6_w};

    for(size_t i = 0U; i < sizeof(fan_entries) / sizeof(fan_entries[0]); i++) {
        fan_extensions[i].object = &fan_configs[i];
        fan_extensions[i].read = NULL;
        fan_extensions[i].write = fan_callback;
        OD_extension_init(fan_entries[i], &fan_extensions[i]);
    }


#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
    static CO_storage_t storage;
    static CO_storage_entry_t storageEntries[] = {{.addr = &OD_PERSIST_COMM,
                                                   .len = sizeof(OD_PERSIST_COMM),
                                                   .subIndexOD = 2,
                                                   .attr = CO_storage_cmd | CO_storage_restore,
                                                   .addrNV = NULL}};
    uint8_t storageEntriesCount = sizeof(storageEntries) / sizeof(storageEntries[0]);
    uint32_t storageInitError = 0;
#endif

    /* Allocate memory */
    CO_config_t* config_ptr = NULL;
#ifdef CO_MULTIPLE_OD
    /* example usage of CO_MULTIPLE_OD (but still single OD here) */
    CO_config_t co_config = {0};
    OD_INIT_CONFIG(co_config); /* helper macro from OD.h */
    co_config.CNT_LEDS = 1;
    co_config.CNT_LSS_SLV = 1;
    config_ptr = &co_config;
#endif /* CO_MULTIPLE_OD */

    uint32_t heapMemoryUsed;
    CO = CO_new(config_ptr, &heapMemoryUsed);
    if (CO == NULL) {
        log_printf("Error: Can't allocate memory\n");
        return 1;
    } else {
        log_printf("Allocated %" PRIu32 " bytes for CANopen objects\n", heapMemoryUsed);
    }

    canopenNodeSTM32->canOpenStack = CO;

#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
    err = CO_storageBlank_init(&storage, CO->CANmodule, OD_ENTRY_H1010_storeParameters,
                               OD_ENTRY_H1011_restoreDefaultParameters, storageEntries, storageEntriesCount,
                               &storageInitError);

    if (err != CO_ERROR_NO && err != CO_ERROR_DATA_CORRUPT) {
        log_printf("Error: Storage %d\n", storageInitError);
        return 2;
    }
#endif

    canopen_app_resetCommunication();
    return 0;
}

int
canopen_app_resetCommunication() {
    /* CANopen communication reset - initialize CANopen objects *******************/
    log_printf("CANopenNode - Reset communication...\n");

    /* Wait rt_thread. */
    CO->CANmodule->CANnormal = false;

    /* Enter CAN configuration. */
    CO_CANsetConfigurationMode((void*)canopenNodeSTM32);
    CO_CANmodule_disable(CO->CANmodule);

    /* initialize CANopen */
    err = CO_CANinit(CO, canopenNodeSTM32, 0); // Bitrate for STM32 microcontroller is being set in MXCube Settings
    if (err != CO_ERROR_NO) {
        log_printf("Error: CAN initialization failed: %d\n", err);
        return 1;
    }

    CO_LSS_address_t lssAddress = {.identity = {.vendorID = OD_PERSIST_COMM.x1018_identity.vendor_ID,
                                                .productCode = OD_PERSIST_COMM.x1018_identity.productCode,
                                                .revisionNumber = OD_PERSIST_COMM.x1018_identity.revisionNumber,
                                                .serialNumber = OD_PERSIST_COMM.x1018_identity.serialNumber}};
    err = CO_LSSinit(CO, &lssAddress, &canopenNodeSTM32->desiredNodeID, &canopenNodeSTM32->baudrate);
    if (err != CO_ERROR_NO) {
        log_printf("Error: LSS slave initialization failed: %d\n", err);
        return 2;
    }

    canopenNodeSTM32->activeNodeID = canopenNodeSTM32->desiredNodeID;
    uint32_t errInfo = 0;

    err = CO_CANopenInit(CO,                   /* CANopen object */
                         NULL,                 /* alternate NMT */
                         NULL,                 /* alternate em */
                         OD,                   /* Object dictionary */
                         OD_STATUS_BITS,       /* Optional OD_statusBits */
                         NMT_CONTROL,          /* CO_NMT_control_t */
                         FIRST_HB_TIME,        /* firstHBTime_ms */
                         SDO_SRV_TIMEOUT_TIME, /* SDOserverTimeoutTime_ms */
                         SDO_CLI_TIMEOUT_TIME, /* SDOclientTimeoutTime_ms */
                         SDO_CLI_BLOCK,        /* SDOclientBlockTransfer */
                         canopenNodeSTM32->activeNodeID, &errInfo);
    if (err != CO_ERROR_NO && err != CO_ERROR_NODE_ID_UNCONFIGURED_LSS) {
        if (err == CO_ERROR_OD_PARAMETERS) {
            log_printf("Error: Object Dictionary entry 0x%" PRIx32 "\n", errInfo);
        } else {
            log_printf("Error: CANopen initialization failed: %d\n", err);
        }
        return 3;
    }

    err = CO_CANopenInitPDO(CO, CO->em, OD, canopenNodeSTM32->activeNodeID, &errInfo);
    if (err != CO_ERROR_NO && err != CO_ERROR_NODE_ID_UNCONFIGURED_LSS) {
        if (err == CO_ERROR_OD_PARAMETERS) {
            log_printf("Error: Object Dictionary entry 0x%" PRIx32 "\n", errInfo);
        } else {
            log_printf("Error: PDO initialization failed: %d\n", err);
        }
        return 4;
    }

    /* Configure Timer interrupt function for execution every 1 millisecond */
    HAL_TIM_Base_Start_IT(canopenNodeSTM32->timerHandle); //1ms interrupt

    /* Configure CAN transmit and receive interrupt */

    /* Configure CANopen callbacks, etc */
    if (!CO->nodeIdUnconfigured) {

#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
        if (storageInitError != 0) {
            CO_errorReport(CO->em, CO_EM_NON_VOLATILE_MEMORY, CO_EMC_HARDWARE, storageInitError);
        }
#endif
    } else {
        log_printf("CANopenNode - Node-id not initialized\n");
    }

    /* start CAN */
    CO_CANsetNormalMode(CO->CANmodule);

    log_printf("CANopenNode - Running...\n");
    fflush(stdout);
    time_old = time_current = HAL_GetTick();
    return 0;
}

void
canopen_app_process() {
    /* loop for normal program execution ******************************************/
    /* get time difference since last function call */
    smbus_queue_process();

    time_current = HAL_GetTick();

    if ((time_current - time_old) > 0) { // Make sure more than 1ms elapsed
        /* CANopen process */
        CO_NMT_reset_cmd_t reset_status;
        uint32_t timeDifference_us = (time_current - time_old) * 1000;
        time_old = time_current;
        reset_status = CO_process(CO, false, timeDifference_us, NULL);
        canopenNodeSTM32->outStatusLEDRed = CO_LED_RED(CO->LEDs, CO_LED_CANopen);
        canopenNodeSTM32->outStatusLEDGreen = CO_LED_GREEN(CO->LEDs, CO_LED_CANopen);

        if (reset_status == CO_RESET_COMM) {
            /* delete objects from memory */
        	HAL_TIM_Base_Stop_IT(canopenNodeSTM32->timerHandle);
            CO_CANsetConfigurationMode((void*)canopenNodeSTM32);
            CO_delete(CO);
            log_printf("CANopenNode Reset Communication request\n");
            canopen_app_init(canopenNodeSTM32); // Reset Communication routine
        } else if (reset_status == CO_RESET_APP) {
            log_printf("CANopenNode Device Reset\n");
            HAL_NVIC_SystemReset(); // Reset the STM32 Microcontroller
        }
    }
}

/* Thread function executes in constant intervals, this function can be called from FreeRTOS tasks or Timers ********/
void
canopen_app_interrupt(void) {
    CO_LOCK_OD(CO->CANmodule);
    if (!CO->nodeIdUnconfigured && CO->CANmodule->CANnormal) {
        bool_t syncWas = false;
        /* get time difference since last function call */
        uint32_t timeDifference_us = 1000; // 1ms second

#if (CO_CONFIG_SYNC) & CO_CONFIG_SYNC_ENABLE
        syncWas = CO_process_SYNC(CO, timeDifference_us, NULL);
#endif
#if (CO_CONFIG_PDO) & CO_CONFIG_RPDO_ENABLE
        CO_process_RPDO(CO, syncWas, timeDifference_us, NULL);
#endif
#if (CO_CONFIG_PDO) & CO_CONFIG_TPDO_ENABLE
        CO_process_TPDO(CO, syncWas, timeDifference_us, NULL);
#endif

        /* Further I/O or nonblocking application code may go here. */
    }
    CO_UNLOCK_OD(CO->CANmodule);
}
