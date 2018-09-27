#ifndef __DRV_PRESENCE_H__
#define __DRV_PRESENCE_H__

#include "nrf_drv_twi.h"
#include <stdint.h>
#include "ble_dds.h"
#include "m_ble.h"

/**@brief Pressure driver event types.
 */
typedef enum
{
    DRV_PRESENCE_EVT_DATA,    /**<Converted value ready to be read.*/
    DRV_PRESENCE_EVT_MOTION_STOP,
    DRV_PRESENCE_EVT_ERROR    /**<HW error on the communication bus.*/
}drv_presence_evt_type_t;

/**@brief Pressure modes of operation.
 */
// typedef enum
// {
//     DRV_PRESENCE_MODE_CONTINUOUS,
//     DRV_PRESENCE_MODE_MOTION
// }drv_presence_mode_t;

/**@brief Pressure event struct.
 */
typedef struct
{
    drv_presence_evt_type_t type;
    ble_dds_sample_mode_t     mode;
}drv_presence_evt_t;

/**@brief Pressure driver event handler callback type.
 */
typedef void (*drv_presence_evt_handler_t)(drv_presence_evt_t const * p_evt);

/**@brief Initialization struct for presence driver.
 */
typedef struct
{
    uint8_t                      twi_addr;          ///< TWI address.
    uint32_t                     pin_int;           ///< Interrupt pin.
    nrf_drv_twi_t        const * p_twi_instance;    ///< The instance of TWI master to be used for transactions.
    nrf_drv_twi_config_t const * p_twi_cfg;         ///< The TWI configuration to use while the driver is enabled.
    drv_presence_evt_handler_t   evt_handler;       ///< Event handler - called after a pin interrupt has been detected.
    ble_dds_sample_mode_t          mode;              ///< Current mode of operation.
}drv_presence_init_t;

uint32_t drv_presence_disable_dri(void);

uint32_t drv_presence_enable_dri(void);

uint32_t gpiote_init(uint32_t pin);

void gpiote_uninit(uint32_t pin);

uint32_t drv_presence_clear_int(void);

uint32_t drv_presence_read_int(uint8_t * status);

/**@brief Function for initializing the presence driver.
 *
 * @param[in] p_params      Pointer to init parameters.
 *
 * @retval NRF_SUCCESS             If initialization was successful.
 * @retval NRF_ERROR_INVALID_STATE If the driver is in invalid state.
 */
uint32_t drv_presence_init(drv_presence_init_t * p_params);

/**@brief Function for enabling the presence sensor.
 *
 * @retval NRF_SUCCESS             If initialization was successful.
 */
uint32_t drv_presence_enable(void);

/**@brief Function for disabling the presence sensor.
 *
 * @retval NRF_SUCCESS             If initialization was successful.
 */
uint32_t drv_presence_disable(void);

/**@brief Function for resetting the chip to all default register values.
*
* @retval NRF_SUCCESS             If operation was successful.
* @retval NRF_ERROR_BUSY          If TWI bus was busy.
*/
uint32_t drv_presence_reset(void);

/**@brief Function for changing the mode of the presence sensor.
 *
 * @param[in] mode                 Altimeter or Barometer.
 *
 * @retval NRF_SUCCESS             If configuration was successful.
 * @retval NRF_ERROR_BUSY          If the TWI drivers are busy.
 */
uint32_t drv_presence_mode_set(ble_dds_sample_mode_t mode);

/**@brief Function for getting the presence data [hPa].
 *
 * @retval Pressure data.
 */
uint32_t drv_presence_get(ble_dds_presence_t * presence);

/**@brief Function for starting the sampling.
 *
 * @retval NRF_SUCCESS             If start sampling was successful.
 */
uint32_t drv_presence_sample(void);

/**@brief Function for putting the sensor to sleep.
 *
 * @retval NRF_SUCCESS             If sleep was successful.
 */
uint32_t drv_presence_sleep(void);

#endif