#include "m_detection.h"
#include "detect_board.h"
#include "drv_presence.h"

static ble_dds_t              m_dds;            ///< Structure to identify the Thingy Environment Service.
static ble_dds_config_t     * m_p_config;       ///< Configuraion pointer./
static const ble_dds_config_t m_default_config = DETECTION_CONFIG_DEFAULT; ///< Default configuraion.

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
                ble_dds_presence_t presence;

                drv_presence_get(&presence);
                (void)ble_dds_presence_set(&m_dds, &presence);
                NRF_LOG_INFO("********PRESENCE EVENT!!!!! \r\n");

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

/**@brief Function for stopping pressure sampling.
 */
static uint32_t presence_stop(void)
{
    uint32_t err_code;

    err_code = app_timer_stop(presence_timer_id);
    APP_ERROR_CHECK(err_code);

    return drv_presence_disable();
}

uint32_t m_detection_stop(void)
{
    uint32_t err_code;

    err_code = presence_stop();
    APP_ERROR_CHECK(err_code);

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

    return app_timer_start(presence_timer_id,
                           APP_TIMER_TICKS(m_p_config->presence_interval_ms),
                           NULL);                     
}

static uint32_t config_verify(ble_dds_config_t * p_config)
{
    uint32_t err_code;

    if ( (p_config->presence_interval_ms < BLE_DDS_CONFIG_PRESENCE_INT_MIN)            ||
         (p_config->presence_interval_ms > BLE_DDS_CONFIG_PRESENCE_INT_MAX)            ||
         (p_config->range_interval_ms < BLE_DDS_CONFIG_RANGE_INT_MIN)                  ||
         (p_config->range_interval_ms > BLE_DDS_CONFIG_RANGE_INT_MAX)                  ||
         (p_config->threshold_config.eth13h < BLE_DDS_CONFIG_THRESHOLD_MIN)            ||
         (p_config->threshold_config.eth13l > BLE_DDS_CONFIG_THRESHOLD_MAX)            ||
         (p_config->threshold_config.eth24h < BLE_DDS_CONFIG_THRESHOLD_MIN)            ||
         ((int)p_config->threshold_config.eth24l > (int)BLE_DDS_CONFIG_THRESHOLD_MAX))
    {
        err_code = m_det_flash_config_store((ble_dds_config_t *)&m_default_config);
        APP_ERROR_CHECK(err_code);
    }

    return NRF_SUCCESS;
}

/**@brief Function for applying the configuration.
 *
 */
static uint32_t config_apply(ble_dds_config_t * p_config)
{
    uint32_t err_code;

    VERIFY_PARAM_NOT_NULL(p_config);

    (void)presence_stop();

     if ((p_config->presence_interval_ms > 0) &&
        (m_dds.is_presence_notif_enabled) )
    {
        err_code = presence_start();
        APP_ERROR_CHECK(err_code);
    }

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
    NRF_LOG_INFO("ble_dds_on_ble_evt!!!!! \r\n");

    ble_dds_on_ble_evt(&m_dds, p_ble_evt);

    if (p_ble_evt->header.evt_id == BLE_GAP_EVT_DISCONNECTED)
    {
        NRF_LOG_INFO("DETECTION ON BLE EVT \r\n");
        uint32_t err_code;
        err_code = m_detection_stop();
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling event from the Detect Detection Service.
 *
 * @details This function will process the data received from the Detect Detection BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_tes    Detect Detection Service structure.
 * @param[in] evt_type Detect Detection Service event type.
 * @param[in] p_data   Event data.
 * @param[in] length   Length of the data.
 */
static void ble_dds_evt_handler( ble_dds_t        * p_dds,
                                 ble_dds_evt_type_t evt_type,
                                 uint8_t          * p_data,
                                 uint16_t           length)
{
    uint32_t err_code;

    switch (evt_type)
    {
        case BLE_DDS_EVT_NOTIF_PRESENCE:
            NRF_LOG_DEBUG("tes_evt_handler: BLE_TES_EVT_NOTIF_PRESENCE: %d\r\n", p_dds->is_presence_notif_enabled);
            if (p_dds->is_presence_notif_enabled)
            {
                err_code = presence_start();
                APP_ERROR_CHECK(err_code);
            }
            else
            {
                err_code = presence_stop();
                APP_ERROR_CHECK(err_code);
            }
            break;

        // case BLE_TES_EVT_NOTIF_RANGE:
        //     NRF_LOG_DEBUG("tes_evt_handler: BLE_TES_EVT_NOTIF_RANGE: %d\r\n", p_tes->is_range_notif_enabled);
        //     if (p_tes->is_range_notif_enabled)
        //     {
        //         err_code = range_start();
        //         APP_ERROR_CHECK(err_code);
        //     }
        //     else
        //     {
        //         err_code = range_stop();
        //         APP_ERROR_CHECK(err_code);
        //     }
        //     break;

        case BLE_DDS_EVT_CONFIG_RECEIVED:
        {
            NRF_LOG_DEBUG("dds_evt_handler: BLE_DDS_EVT_CONFIG_RECEIVED: %d\r\n", length);
            APP_ERROR_CHECK_BOOL(length == sizeof(ble_dds_config_t));

            err_code = m_det_flash_config_store((ble_dds_config_t *)p_data);
            APP_ERROR_CHECK(err_code);

            err_code = config_apply((ble_dds_config_t *)p_data);
            APP_ERROR_CHECK(err_code);
        }
        break;

        default:
            break;

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
    uint32_t err_code;
    ret_code_t rc;
    ble_dds_init_t       dds_init;

    /**@brief Load configuration from flash. */
    rc = m_det_flash_init(&m_default_config, &m_p_config);
    APP_ERROR_CHECK(rc);

    if (major_minor_fw_ver_changed)
    {
        err_code = m_det_flash_config_store(&m_default_config);
        APP_ERROR_CHECK(err_code);
    }

    err_code = config_verify(m_p_config);
    APP_ERROR_CHECK(err_code);

    dds_init.p_init_config = m_p_config;
    dds_init.evt_handler = ble_dds_evt_handler;

    NRF_LOG_INFO("Init: ble_dds_init \r\n");
    err_code = ble_dds_init(&m_dds, &dds_init);
    APP_ERROR_CHECK(err_code);

    (void)config_apply(m_p_config);

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
        .interrupt_priority = APP_IRQ_PRIORITY_THREAD,
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