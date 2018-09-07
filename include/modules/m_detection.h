#ifndef __DETECTION_H__
#define __DETECTION_H__

#include <stdint.h>
#include <stdbool.h>
#include "m_ble.h"
#include "nrf_drv_twi.h"

/**@brief Initialization parameters. */
typedef struct
{
    const nrf_drv_twi_t * p_twi_instance;
} m_detection_init_t;

uint32_t m_detection_init(m_ble_service_handle_t * p_handle, m_detection_init_t * p_params);


#endif