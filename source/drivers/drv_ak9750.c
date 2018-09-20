#include "drv_ak9750.h"
#include "twi_manager.h"
#include "nrf_log.h"
#include "nrf_delay.h"
#include "ble_dds.h"

#define ECNTL1                         0x1C            // Mode setting/ Digital Filter Cutoff Frequency (Fc) setting (Read / Write registers)
#define CNTL2                          0x1D            // Soft Reset (Read / Write Registers)
#define EINTEN                         0x1B            // Interrupt Source setting (Read / Write Registers)
#define INTST                          0x04            // Interrupt Status (Read Only Registers)
#define ST1                            0x05            // Status 1 (Read Only Registers)
#define ST2                            0x10            // Status 2 (Read Only Registers)
#define IR1L                           0x06            // IR1 A/D Converted data (Low)
#define IR1H                           0x07            // IR1 A/D Converted data (High)
#define IR2L                           0x08            // IR2 A/D Converted data (Low)
#define IR2H                           0x09            // IR2 A/D Converted data (High)
#define IR3L                           0x0A            // IR3 A/D Converted data (Low)
#define IR3H                           0x0B            // IR3 A/D Converted data (High)
#define IR4L                           0x0C            // IR4 A/D Converted data (Low)
#define IR4H                           0x0D            // IR4 A/D Converted data (High)

#define INTST_DRI_MASK                 0x01
#define NORMAL_FC_8_8_SINGLE_SHOT_MODE 0xAA
#define DRI_ENABLE_ONLY                0xC1
#define SRST                           0xFF

/**@brief Check if the driver is open, if not return NRF_ERROR_INVALID_STATE.
 */
#define DRV_CFG_CHECK(PARAM)                                                                      \
        if ((PARAM) == NULL)                                                                      \
        {                                                                                         \
            return NRF_ERROR_INVALID_STATE;                                                       \
        }

/**@brief TWI configuration.
 */
static struct
{
    drv_ak9750_twi_cfg_t const * p_cfg;
} m_ak9750;


/**@brief Open the TWI bus for communication.
 */
static __inline uint32_t twi_open(void)
{
    uint32_t err_code;

    err_code = twi_manager_request(m_ak9750.p_cfg->p_twi_instance,
                                   m_ak9750.p_cfg->p_twi_cfg,
                                   NULL,
                                   NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(m_ak9750.p_cfg->p_twi_instance);

    return NRF_SUCCESS;
}

/**@brief Function to deinit the TWI module when this driver does not need to
 *        communicate on the TWI bus, so that other drivers can use the module.
 */
static __inline uint32_t twi_close(void)
{
    nrf_drv_twi_disable(m_ak9750.p_cfg->p_twi_instance);

    nrf_drv_twi_uninit(m_ak9750.p_cfg->p_twi_instance);

    return NRF_SUCCESS;
}

/**@brief Function for writing to a sensor register.
 *
 * @param[in]  reg_addr            Address of the register to write to.
 * @param[in]  reg_val             Value to write to the register.
 *
 * @retval NRF_SUCCESS             If operation was successful.
 * @retval NRF_ERROR_BUSY          If the TWI drivers are busy.
 */
static uint32_t reg_write(uint8_t reg_addr, uint8_t reg_val)
{
    uint32_t err_code;

    uint8_t buffer[2] = {reg_addr, reg_val};

    err_code = nrf_drv_twi_tx( m_ak9750.p_cfg->p_twi_instance,
                               m_ak9750.p_cfg->twi_addr,
                               buffer,
                               2,
                               false );
    RETURN_IF_ERROR(err_code);

    return NRF_SUCCESS;
}

/**@brief Function for reading a sensor register.
 *
 * @param[in]  reg_addr            Address of the register to read.
 * @param[out] p_reg_val           Pointer to a buffer to receive the read value.
 *
 * @retval NRF_SUCCESS             If operation was successful.
 * @retval NRF_ERROR_BUSY          If the TWI drivers are busy.
 */
static uint32_t reg_read(uint8_t reg_addr, uint8_t * p_reg_val)
{
    uint32_t err_code;

    err_code = nrf_drv_twi_tx( m_ak9750.p_cfg->p_twi_instance,
                               m_ak9750.p_cfg->twi_addr,
                               &reg_addr,
                               1,
                               true );
    RETURN_IF_ERROR(err_code);

    err_code = nrf_drv_twi_rx( m_ak9750.p_cfg->p_twi_instance,
                               m_ak9750.p_cfg->twi_addr,
                               p_reg_val,
                               1 );
    RETURN_IF_ERROR(err_code);

    return NRF_SUCCESS;
}

uint32_t drv_ak9750_open(drv_ak9750_twi_cfg_t const * const p_cfg)
{
    m_ak9750.p_cfg = p_cfg;

    return twi_open();
}

uint32_t drv_ak9750_verify(uint8_t * who_am_i)
{
    uint32_t err_code;

    DRV_CFG_CHECK(m_ak9750.p_cfg);

    err_code = reg_read(DEVICE_ID, who_am_i);
    RETURN_IF_ERROR(err_code);
    // NRF_LOG_INFO("who_am_i: %d", *who_am_i);

    return (*who_am_i == DEVICE_ID_VALUE) ? NRF_SUCCESS : NRF_ERROR_NOT_FOUND;
}

uint32_t drv_ak9750_reset(void)
{
    uint32_t err_code;

    err_code = reg_write(CNTL2, SRST);
    RETURN_IF_ERROR(err_code);

    return NRF_SUCCESS;
}

uint32_t drv_ak9750_init()
{
    DRV_CFG_CHECK(m_ak9750.p_cfg);

    drv_ak9750_reset();

    return NRF_SUCCESS;
}

uint32_t drv_ak9750_cfg_set(void)
{
    uint32_t err_code;
    uint8_t  dummy;

    DRV_CFG_CHECK(m_ak9750.p_cfg);

    // Enable Interrupt for DRI
    err_code = reg_write(EINTEN, DRI_ENABLE_ONLY);
    RETURN_IF_ERROR(err_code);

    // Read Interrupt Status
    err_code = reg_read(INTST, &dummy);
    RETURN_IF_ERROR(err_code);

    return NRF_SUCCESS;
}

uint32_t drv_ak9750_close(void)
{
    uint32_t err_code = twi_close();

    m_ak9750.p_cfg = NULL;

    return err_code;
}

uint32_t drv_ak9750_one_shot(void)
{
    uint32_t err_code;

    DRV_CFG_CHECK(m_ak9750.p_cfg);

    // Set Mode Reg
    err_code = reg_write(ECNTL1, NORMAL_FC_8_8_SINGLE_SHOT_MODE);
    RETURN_IF_ERROR(err_code);
 
    // Read Mode Reg
    //err_code = reg_read(ECNTL1, &dummy);
    //RETURN_IF_ERROR(err_code);
    //NRF_LOG_INFO("ECNTL1 %d \r\n", dummy);

    return NRF_SUCCESS;
}

uint32_t drv_ak9750_get_irs(ble_dds_presence_t * presence)
{
    uint32_t err_code;
    uint8_t ir1_l;
    uint8_t ir1_h;
    uint8_t ir2_l;
    uint8_t ir2_h;
    uint8_t ir3_l;
    uint8_t ir3_h;
    uint8_t ir4_l;
    uint8_t ir4_h;
    uint8_t  dummy;

    DRV_CFG_CHECK(m_ak9750.p_cfg);

    err_code = reg_read(ST1, &dummy);
    RETURN_IF_ERROR(err_code);

    // IR1
    err_code = reg_read(IR1L, &ir1_l);
    RETURN_IF_ERROR(err_code);
    //NRF_LOG_INFO("IR1L %d \r\n", ir1_l);

    err_code = reg_read(IR1H, &ir1_h);
    RETURN_IF_ERROR(err_code);
    //NRF_LOG_INFO("IR1H %d \r\n", ir1_h);

    // IR2
    err_code = reg_read(IR2L, &ir2_l);
    RETURN_IF_ERROR(err_code);
    //NRF_LOG_INFO("IR2L %d \r\n", ir2_l);

    err_code = reg_read(IR2H, &ir2_h);
    RETURN_IF_ERROR(err_code);
    //NRF_LOG_INFO("IR2H %d \r\n", ir2_h);

    // IR3
    err_code = reg_read(IR3L, &ir3_l);
    RETURN_IF_ERROR(err_code);
    //NRF_LOG_INFO("IR3L %d \r\n", ir3_l);

    err_code = reg_read(IR3H, &ir3_h);
    RETURN_IF_ERROR(err_code);
    //NRF_LOG_INFO("IR3H %d \r\n", ir3_h);

    // IR4
    err_code = reg_read(IR4L, &ir4_l);
    RETURN_IF_ERROR(err_code);
    //NRF_LOG_INFO("IR4L %d \r\n", ir4_l);

    err_code = reg_read(IR4H, &ir4_h);
    RETURN_IF_ERROR(err_code);
    //NRF_LOG_INFO("IR4H %d \r\n", ir4_h);

    // Read Dummy Reg
    err_code = reg_read(ST1, &dummy);
    RETURN_IF_ERROR(err_code);

    err_code = reg_read(ST2, &dummy);
    RETURN_IF_ERROR(err_code);

    presence->ir1 = ir1_l + (ir1_h << 8);
    NRF_LOG_RAW_INFO("\nIR1: %d  \n", presence->ir1);

    presence->ir2 = ir2_l + (ir2_h << 8);
    NRF_LOG_RAW_INFO("IR2: %d  \n", presence->ir2);

    presence->ir3 = ir3_l + (ir3_h << 8);
    NRF_LOG_RAW_INFO("IR3: %d  \n", presence->ir3);

    presence->ir4 = ir4_l + (ir4_h << 8);
    NRF_LOG_RAW_INFO("IR4: %d  \n", presence->ir4);

    return NRF_SUCCESS;
}