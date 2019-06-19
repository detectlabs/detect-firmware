#ifndef __DETECTION_H__
#define __DETECTION_H__

#include <stdint.h>
#include <stdbool.h>
#include "m_ble.h"
#include "nrf_drv_twi.h"
#include "ble_dds.h"

/**@brief Initialization parameters. */
typedef struct
{
    const nrf_drv_twi_t * p_twi_instance;
} m_detection_init_t;

/**@brief Detection default configuration. */

/* presence_interval_ms      In Continuous sample mode, the sample should be iniated right after the last data is read, otherwise you incur the time to measure plus the interval time 8 */
#define     DETECTION_CONFIG_DEFAULT {                 \
    .presence_interval_ms = 33,                        \
    .range_interval_ms    = 33,                        \
    .threshold_config     =                            \
    {                                                  \
        .eth13h            =  200,                     \
        .eth13l            = -200,                     \
        .eth24h            =  200,                     \
        .eth24l            = -200                      \
    },                                                 \
    .sample_mode          = SAMPLE_MODE_MOTION     \
}

uint32_t m_detection_init(m_ble_service_handle_t * p_handle, m_detection_init_t * p_params);


#endif