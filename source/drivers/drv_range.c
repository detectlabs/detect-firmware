#include "sdk_macros.h"
#include "nrf_log.h"
#include "nrf_drv_gpiote.h"
#include "app_scheduler.h"
#include "drv_range.h"
#include "drv_vl53l0x.h"
#include "nrf_delay.h"

/**@brief Pressure configuration struct.
 */
typedef struct
{
    drv_vl53l0x_twi_cfg_t            cfg;           ///< TWI configuraion.
    drv_range_evt_handler_t  evt_handler;   ///< Event handler called by gpiote_evt_sceduled.
    drv_range_mode_t                mode;          ///< Mode of operation.
    bool                         enabled;       ///< Driver enabled.
} drv_range_t;

/**@brief Stored configuration.
 */
static drv_range_t m_drv_range;


uint32_t drv_range_init(drv_range_init_t * p_params)
{
    uint32_t err_code;
    uint8_t  who_am_i;

    VERIFY_PARAM_NOT_NULL(p_params);
    VERIFY_PARAM_NOT_NULL(p_params->p_twi_instance);
    VERIFY_PARAM_NOT_NULL(p_params->p_twi_cfg);
    VERIFY_PARAM_NOT_NULL(p_params->evt_handler);

    m_drv_range.mode               = p_params->mode;
    m_drv_range.evt_handler        = p_params->evt_handler;

    m_drv_range.cfg.twi_addr       = p_params->twi_addr;
    m_drv_range.cfg.pin_int        = p_params->pin_int;
    m_drv_range.cfg.p_twi_instance = p_params->p_twi_instance;
    m_drv_range.cfg.p_twi_cfg      = p_params->p_twi_cfg;

    m_drv_range.enabled            = false;

    err_code = drv_vl53l0x_open(&m_drv_range.cfg);
    RETURN_IF_ERROR(err_code);

    err_code = drv_vl53l0x_verify(&who_am_i);
    RETURN_IF_ERROR(err_code);

    err_code = drv_vl53l0x_init();
    RETURN_IF_ERROR(err_code);

    err_code = drv_vl53l0x_close();
    RETURN_IF_ERROR(err_code);
    
    // range sensor has internal pullup
    nrf_gpio_cfg_input(m_drv_range.cfg.pin_int, GPIO_PIN_CNF_PULL_Disabled);

    return NRF_SUCCESS;
}

uint32_t drv_range_sample(void)
{
    uint32_t err_code;

    err_code = drv_vl53l0x_open(&m_drv_range.cfg);
    RETURN_IF_ERROR(err_code);

    // err_code = drv_ak9750_one_shot();
    // RETURN_IF_ERROR(err_code);

    NRF_LOG_INFO("*** Range Sample ****\r\n");

    vl53l0x_init(true);
    // lower the return signal rate limit (default is 0.25 MCPS)
    setSignalRateLimit(0.1);
    // increase laser pulse periods (defaults are 14 and 10 PCLKs)
    setVcselPulsePeriod(VcselPeriodPreRange, 18);
    setVcselPulsePeriod(VcselPeriodFinalRange, 14);

    setMeasurementTimingBudget(20000);

    nrf_delay_ms(200);

    startRangeSingleMillimeters();
    NRF_LOG_RAW_INFO("\nRange: %d  \n", readRangeContinuousMillimeters());

    err_code = drv_vl53l0x_close();
    RETURN_IF_ERROR(err_code);

    return NRF_SUCCESS;
}

