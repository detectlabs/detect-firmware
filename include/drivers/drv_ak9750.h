#ifndef __AK9750_H__
#define __AK9750_H__

#include "macros.h"
#include "nrf_drv_twi.h"
#include <stdint.h>
#include "ble_dds.h"
#include "drv_range.h"

/**@brief Device WHO_AM_I register. */
#define DEVICE_ID                            0x01
#define DEVICE_ID_VALUE                      0x13

/**@brief Configuration struct for the AK9750 presence sensor.
 */
typedef struct
{
    uint8_t  int_cfg_reg;
    uint16_t int_threshold;
    uint8_t  ctrl_reg1;
    uint8_t  ctrl_reg2;
    uint8_t  ctrl_reg3;
    uint8_t  fifo_ctrl;
    uint8_t  res_conf;
}drv_ak9750_cfg_t;

/**@brief Initialization struct for the AK9750 presence sensor driver.
 */
typedef struct
{
    uint8_t                      twi_addr;        ///< TWI address.
    uint32_t                     pin_int;         ///< Interrupt pin number.
    nrf_drv_twi_t        const * p_twi_instance;  ///< The instance of TWI master to be used for transactions.
    nrf_drv_twi_config_t const * p_twi_cfg;       ///< The TWI configuration to use while the driver is enabled.
} drv_ak9750_twi_cfg_t;

uint32_t drv_ak9750_disable_dri(void);

uint32_t drv_ak9750_enable_dri(void);

uint32_t drv_ak9750_clear_int(void);

uint32_t drv_ak9750_read_int(uint8_t * status);

uint32_t drv_ak9750_cfg_set(drv_range_mode_t mode);

uint32_t drv_ak9750_get_irs(ble_dds_presence_t * presence);

uint32_t drv_ak9750_one_shot(void);

uint32_t drv_ak9750_open(drv_ak9750_twi_cfg_t const * const p_cfg);

uint32_t drv_ak9750_verify(uint8_t * who_am_i);

uint32_t drv_ak9750_reset(void);

uint32_t drv_ak9750_init(void);

uint32_t drv_ak9750_close(void);

uint32_t get_drv_ak9750_values(int16_t *ir1, int16_t *ir2, int16_t *ir3, int16_t *ir4);

#endif