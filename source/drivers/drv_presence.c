#include "drv_presence.h"
#include "drv_ak9750.h"
#include "sdk_macros.h"
#include "nrf_log.h"
#include "nrf_drv_gpiote.h"
#include "app_scheduler.h"

/**@brief Pressure configuration struct.
 */
typedef struct
{
    drv_ak9750_twi_cfg_t         cfg;           ///< TWI configuraion.
    drv_presence_evt_handler_t   evt_handler;   ///< Event handler called by gpiote_evt_sceduled.
    drv_presence_mode_t          mode;          ///< Mode of operation.
    bool                         enabled;       ///< Driver enabled.
} drv_presence_t;

/**@brief Stored configuration.
 */
static drv_presence_t m_drv_presence;

/**@brief GPIOTE sceduled handler, executed in main-context.
 */
static void gpiote_evt_sceduled(void * p_event_data, uint16_t event_size)
{
    // Data ready
    drv_presence_evt_t evt;
    evt.type = DRV_PRESENCE_EVT_DATA;
    evt.mode = DRV_PRESENCE_MODE_CONTINUOUS;

    m_drv_presence.evt_handler(&evt);
}

/**@brief GPIOTE event handler, executed in interrupt-context.
 */
static void gpiote_evt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    uint32_t err_code;

    //if ((pin == m_drv_presence.cfg.pin_int) && (nrf_gpio_pin_read(m_drv_presence.cfg.pin_int) == 0))
    //{
        err_code = app_sched_event_put(0, 0, gpiote_evt_sceduled);
        APP_ERROR_CHECK(err_code);
    //}
}

/**@brief Initialize the GPIO tasks and events system to catch pin data ready interrupts.
 */
static uint32_t gpiote_init(uint32_t pin)
{
    uint32_t err_code;

    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        RETURN_IF_ERROR(err_code);
    }

    // nrf_drv_gpiote_in_config_t gpiote_in_config;
    // gpiote_in_config.is_watcher  = false;
    // gpiote_in_config.hi_accuracy = false;
    // gpiote_in_config.pull        = NRF_GPIO_PIN_NOPULL;
    // gpiote_in_config.sense       = NRF_GPIOTE_POLARITY_TOGGLE;

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = GPIO_PIN_CNF_PULL_Disabled;

    err_code = nrf_drv_gpiote_in_init(pin, &in_config, gpiote_evt_handler);
    RETURN_IF_ERROR(err_code);

    nrf_drv_gpiote_in_event_enable(pin, true);

    return NRF_SUCCESS;
}

uint32_t drv_presence_init(drv_presence_init_t * p_params)
{
    uint32_t err_code;
    uint8_t  who_am_i;

    VERIFY_PARAM_NOT_NULL(p_params);
    VERIFY_PARAM_NOT_NULL(p_params->p_twi_instance);
    VERIFY_PARAM_NOT_NULL(p_params->p_twi_cfg);
    VERIFY_PARAM_NOT_NULL(p_params->evt_handler);

    m_drv_presence.mode               = p_params->mode;
    m_drv_presence.evt_handler        = p_params->evt_handler;

    m_drv_presence.cfg.twi_addr       = p_params->twi_addr;
    m_drv_presence.cfg.pin_int        = p_params->pin_int;
    m_drv_presence.cfg.p_twi_instance = p_params->p_twi_instance;
    m_drv_presence.cfg.p_twi_cfg      = p_params->p_twi_cfg;

    m_drv_presence.enabled            = false;

    err_code = drv_ak9750_open(&m_drv_presence.cfg);
    RETURN_IF_ERROR(err_code);

    err_code = drv_ak9750_verify(&who_am_i);
    RETURN_IF_ERROR(err_code);

    err_code = drv_ak9750_init();
    RETURN_IF_ERROR(err_code);

    err_code = drv_ak9750_close();
    RETURN_IF_ERROR(err_code);
    
    // Presence sensor has internal pullup
    nrf_gpio_cfg_input(m_drv_presence.cfg.pin_int, GPIO_PIN_CNF_PULL_Disabled);

    return NRF_SUCCESS;
}

uint32_t drv_presence_enable(void)
{
    uint32_t err_code;

    if (m_drv_presence.enabled)
    {
        return NRF_SUCCESS;
    }

    err_code = gpiote_init(m_drv_presence.cfg.pin_int);
    RETURN_IF_ERROR(err_code);

    err_code = drv_ak9750_open(&m_drv_presence.cfg);
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_cfg_set();
    RETURN_IF_ERROR(err_code);

    err_code = drv_ak9750_close();
    RETURN_IF_ERROR(err_code);

    m_drv_presence.enabled = true;

    return NRF_SUCCESS;
}

/**@brief Uninitialize the GPIO tasks and events system.
 */
static void gpiote_uninit(uint32_t pin)
{
    nrf_drv_gpiote_in_uninit(pin);
}

uint32_t drv_presence_disable(void)
{
    uint32_t err_code;

    if (m_drv_presence.enabled == false)
    {
        return NRF_SUCCESS;
    }
    m_drv_presence.enabled = false;

    gpiote_uninit(m_drv_presence.cfg.pin_int);

    return NRF_SUCCESS;
}

uint32_t drv_presence_sample(void)
{
    uint32_t err_code;

    err_code = drv_ak9750_open(&m_drv_presence.cfg);
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_one_shot();
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_close();
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

uint32_t drv_presence_get(ble_dds_presence_t * presence)
{
    uint32_t err_code;

    err_code = drv_ak9750_open(&m_drv_presence.cfg);
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_get_irs(presence);
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_close();
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}
