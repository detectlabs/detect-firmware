#ifndef __DETECTION_FLASH_H__
#define __DETECTION_FLASH_H__

#include "ble_dds.h"

uint32_t m_det_flash_config_store(const ble_dds_config_t * p_config);

/**@brief Function for initializing weather station flash handling.
 *
 * @param[in]  p_default_config     Pointer to default configuration.
 * @param[out] p_config             Pointer to current configuration.
 * @param[in]  p_default_baseline   Pointer to default gas sensor baseline.
 * @param[out] p_baseline           Pointer to current gas sensor baseline.
 *
 * @retval NRF_SUCCESS      If initialization was successful.
 * @retval NRF_ERROR_NULL   If a NULL pointer was supplied.
 * @retval Other codes from the underlying drivers.
 */
uint32_t m_det_flash_init(const ble_dds_config_t * p_default_config,
                          ble_dds_config_t ** p_config);


#endif