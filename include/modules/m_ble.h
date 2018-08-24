 /** @file Motion module
 *
 * @defgroup m_motion Motion
 * @{
 * @ingroup modules
 * @brief Motion module API.
 *
 */

#ifndef __M_BLE_H__
#define __M_BLE_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_dfu_ble_svci_bond_sharing.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_state.h"
#include "ble_dfu.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "fds.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"
#include "nrf_power.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_bootloader_info.h"

void m_ble_advertising_start(bool erase_bonds);                                    /**< Forward declaration of advertising start function */

uint32_t m_ble_advertising_restart_without_whitelist(void);

uint32_t m_sd_ble_gap_disconnect(void);

/**@brief Function for initializing the BLE handling module..
 *
 *
 * @retval NRF_SUCCESS If initialization was successful.
 */
uint32_t m_ble_init();

#endif
