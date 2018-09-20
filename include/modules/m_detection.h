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
#define     DETECTION_CONFIG_DEFAULT {                 \
    .presence_interval_ms = 1000,                      \
    .range_interval_ms    = 1000,                      \
    .threshold_config     =                            \
    {                                                  \
        .eth13h            = 100,                      \
        .eth13l            = 100,                      \
        .eth24h            = 100,                      \
        .eth24l            = 100                       \
    },                                                 \
    .sample_mode          = SAMPLE_MODE_CONTINUOUS     \
}

uint32_t m_detection_init(m_ble_service_handle_t * p_handle, m_detection_init_t * p_params);


#endif