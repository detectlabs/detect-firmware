#ifndef __DETECTION_FLASH_H__
#define __DETECTION_FLASH_H__

#include "ble_dds.h"

/**@brief Gas sensor baseline values for different modes of operation.
 */
// typedef PACKED( struct
// {
//     uint16_t mode_250ms;
//     uint16_t mode_1s;
//     uint16_t mode_10s;
//     uint16_t mode_60s;
// }) m_gas_baseline_t;

/**@brief Function for storing the configuration.
 *
 * @param[in] p_config  Pointer to configuration to be stored.
 *
 * @retval NRF_SUCCESS      If initialization was successful.
 * @retval NRF_ERROR_NULL   If a NULL pointer was supplied.
 * @retval Other codes from the underlying drivers.
 */
//uint32_t m_env_flash_config_store(const ble_tes_config_t * p_config);

/**@brief Function for loading the configuration.
 *
 * @param[out] p_config  Pointer to loaded configuration.
 *
 * @retval NRF_SUCCESS      If initialization was successful.
 * @retval Other codes from the underlying drivers.
 */
//uint32_t m_env_flash_config_load(ble_tes_config_t ** p_config);

/**@brief Function for storing the gas sensor baseline.
 *
 * @param[in] p_baseline  Pointer to baseline to be stored.
 *
 * @retval NRF_SUCCESS      If initialization was successful.
 * @retval NRF_ERROR_NULL   If a NULL pointer was supplied.
 * @retval Other codes from the underlying drivers.
 */
//uint32_t m_env_flash_baseline_store(const m_gas_baseline_t * p_baseline);

/**@brief Function for loading the gas sensor baseline.
 *
 * @param[out] p_baseline  Pointer to loaded baseline.
 *
 * @retval NRF_SUCCESS      If initialization was successful.
 * @retval Other codes from the underlying drivers.
 */
//uint32_t m_env_flash_baseline_load(m_gas_baseline_t ** p_baseline);

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