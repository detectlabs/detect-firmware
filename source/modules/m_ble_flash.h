#ifndef __M_FDS_H__
#define __M_FDS_H__

#include "ble_dcs.h"

/**@brief Function for initializing Thingy configuration flash handling.
 *
 * @param[in] p_default_config  Pointer to default configuration.
 * @param[out] p_config         Pointer to current configuration.
 *
 * @retval NRF_SUCCESS      Operation was successful.
 * @retval NRF_ERROR_NULL   If a NULL pointer was supplied.
 * @retval Other codes from the underlying drivers.
 */

uint32_t m_ble_flash_config_store(const ble_dcs_params_t * p_config);

uint32_t m_ble_flash_init(const ble_dcs_params_t * p_default_config, ble_dcs_params_t ** p_config);


#endif