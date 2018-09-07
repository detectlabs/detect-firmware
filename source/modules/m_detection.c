#include "m_detection.h"
#include "detect_board.h"
#include "drv_presence.h"

//static ble_dds_t              m_dds;            ///< Structure to identify the Thingy Environment Service.

APP_TIMER_DEF(presence_timer_id);

/**@brief Pressure sensor event handler.
 */
static void drv_presence_evt_handler(drv_presence_evt_t const * p_event)
{
    NRF_LOG_INFO("******************************************HERE \r\n");
    switch (p_event->type)
    {
        case DRV_PRESENCE_EVT_DATA:
        {
            if (p_event->mode == DRV_PRESENCE_MODE_CONTINUOUS)
            {
                //ble_tes_presence_t pressure;
                //presence_conv_data(drv_presence_get(),&pressure);
                //(void)ble_tes_presence_set(&m_dds, &pressure);
                NRF_LOG_INFO("********PRESENCE EVENT!!!!! \r\n");
                drv_presence_get();

            }
        }
        break;

        case DRV_PRESENCE_EVT_ERROR:
            APP_ERROR_CHECK_BOOL(false);
            break;

        default:
            break;
    }
}

/**@brief Function for handling pressure timer timout event.
 *
 * @details This function will read the pressure at the configured rate.
 */
static void presence_timeout_handler(void * p_context)
{
    uint32_t err_code;

    NRF_LOG_INFO("PRESENCE TIMEOUT HANDLER!!!!! \r\n");

    err_code = drv_presence_sample();
    APP_ERROR_CHECK(err_code);
}

uint32_t m_detection_stop(void)
{
    uint32_t err_code;

    // err_code = temperature_stop(false);
    // APP_ERROR_CHECK(err_code);

    // err_code = pressure_stop();
    // APP_ERROR_CHECK(err_code);

    // err_code = humidity_stop(true);
    // APP_ERROR_CHECK(err_code);

    // err_code = color_stop();
    // APP_ERROR_CHECK(err_code);

    // err_code = gas_stop();
    // APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

/**@brief Function for starting pressure sampling.
 */
static uint32_t presence_start(void)
{
    uint32_t err_code;

    err_code = drv_presence_enable();
    APP_ERROR_CHECK(err_code);

    err_code = drv_presence_sample();
    APP_ERROR_CHECK(err_code);

    // return app_timer_start(presence_timer_id,
    //                        APP_TIMER_TICKS(m_p_config->presence_interval_ms),
    //                        NULL);

    return app_timer_start(presence_timer_id,
                           APP_TIMER_TICKS(2000),
                           NULL);                       
}

/**@brief Function for applying the configuration.
 *
 */
static uint32_t config_apply(void) //ble_dds_config_t * p_config)
{
    uint32_t err_code;

    // NULL_PARAM_CHECK(p_config);

    // (void)presence_stop(false);

    // if ((p_config->presence_interval_ms > 0) &&
    //     (m_dds.is_presence_notif_enabled) )
    // {
        err_code = presence_start();
        APP_ERROR_CHECK(err_code);
    //}

    return NRF_SUCCESS;
}


/**@brief Function for passing the BLE event to the Thingy Environment service.
 *
 * @details This callback function will be called from the BLE handling module.
 *
 * @param[in] p_ble_evt    Pointer to the BLE event.
 */
static void detection_on_ble_evt(ble_evt_t * p_ble_evt)
{
    //ble_tes_on_ble_evt(&m_dds, p_ble_evt);

    if (p_ble_evt->header.evt_id == BLE_GAP_EVT_DISCONNECTED)
    {
        NRF_LOG_INFO("DETECTION ON BLE EVT \r\n");
        uint32_t err_code;
        err_code = m_detection_stop();
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for initializing the Thingy Environment Service.
 *
 * @details This callback function will be called from the ble handling module to initialize the Thingy Environment service.
 *
 * @retval NRF_SUCCESS If initialization was successful.
 */
static uint32_t detection_service_init(bool major_minor_fw_ver_changed)
{
    NRF_LOG_INFO("Init: ble_dds_init \r\n");

    (void)config_apply(); //m_p_config);

    return NRF_SUCCESS;
}

static uint32_t presence_sensor_init(const nrf_drv_twi_t * p_twi_instance)
{
    drv_presence_init_t init_params;

    static const nrf_drv_twi_config_t twi_config =
    {
        .scl                = TWI_SCL,
        .sda                = TWI_SDA,
        .frequency          = NRF_TWI_FREQ_400K,
        .interrupt_priority = APP_IRQ_PRIORITY_LOW,
        .clear_bus_init     = false
    };

    init_params.twi_addr                = AK9750_ADDR;
    init_params.pin_int                 = AK9750_INT;
    init_params.p_twi_instance          = p_twi_instance;
    init_params.p_twi_cfg               = &twi_config;
    init_params.evt_handler             = drv_presence_evt_handler;
    init_params.mode                    = DRV_PRESENCE_MODE_CONTINUOUS;

    return drv_presence_init(&init_params);
}

uint32_t m_detection_init(m_ble_service_handle_t * p_handle, m_detection_init_t * p_params)
{
    uint32_t err_code;

    VERIFY_PARAM_NOT_NULL(p_handle);
    VERIFY_PARAM_NOT_NULL(p_params);

    NRF_LOG_INFO("**** Detection Init ****\r\n");

    p_handle->ble_evt_cb = detection_on_ble_evt;
    p_handle->init_cb    = detection_service_init;

    NRF_LOG_INFO("*** Presence Init ****\r\n");

    /**@brief Init drivers */
    err_code = presence_sensor_init(p_params->p_twi_instance);
    APP_ERROR_CHECK(err_code);

    /**@brief Init application timers */
    err_code = app_timer_create(&presence_timer_id, APP_TIMER_MODE_REPEATED, presence_timeout_handler);
    APP_ERROR_CHECK(err_code);
}