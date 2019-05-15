#include "m_batt_meas.h"
#include "sdk_config.h"
#include "nrf_drv_saadc.h"
#include "app_timer.h"
#include "math.h"
#include "nrf_gpio.h"
#include "app_scheduler.h"
#include "nrf_drv_gpiote.h"
#include "ble_bas.h"
#include <stdint.h>
#include <string.h>

//#define  NRF_LOG_MODULE_NAME "m_batt_meas   "
#include "nrf_log.h"

#define ADC_GAIN                    NRF_SAADC_GAIN1     // ADC gain.
#define ADC_REFERENCE_VOLTAGE       (0.6f)              // The standard internal ADC reference voltage.
#define ADC_RESOLUTION_BITS         (8 + (SAADC_CONFIG_RESOLUTION * 2)) //ADC resolution [bits].
#define ADC_BUF_SIZE                (1)                 // Size of each ADC buffer.
#define INVALID_BATTERY_LEVEL       (0xFF)              // Invalid/default battery level.

// static ble_bas_t                    m_bas;                          // Structure to identify the battery service.
// static m_batt_meas_event_handler_t  m_evt_handler;                  // Event handler function pointer.
// static batt_meas_param_t            m_batt_meas_param;              // Battery parameters.
// static float                        m_battery_divider_factor;       //
// static nrf_saadc_value_t            m_buffer[ADC_BUF_SIZE];         //
// static volatile bool                m_adc_cal_in_progress;          //
// static bool                         m_ble_bas_configured = false;   // Has the BLE battery service been initalized?
// static uint8_t                      m_initial_batt_level_percent = INVALID_BATTERY_LEVEL;  // Initial battery level in percent.

/** @brief Timer for periodic battery measurement.
 */
//APP_TIMER_DEF(batt_meas_app_timer_id);


// /**@brief Function for passing the BLE event to the Thingy Battery module.
//  *
//  * @details This callback function will be called from the BLE handling module.
//  *
//  * @param[in] p_ble_evt    Pointer to the BLE event.
//  */
// static void battery_on_ble_evt(ble_evt_t * p_ble_evt)
// {
//     ble_bas_on_ble_evt(&m_bas, p_ble_evt);

//     if (p_ble_evt->header.evt_id == BLE_GAP_EVT_DISCONNECTED)
//     {
//         // No particular action needed on disconnect.
//     }
// }

// /**@brief Event handler, handles events in the Battery Service.
//  *
//  * @details This callback function is often used to enable a service when requested over BLE,
//  * and disable when not requested to save power. m_batt_meas runs all the time, so no
//  * enabling/disabling is performed.
//  */
// static void ble_bas_evt_handler(ble_bas_t * p_bas, ble_bas_evt_t * p_evt)
// {
//     switch (p_evt->evt_type)
//     {
//         case BLE_BAS_EVT_NOTIFICATION_ENABLED:
//             NRF_LOG_DEBUG("BLE_BAS_EVT_NOTIFICATION_ENABLED \r\n");
//             break;

//         case BLE_BAS_EVT_NOTIFICATION_DISABLED:
//             NRF_LOG_DEBUG("BLE_BAS_EVT_NOTIFICATION_DISABLED \r\n");
//             break;

//         default:
//             break;
//     }
// }

// /**@brief Function for initializing the Thingy Battery Service.
//  *
//  * @details This callback function will be called from the ble handling module to initialize the Battery service.
//  *
//  * @retval NRF_SUCCESS If initialization was successful.
//  */
// static uint32_t battery_service_init(bool major_minor_fw_ver_changed)
// {
//     uint32_t              err_code;
//     ble_bas_init_t        bas_init;

//     memset(&bas_init, 0, sizeof(bas_init));

//     NRF_LOG_DEBUG("battery_service_init: ble_bas_init \r\n ");

//     // Security level for the Battery Service
//     BLE_GAP_CONN_SEC_MODE_SET_OPEN(     &bas_init.battery_level_char_attr_md.cccd_write_perm);
//     BLE_GAP_CONN_SEC_MODE_SET_OPEN(     &bas_init.battery_level_char_attr_md.read_perm);
//     BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);
//     BLE_GAP_CONN_SEC_MODE_SET_OPEN(     &bas_init.battery_level_report_read_perm);

//     bas_init.evt_handler          = ble_bas_evt_handler;
//     bas_init.support_notification = true;
//     bas_init.p_report_ref         = NULL;
//     bas_init.initial_batt_level   = m_initial_batt_level_percent;

//     err_code = ble_bas_init(&m_bas, &bas_init);
//     APP_ERROR_CHECK(err_code);

//     m_ble_bas_configured = true;

//     return NRF_SUCCESS;
// }

uint32_t m_batt_meas_enable(uint32_t meas_interval_ms)
{
    // uint32_t err_code;

    // if (meas_interval_ms < MEAS_INTERVAL_LOW_LIMIT_MS)
    // {
    //     return M_BATT_STATUS_CODE_INVALID_PARAM;
    // }

    // if (m_batt_meas_param.batt_mon_en_pin_used)
    // {
    //     err_code = drv_ext_gpio_pin_set(m_batt_meas_param.batt_mon_en_pin_no);     // Enable battery monitoring.
    //     APP_ERROR_CHECK(err_code);
    // }

    // // Call for a battery voltage sample immediately after enabling battery measurements.
    // app_timer_periodic_handler(NULL);

    // err_code = app_timer_create(&batt_meas_app_timer_id,
    //                             APP_TIMER_MODE_REPEATED,
    //                             app_timer_periodic_handler);
    // APP_ERROR_CHECK(err_code);

    // err_code = app_timer_start(batt_meas_app_timer_id,
    //                            APP_TIMER_TICKS(meas_interval_ms), NULL);
    // APP_ERROR_CHECK(err_code);

    return M_BATT_STATUS_CODE_SUCCESS;
}


// uint32_t m_batt_meas_disable(void)
// {
//     uint32_t err_code;

//     if (m_batt_meas_param.batt_mon_en_pin_used)
//     {
//         err_code = drv_ext_gpio_pin_clear(m_batt_meas_param.batt_mon_en_pin_no);     // Disable battery monitoring to save power.
//         APP_ERROR_CHECK(err_code);
//     }

//     err_code = app_timer_stop(batt_meas_app_timer_id);
//     APP_ERROR_CHECK(err_code);

//     return M_BATT_STATUS_CODE_SUCCESS;
// }

uint32_t m_batt_meas_init(m_ble_service_handle_t * p_handle, batt_meas_init_t const * const p_batt_meas_init)
{
    // uint32_t err_code;

    // VERIFY_PARAM_NOT_NULL(p_handle);
    // VERIFY_PARAM_NOT_NULL(p_batt_meas_init);

    // NRF_LOG_DEBUG("Init \r\n");

    // p_handle->ble_evt_cb = battery_on_ble_evt;
    // p_handle->init_cb    = battery_service_init;  // Pointer to ble init function.

    // err_code = param_check(p_batt_meas_init);
    // RETURN_IF_ERROR(err_code);

    // m_evt_handler = p_batt_meas_init->evt_handler;
    // m_batt_meas_param = p_batt_meas_init->batt_meas_param;

    // if (m_batt_meas_param.batt_mon_en_pin_used)
    // {
    //     err_code = drv_ext_gpio_cfg_output(m_batt_meas_param.batt_mon_en_pin_no);
    //     RETURN_IF_ERROR(err_code);

    //     err_code = drv_ext_gpio_pin_clear(m_batt_meas_param.batt_mon_en_pin_no);     // Disable battery monitoring to save power.
    //     RETURN_IF_ERROR(err_code);
    // }

    // nrf_gpio_cfg_input(m_batt_meas_param.batt_chg_stat_pin_no, NRF_GPIO_PIN_NOPULL);
    // nrf_gpio_cfg_input(m_batt_meas_param.usb_detect_pin_no,    NRF_GPIO_PIN_NOPULL);
    // nrf_gpio_cfg_input(m_batt_meas_param.adc_pin_no,           NRF_GPIO_PIN_NOPULL);

    // err_code = gpiote_init();
    // RETURN_IF_ERROR(err_code);

    // err_code = saadc_calibrate();
    // RETURN_IF_ERROR(err_code);

    return M_BATT_STATUS_CODE_SUCCESS;
}