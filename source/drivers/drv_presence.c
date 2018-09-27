#include "drv_presence.h"
#include "drv_ak9750.h"
#include "sdk_macros.h"
#include "nrf_log.h"
#include "nrf_drv_gpiote.h"
#include "app_scheduler.h"

#define DRI_MASK                    0x01
#define IR13H_MASK                  0x10
#define IR13L_MASK                  0x08
#define IR24H_MASK                  0x06
#define IR24L_MASK                  0x04

/**@brief Pressure configuration struct.
 */
typedef struct
{
    drv_ak9750_twi_cfg_t         cfg;           ///< TWI configuraion.
    drv_presence_evt_handler_t   evt_handler;   ///< Event handler called by gpiote_evt_sceduled.
    ble_dds_sample_mode_t          mode;          ///< Mode of operation.
    bool                         enabled;       ///< Driver enabled.
} drv_presence_t;

/**@brief Stored configuration.
 */
static drv_presence_t m_drv_presence;

static bool ak9750_output_active = false;

APP_TIMER_DEF(timeout_motion_timer_id);

// Timeout handler for the repeated timer
static void motion_timeout_handler(void * p_context)
{
     // Data ready
    drv_presence_evt_t evt;
    evt.type = DRV_PRESENCE_EVT_MOTION_STOP;
    evt.mode = SAMPLE_MODE_MOTION;
 
    //application_timers_stop();

    ak9750_output_active = false; 

    NRF_LOG_INFO("TIME ELAPSED ********************************************************************************************");

    //drv_presence_disable_dri();

    m_drv_presence.evt_handler(&evt);

    //Stop pushing new ir and range data
}

/**@brief GPIOTE sceduled handler, executed in main-context.
 */
static void gpiote_evt_sceduled(void * p_event_data, uint16_t event_size)
{
    // Data ready
    drv_presence_evt_t evt;
    uint8_t int_status;
    uint32_t err_code;

    NRF_LOG_RAW_INFO("\r####################### m_drv_presence.mode: %d  \n", m_drv_presence.mode);

    gpiote_uninit(m_drv_presence.cfg.pin_int);

    drv_presence_read_int(&int_status);

    if(m_drv_presence.mode == SAMPLE_MODE_MOTION )//&& int_status | ETH_MASK)
    {
         NRF_LOG_INFO("***********************   SAMPLE_MODE_MOTION ********************");

        if(int_status & IR13H_MASK || int_status & IR13L_MASK || int_status & IR24H_MASK || int_status & IR24L_MASK)
        {
            //NRF_LOG_RAW_INFO("\n$$$               INTST IR: %d            $$$$  \n", int_status);

            if(!ak9750_output_active)
            {
                //drv_presence_enable_dri();

                evt.type = DRV_PRESENCE_EVT_DATA;
                evt.mode = SAMPLE_MODE_MOTION;

                //Start timeout timer, that will stop data collection when there is no more motion
                err_code = app_timer_start(timeout_motion_timer_id, APP_TIMER_TICKS(3000), NULL);
                APP_ERROR_CHECK(err_code); 

                ak9750_output_active = true;

                m_drv_presence.evt_handler(&evt);
            }
            else
            {
                err_code = app_timer_stop(timeout_motion_timer_id);
                APP_ERROR_CHECK(err_code); 

                err_code = app_timer_start(timeout_motion_timer_id, APP_TIMER_TICKS(3000), NULL);
                APP_ERROR_CHECK(err_code); 
            }

            //Switch to single shot mode and push data

        }
    }
    else if(m_drv_presence.mode == SAMPLE_MODE_CONTINUOUS)
    {
        NRF_LOG_INFO("***********************   SAMPLE_MODE_CONTINUOUS ********************");

        evt.type = DRV_PRESENCE_EVT_DATA;
        evt.mode = SAMPLE_MODE_CONTINUOUS;

        m_drv_presence.evt_handler(&evt);
    }

    // Re-enable Pin Interrupt
    err_code = gpiote_init(m_drv_presence.cfg.pin_int);
    APP_ERROR_CHECK(err_code);

}

/**@brief GPIOTE event handler, executed in interrupt-context.
 */
static void gpiote_evt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    uint32_t err_code;

    NRF_LOG_INFO("*************************   PRESENCE INT ********************");


    err_code = app_sched_event_put(0, 0, gpiote_evt_sceduled);
    APP_ERROR_CHECK(err_code);

}

/**@brief Initialize the GPIO tasks and events system to catch pin data ready interrupts.
 */
uint32_t gpiote_init(uint32_t pin)
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

    NRF_LOG_RAW_INFO("\r####################### p_params->mode: %d  \n", p_params->mode);

    //m_drv_presence.mode               = p_params->mode;
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

    /**@brief Init application timers */
    err_code = app_timer_create(&timeout_motion_timer_id, APP_TIMER_MODE_SINGLE_SHOT, motion_timeout_handler);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

uint32_t drv_presence_enable(ble_dds_sample_mode_t mode)
{
    uint32_t err_code;

    if (m_drv_presence.enabled)
    {
        return NRF_SUCCESS;
    }

    m_drv_presence.mode = mode;

    err_code = gpiote_init(m_drv_presence.cfg.pin_int);
    RETURN_IF_ERROR(err_code);

    err_code = drv_ak9750_open(&m_drv_presence.cfg);
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_cfg_set(mode);
    RETURN_IF_ERROR(err_code);

    err_code = drv_ak9750_close();
    RETURN_IF_ERROR(err_code);

    m_drv_presence.enabled = true;

    return NRF_SUCCESS;
}

/**@brief Uninitialize the GPIO tasks and events system.
 */
void gpiote_uninit(uint32_t pin)
{
    nrf_drv_gpiote_in_uninit(pin);
}

uint32_t drv_presence_disable(void)
{
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

    NRF_LOG_INFO("!!!!!! AK ONE SHOT !!!!! \r\n");

    err_code = drv_ak9750_one_shot();
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_close();
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

uint32_t drv_presence_enable_dri(void)
{
    uint32_t err_code;

    err_code = drv_ak9750_open(&m_drv_presence.cfg);
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_enable_dri();
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_close();
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

uint32_t drv_presence_disable_dri(void)
{
    uint32_t err_code;

    err_code = drv_ak9750_open(&m_drv_presence.cfg);
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_disable_dri();
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_close();
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

uint32_t drv_presence_read_int(uint8_t * status)
{
    uint32_t err_code;

    err_code = drv_ak9750_open(&m_drv_presence.cfg);
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_read_int(status);
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_close();
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

uint32_t drv_presence_clear_int(void)
{
    uint32_t err_code;

    err_code = drv_ak9750_open(&m_drv_presence.cfg);
    APP_ERROR_CHECK(err_code);

    err_code = drv_ak9750_clear_int();
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
