#include "drv_ak9750.h"
#include "twi_manager.h"
#include "nrf_log.h"
#include "nrf_delay.h"

#define ECNTL1              0x1C            // Mode setting/ Digital Filter Cutoff Frequency (Fc) setting (Read / Write registers)
#define CNTL2               0x1D            // Soft Reset (Read / Write Registers)
#define EINTEN              0x1B            // Interrupt Source setting (Read / Write Registers)
#define INTST               0x04            // Interrupt Status (Read Only Registers)
#define ST1                 0x05            // Status 1 (Read Only Registers)
#define ST2                 0x10            // Status 2 (Read Only Registers)
#define IR1L                0x06            // IR1 A/D Converted data (Low)
#define IR1H                0x07            // IR1 A/D Converted data (High)
#define IR2L                0x08            // IR2 A/D Converted data (Low)
#define IR2H                0x09            // IR2 A/D Converted data (High)
#define IR3L                0x0A            // IR3 A/D Converted data (Low)
#define IR3H                0x0B            // IR3 A/D Converted data (High)
#define IR4L                0x0C            // IR4 A/D Converted data (Low)
#define IR4H                0x0D            // IR4 A/D Converted data (High)

#define INTST_DRI_MASK      0x01

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
//    RETURN_IF_ERROR(err_code);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(m_ak9750.p_cfg->p_twi_instance);

    NRF_LOG_INFO("END TWI OPEN!!!!! \r\n");

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

ret_code_t i2c_read(uint8_t regAddr, uint8_t * pdata, size_t size)
{
    ret_code_t ret;
    do
    {
       ret = nrf_drv_twi_tx(m_ak9750.p_cfg->p_twi_instance, m_ak9750.p_cfg->twi_addr, &regAddr, 1, true);
       if (NRF_SUCCESS != ret)
       {
           break;
       }
       ret = nrf_drv_twi_rx(m_ak9750.p_cfg->p_twi_instance, m_ak9750.p_cfg->twi_addr, pdata, size);
    }while (0);
    return ret;
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

    NRF_LOG_INFO("VERIFY START!!!!! \r\n");

    DRV_CFG_CHECK(m_ak9750.p_cfg);

    NRF_LOG_INFO("DRV CHECK PASS!!!!! \r\n");

    err_code = reg_read(DEVICE_ID, who_am_i);
    NRF_LOG_INFO("ERROR: %d \r\n", err_code);
    RETURN_IF_ERROR(err_code);

    NRF_LOG_INFO("READ PASS!!!!! \r\n");

    NRF_LOG_INFO("who_am_i: %d", *who_am_i);

    return (*who_am_i == DEVICE_ID_VALUE) ? NRF_SUCCESS : NRF_ERROR_NOT_FOUND;
}

uint32_t drv_ak9750_reset(void)
{
    uint32_t err_code;

    err_code = reg_write(CNTL2, 0xFF);
    RETURN_IF_ERROR(err_code);

    return NRF_SUCCESS;
}

uint32_t drv_ak9750_init()
{
    uint32_t err_code;
    uint8_t  dummy;

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
    err_code = reg_write(EINTEN, 0xC1);
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
    uint8_t  reg_val;
    uint8_t  dummy;
    uint8_t st1[1];
    int iTimeout = 0;

    DRV_CFG_CHECK(m_ak9750.p_cfg);

    err_code = reg_write(ECNTL1, 0xAA);
    RETURN_IF_ERROR(err_code);

    err_code = reg_read(ECNTL1, &dummy);
    RETURN_IF_ERROR(err_code);
    NRF_LOG_INFO("ECNTL1 %d \r\n", dummy);

    // do{
    //     err_code = reg_read(ST1, &st1);
    //     RETURN_IF_ERROR(err_code);
    //     iTimeout++;
    //     //usleep(5000);
    //     nrf_delay_ms(1);
    //     if (iTimeout > 100) 
    //     {
    //         NRF_LOG_RAW_INFO("\nAK Timeout\n");
    //         return 1; 
    //     }
    // }while(!(st1[0] & (1<<0)));

    

    // err_code = reg_read(IR1L, &dummy);
    // RETURN_IF_ERROR(err_code);
    // NRF_LOG_INFO("IR1L %d \r\n", dummy);

    // err_code = reg_read(IR1H, &dummy);
    // RETURN_IF_ERROR(err_code);
    // NRF_LOG_INFO("IR1H %d \r\n", dummy);

    // // if (reg_val & INTST_DRI_MASK)
    // // {
    // //     return NRF_ERROR_BUSY;
    // // }

    // //reg_val |= CTRL_REG2_ONE_SHOT_Msk;

    // //err_code = reg_write(CTRL_REG2, reg_val);
    // //RETURN_IF_ERROR(err_code);

    // err_code = reg_read(ST1, &dummy);
    // RETURN_IF_ERROR(err_code);

    // err_code = reg_read(ST2, &dummy);
    // RETURN_IF_ERROR(err_code);

    // err_code = reg_read(ECNTL1, &dummy);
    // RETURN_IF_ERROR(err_code);
    // NRF_LOG_INFO("ECNTL1 2 %d \r\n", dummy);

    return NRF_SUCCESS;
}

uint32_t drv_ak9750_get_irs(int16_t *ir1, int16_t *ir2, int16_t *ir3, int16_t *ir4)
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
    NRF_LOG_INFO("IR1L %d \r\n", ir1_l);

    err_code = reg_read(IR1H, &ir1_h);
    RETURN_IF_ERROR(err_code);
    NRF_LOG_INFO("IR1H %d \r\n", ir1_h);

    // IR2
    err_code = reg_read(IR2L, &ir2_l);
    RETURN_IF_ERROR(err_code);
    NRF_LOG_INFO("IR2L %d \r\n", ir2_l);

    err_code = reg_read(IR2H, &ir2_h);
    RETURN_IF_ERROR(err_code);
    NRF_LOG_INFO("IR2H %d \r\n", ir2_h);

    // IR3
    err_code = reg_read(IR3L, &ir3_l);
    RETURN_IF_ERROR(err_code);
    NRF_LOG_INFO("IR3L %d \r\n", ir3_l);

    err_code = reg_read(IR3H, &ir3_h);
    RETURN_IF_ERROR(err_code);
    NRF_LOG_INFO("IR3H %d \r\n", ir3_h);

    // IR4
    err_code = reg_read(IR4L, &ir4_l);
    RETURN_IF_ERROR(err_code);
    NRF_LOG_INFO("IR4L %d \r\n", ir4_l);

    err_code = reg_read(IR4H, &ir4_h);
    RETURN_IF_ERROR(err_code);
    NRF_LOG_INFO("IR4H %d \r\n", ir4_h);

    // Read Dummy Reg
    err_code = reg_read(ST1, &dummy);
    RETURN_IF_ERROR(err_code);

    err_code = reg_read(ST2, &dummy);
    RETURN_IF_ERROR(err_code);

    return NRF_SUCCESS;
}

uint32_t get_drv_ak9750_values(int16_t *ir1, int16_t *ir2, int16_t *ir3, int16_t *ir4)
{
    int iTimeout = 0;
    uint8_t dat1[2];
    uint8_t dat2[2];
    uint8_t dat3[2];
    uint8_t dat4[2];
    uint8_t st1[1];
    uint8_t dummy_data[1];
    //int16_t ir1;
    //int16_t ir2;
    //int16_t ir3;
    //int16_t ir4;
    //uint8_t write[2] = {0x1C, 0xAC};

    //Set ECNTL1 Mode Settings
    //nrf_drv_twi_tx(m_twi_master, 0x64, write, 2, true);
    //i2c_write(AK9750_ADDR, write, 2, true);

    //Refresh Dummy Read for IRx's
    i2c_read(0x10, dummy_data, 1);

    do{
        i2c_read(0x05, st1, 1);
        iTimeout++;
        //usleep(5000);
        nrf_delay_ms(1);
        if (iTimeout > 100) 
        {
            NRF_LOG_RAW_INFO("\nAK Timeout\n");
            *ir1 = 8888;
            *ir2 = 8888;
            *ir3 = 8888;
            *ir4 = 8888;
            return 1; 
        }
    }while(!(st1[0] & (1<<0)));


    i2c_read(0x06, dat1, 2);
    i2c_read(0x08, dat2, 2);
    i2c_read(0x0A, dat3, 2);
    i2c_read(0x0C, dat4, 2);

    *ir1 = dat1[0] + (dat1[1] << 8);
    NRF_LOG_RAW_INFO("\nIR1: %d  \n", ir1);

    *ir2 = dat2[0] + (dat2[1] << 8);
    NRF_LOG_RAW_INFO("IR2: %d  \n", ir2);

    *ir3 = dat3[0] + (dat3[1] << 8);
    NRF_LOG_RAW_INFO("IR3: %d  \n", ir3);

    *ir4 = dat4[0] + (dat4[1] << 8);
    NRF_LOG_RAW_INFO("IR4: %d  \n", ir4);

    //Read ST2 Register Dummy
    i2c_read(0x10, dummy_data, 1);

    return NRF_SUCCESS;
}

void drv_ak9750_print_values(void)
{
    int iTimeout = 0;
    uint8_t dat1[2];
    uint8_t dat2[2];
    uint8_t dat3[2];
    uint8_t dat4[2];
    uint8_t st1[1];
    uint8_t dummy_data[1];
    int16_t ir1;
    int16_t ir2;
    int16_t ir3;
    int16_t ir4;

    //Refresh Dummy Read for IRx's
    i2c_read(0x10, dummy_data, 1);

    do{
        i2c_read(0x05, st1, 1);
        iTimeout++;
        //usleep(5000);
        nrf_delay_ms(1);
        if (iTimeout > 100) 
        {
            NRF_LOG_RAW_INFO("\n*********************AK Timeout*********************\n");
            ir1 = 8888;
            ir2 = 8888;
            ir3 = 8888;
            ir4 = 8888;
            return 1; 
        }
    }while(!(st1[0] & (1<<0)));


    i2c_read(0x06, dat1, 2);
    i2c_read(0x08, dat2, 2);
    i2c_read(0x0A, dat3, 2);
    i2c_read(0x0C, dat4, 2);

    ir1 = dat1[0] + (dat1[1] << 8);
    NRF_LOG_RAW_INFO("\nIR1: %d  \n", ir1);

    ir2 = dat2[0] + (dat2[1] << 8);
    NRF_LOG_RAW_INFO("IR2: %d  \n", ir2);

    ir3 = dat3[0] + (dat3[1] << 8);
    NRF_LOG_RAW_INFO("IR3: %d  \n", ir3);

    ir4 = dat4[0] + (dat4[1] << 8);
    NRF_LOG_RAW_INFO("IR4: %d  \n", ir4);

    //Read ST2 Register Dummy
    i2c_read(0x10, dummy_data, 1);
}