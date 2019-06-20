#ifndef __DRV_RANGE_H__
#define __DRV_RANGE_H__

#include "nrf_drv_twi.h"
#include <stdint.h>
#include "ble_dds.h"

/**@brief range driver event types.
 */
typedef enum
{
    DRV_RANGE_EVT_DATA,    /**<Converted value ready to be read.*/
    DRV_RANGE_EVT_ERROR    /**<HW error on the communication bus.*/
}drv_range_evt_type_t;

/**@brief range modes of operation.
 */
typedef enum
{
    DRV_RANGE_MODE_CONTINUOUS,
    DRV_RANGE_MODE_MOTION
}drv_range_mode_t;

/**@brief range event struct.
 */
typedef struct
{
    drv_range_evt_type_t type;
    drv_range_mode_t     mode;
}drv_range_evt_t;

/**@brief range driver event handler callback type.
 */
typedef void (*drv_range_evt_handler_t)(drv_range_evt_t const * p_evt);

/**@brief Initialization struct for range driver.
 */
typedef struct
{
    uint8_t                             twi_addr;   ///< TWI address.
    uint32_t                            pin_int;    ///< Interrupt pin.
    nrf_drv_twi_t        const * p_twi_instance;    ///< The instance of TWI master to be used for transactions.
    nrf_drv_twi_config_t const *      p_twi_cfg;    ///< The TWI configuration to use while the driver is enabled.
    drv_range_evt_handler_t         evt_handler;    ///< Event handler - called after a pin interrupt has been detected.
    drv_range_mode_t                       mode;    ///< Current mode of operation.
    uint8_t                    sampling_interval;   ///< The Sampling Interval to Initialize with
}drv_range_init_t;

/**@brief Function for initializing the range driver.
 *
 * @param[in] p_params      Pointer to init parameters.
 *
 * @retval NRF_SUCCESS             If initialization was successful.
 * @retval NRF_ERROR_INVALID_STATE If the driver is in invalid state.
 */
uint32_t drv_range_init(drv_range_init_t * p_params);

/**@brief Function for enabling the range sensor.
 *
 * @retval NRF_SUCCESS             If initialization was successful.
 */
uint32_t drv_range_enable(void);

/**@brief Function for disabling the range sensor.
 *
 * @retval NRF_SUCCESS             If initialization was successful.
 */
uint32_t drv_range_disable(void);

/**@brief Function for resetting the chip to all default register values.
*
* @retval NRF_SUCCESS             If operation was successful.
* @retval NRF_ERROR_BUSY          If TWI bus was busy.
*/
uint32_t drv_range_reset(void);

/**@brief Function for changing the mode of the range sensor.
 *
 * @param[in] mode                 Altimeter or Barometer.
 *
 * @retval NRF_SUCCESS             If configuration was successful.
 * @retval NRF_ERROR_BUSY          If the TWI drivers are busy.
 */
uint32_t drv_range_mode_set(drv_range_mode_t mode);

/**@brief Function for getting the presence data [hPa].
 *
 * @retval Pressure data.
 */
uint32_t drv_range_get(ble_dds_range_t * range);

/**@brief Function for starting the sampling.
 *
 * @retval NRF_SUCCESS             If start sampling was successful.
 */
uint32_t drv_range_sample(void);

/**@brief Function for putting the sensor to sleep.
 *
 * @retval NRF_SUCCESS             If sleep was successful.
 */
uint32_t drv_range_sleep(void);

#endif