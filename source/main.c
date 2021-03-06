/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @example examples/ble_peripheral/ble_app_buttonless_dfu
 *
 * @brief Secure DFU Buttonless Service Application main file.
 *
 * This file contains the source code for a sample application using the proprietary
 * Secure DFU Buttonless Service. This is a template application that can be modified
 * to your needs. To extend the functionality of this application, please find
 * locations where the comment "// YOUR_JOB:" is present and read the comments.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_dfu_ble_svci_bond_sharing.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_state.h"
#include "ble_dfu.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "fds.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"
#include "nrf_power.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_bootloader_info.h"

#include "m_ble.h"
#include "m_board.h"
#include "twi_manager.h"
#include "m_detection.h"
#include "app_scheduler.h"
#include "m_batt_meas.h"

#define DETECT_SERVICES_MAX             5

#define DETECT_SERVICE_DETECTION        0
#define DETECT_SERVICE_BATTERY          1

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define SCHED_MAX_EVENT_DATA_SIZE       MAX(APP_TIMER_SCHED_EVENT_DATA_SIZE, BLE_STACK_HANDLER_SCHED_EVT_SIZE) /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                60  /**< Maximum number of events in the scheduler queue. */

#define BATT_MEAS_INTERVAL_MS           5000 // Measurement interval [ms].

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                            /**< Handle of the current connection. */
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

static m_ble_service_handle_t  m_ble_service_handles[DETECT_SERVICES_MAX];


/**
 * @brief TWI master instance.
 *
 * Instance of TWI master driver that will be used for communication with simulated
 * EEPROM memory.
 */
static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(MASTER_TWI_INST);

/**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 *          untill the function returns true. When the function returns true, it means that the
 *          app is ready to reset to DFU mode.
 *
 * @param[in]   event   Power manager event.
 *
 * @retval  True if shutdown is allowed by this power manager handler, otherwise false.
 */
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            NRF_LOG_INFO("Power management wants to reset to DFU mode.");
            // YOUR_JOB: Get ready to reset into DFU mode
            //
            // If you aren't finished with any ongoing tasks, return "false" to
            // signal to the system that reset is impossible at this stage.
            //
            // Here is an example using a variable to delay resetting the device.
            //
            // if (!m_ready_for_reset)
            // {
            //      return false;
            // }
            // else
            //{
            //
            //    // Device ready to enter
            //    uint32_t err_code;
            //    err_code = sd_softdevice_disable();
            //    APP_ERROR_CHECK(err_code);
            //    err_code = app_timer_stop_all();
            //    APP_ERROR_CHECK(err_code);
            //}
            break;

        default:
            // YOUR_JOB: Implement any of the other events available from the power management module:
            //      -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
            //      -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
            //      -NRF_PWR_MGMT_EVT_PREPARE_RESET
            return true;
    }

    NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
    return true;
}

//lint -esym(528, m_app_shutdown_handler)
/**@brief Register application shutdown handler with priority 0.
 */
NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);


static void buttonless_dfu_sdh_state_observer(nrf_sdh_state_evt_t state, void * p_context)
{
    if (state == NRF_SDH_EVT_STATE_DISABLED)
    {
        // Softdevice was disabled before going into reset. Inform bootloader to skip CRC on next boot.
        nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);

        //Go to system off.
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    }
}

/* nrf_sdh state observer. */
NRF_SDH_STATE_OBSERVER(m_buttonless_dfu_state_obs, 0) =
{
    .handler = buttonless_dfu_sdh_state_observer,
};


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{

    APP_SCHED_INIT(APP_TIMER_SCHED_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    // Initialize timer module.
    uint32_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
       uint32_t err_code;
       err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
       APP_ERROR_CHECK(err_code); */
}


// /**@brief Function for starting timers.
//  */
// static void application_timers_start(void)
// {
//     /* YOUR_JOB: Start your timers. below is an example of how to start a timer.
//        uint32_t err_code;
//        err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
//        APP_ERROR_CHECK(err_code); */
// }


/**@brief Function for the Power manager.
 */
static void log_init(void)
{
    uint32_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

/**@brief Battery module data handler.
 */
static void m_batt_meas_handler(m_batt_meas_event_t const * p_batt_meas_event)
{
    NRF_LOG_INFO("Voltage: %d V, Charge: %d %%, Event type: %d \r\n",
                p_batt_meas_event->voltage_mv, p_batt_meas_event->level_percent, p_batt_meas_event->type);
   
    if (p_batt_meas_event != NULL)
    {
        if( p_batt_meas_event->type == M_BATT_MEAS_EVENT_LOW)
        {
            //uint32_t err_code;

            NRF_LOG_WARNING("Battery voltage low, shutting down Thingy. Connect USB to charge \r\n");
            NRF_LOG_FINAL_FLUSH();
            // Go to system-off mode (This function will not return; wakeup will cause a reset).
            // Not powering off for now since we're using single use batteries
            //err_code = sd_power_system_off();

            // #ifdef DEBUG
            //     if(!support_func_sys_halt_debug_enabled())
            //     {
            //         APP_ERROR_CHECK(err_code); // If not in debug mode, return the error and the system will reboot.
            //     }
            //     else
            //     {
            //         NRF_LOG_WARNING("Exec stopped, busy wait \r\n");
            //         NRF_LOG_FLUSH();
            //         while(true) // Only reachable when entering emulated system off.
            //         {
            //             // Infinte loop to ensure that code stops in debug mode.
            //         }
            //     }
            // #else
            //     APP_ERROR_CHECK(err_code);
            // #endif
        }
    }
}

/**@brief Function for handling BLE events.
 */
static void detect_ble_evt_handler(m_ble_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case detect_ble_evt_connected:
            //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN, "Thingy_ble_evt_connected \r\n");
            break;

        case detect_ble_evt_disconnected:
            //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_YELLOW, "Thingy_ble_evt_disconnected \r\n");
            NRF_LOG_FINAL_FLUSH();
            //nrf_delay_ms(5);
            //NVIC_SystemReset();
            break;

        case detect_ble_evt_timeout:
            //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_YELLOW, "Thingy_ble_evt_timeout \r\n");
            //sleep_mode_enter();
            //NVIC_SystemReset();
            break;
    }
}


/**@brief Function for initializaing the Detect.
 */
static void detect_init(void)
{
    uint32_t err_code;
    m_detection_init_t     det_params;
    m_ble_init_t           ble_params;
    batt_meas_init_t       batt_meas_init = BATT_MEAS_PARAM_CFG;

    /**@brief Initialize the TWI manager. */
    err_code = twi_manager_init(APP_IRQ_PRIORITY_HIGHEST);
    APP_ERROR_CHECK(err_code);

    /**@brief Initialize detection module. */
    det_params.p_twi_instance = &m_twi_master;
    err_code = m_detection_init(&m_ble_service_handles[DETECT_SERVICE_DETECTION],
                                  &det_params);
    APP_ERROR_CHECK(err_code);

    /**@brief Initialize the battery measurement. */
    batt_meas_init.evt_handler = m_batt_meas_handler;
    err_code = m_batt_meas_init(&m_ble_service_handles[DETECT_SERVICE_BATTERY], &batt_meas_init);
    APP_ERROR_CHECK(err_code);

    err_code = m_batt_meas_enable(BATT_MEAS_INTERVAL_MS);
    APP_ERROR_CHECK(err_code);

    /**@brief Initialize BLE handling module. */
    ble_params.evt_handler       = detect_ble_evt_handler;
    ble_params.p_service_handles = m_ble_service_handles;
    ble_params.service_num       = DETECT_SERVICES_MAX;

    /**@brief Initialize BLE handling module. */
    err_code = m_ble_init(&ble_params, &m_conn_handle, &m_advertising);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;

    // Initialize.
    log_init();

    NRF_LOG_INFO("******************* Firmware Start **********************");

    timers_init();
    //Init board and timers before ble
    board_init(&m_conn_handle, &m_advertising, &erase_bonds);
    detect_init();

    NRF_LOG_INFO("Detect firmware started.");

    // Start execution.
    //application_timers_start();
    m_ble_advertising_start(erase_bonds);

    // Enter main loop.
    for (;;)
    {
        app_sched_execute();

        idle_state_handle();
    }
}

/**
 * @}
 */
