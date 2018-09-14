#ifndef __VL53L0X_H__
#define __VL53L0X_H__

#include "macros.h"
#include "nrf_drv_twi.h"
#include <stdint.h>

/**@brief Device WHO_AM_I register. */
#define DEVICE_ID                            0xC0
#define DEVICE_ID_VALUE                      0xEE

/**@brief Configuration struct for the VL53L0X presence sensor.
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
}drv_vl53l0x_cfg_t;

/**@brief Initialization struct for the VL53L0X presence sensor driver.
 */
typedef struct
{
    uint8_t                      twi_addr;        ///< TWI address.
    uint32_t                     pin_int;         ///< Interrupt pin number.
    nrf_drv_twi_t        const * p_twi_instance;  ///< The instance of TWI master to be used for transactions.
    nrf_drv_twi_config_t const * p_twi_cfg;       ///< The TWI configuration to use while the driver is enabled.
} drv_vl53l0x_twi_cfg_t;

uint32_t drv_vl53l0x_open(drv_vl53l0x_twi_cfg_t const * const p_cfg);


uint32_t drv_vl53l0x_verify(uint8_t * who_am_i);


uint32_t drv_vl53l0x_reset(void);


uint32_t drv_vl53l0x_init();


uint32_t drv_vl53l0x_close(void);


uint32_t get_drv_vl53l0x_values(int16_t *ir1, int16_t *ir2, int16_t *ir3, int16_t *ir4);

    enum regAddr
    {
      SYSRANGE_START                              = 0x00,

      SYSTEM_THRESH_HIGH                          = 0x0C,
      SYSTEM_THRESH_LOW                           = 0x0E,

      SYSTEM_SEQUENCE_CONFIG                      = 0x01,
      SYSTEM_RANGE_CONFIG                         = 0x09,
      SYSTEM_INTERMEASUREMENT_PERIOD              = 0x04,

      SYSTEM_INTERRUPT_CONFIG_GPIO                = 0x0A,

      GPIO_HV_MUX_ACTIVE_HIGH                     = 0x84,

      SYSTEM_INTERRUPT_CLEAR                      = 0x0B,

      RESULT_INTERRUPT_STATUS                     = 0x13,
      RESULT_RANGE_STATUS                         = 0x14,

      RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN       = 0xBC,
      RESULT_CORE_RANGING_TOTAL_EVENTS_RTN        = 0xC0,
      RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF       = 0xD0,
      RESULT_CORE_RANGING_TOTAL_EVENTS_REF        = 0xD4,
      RESULT_PEAK_SIGNAL_RATE_REF                 = 0xB6,

      ALGO_PART_TO_PART_RANGE_OFFSET_MM           = 0x28,

      I2C_SLAVE_DEVICE_ADDRESS                    = 0x8A,

      MSRC_CONFIG_CONTROL                         = 0x60,

      PRE_RANGE_CONFIG_MIN_SNR                    = 0x27,
      PRE_RANGE_CONFIG_VALID_PHASE_LOW            = 0x56,
      PRE_RANGE_CONFIG_VALID_PHASE_HIGH           = 0x57,
      PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT          = 0x64,

      FINAL_RANGE_CONFIG_MIN_SNR                  = 0x67,
      FINAL_RANGE_CONFIG_VALID_PHASE_LOW          = 0x47,
      FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         = 0x48,
      FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44,

      PRE_RANGE_CONFIG_SIGMA_THRESH_HI            = 0x61,
      PRE_RANGE_CONFIG_SIGMA_THRESH_LO            = 0x62,

      PRE_RANGE_CONFIG_VCSEL_PERIOD               = 0x50,
      PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          = 0x51,
      PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO          = 0x52,

      SYSTEM_HISTOGRAM_BIN                        = 0x81,
      HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT       = 0x33,
      HISTOGRAM_CONFIG_READOUT_CTRL               = 0x55,

      FINAL_RANGE_CONFIG_VCSEL_PERIOD             = 0x70,
      FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        = 0x71,
      FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO        = 0x72,
      CROSSTALK_COMPENSATION_PEAK_RATE_MCPS       = 0x20,

      MSRC_CONFIG_TIMEOUT_MACROP                  = 0x46,

      SOFT_RESET_GO2_SOFT_RESET_N                 = 0xBF,
      IDENTIFICATION_MODEL_ID                     = 0xC0,
      IDENTIFICATION_REVISION_ID                  = 0xC2,

      OSC_CALIBRATE_VAL                           = 0xF8,

      GLOBAL_CONFIG_VCSEL_WIDTH                   = 0x32,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_0            = 0xB0,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_1            = 0xB1,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_2            = 0xB2,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_3            = 0xB3,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_4            = 0xB4,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_5            = 0xB5,

      GLOBAL_CONFIG_REF_EN_START_SELECT           = 0xB6,
      DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         = 0x4E,
      DYNAMIC_SPAD_REF_EN_START_OFFSET            = 0x4F,
      POWER_MANAGEMENT_GO1_POWER_FORCE            = 0x80,

      VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV           = 0x89,

      ALGO_PHASECAL_LIM                           = 0x30,
      ALGO_PHASECAL_CONFIG_TIMEOUT                = 0x30,
    };

    // private variables

    typedef struct 
    {
      bool tcc, msrc, dss, pre_range, final_range;
    } SequenceStepEnables;



    typedef struct 
    {
      int16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;

      int16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
      int32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
    } SequenceStepTimeouts;

    void getSequenceStepEnables(SequenceStepEnables * enables);
    void getSequenceStepTimeouts(SequenceStepEnables * enables, SequenceStepTimeouts * timeouts);
    
    
    enum vcselPeriodType { VcselPeriodPreRange = 0, VcselPeriodFinalRange = 1 };

    typedef enum vcselPeriodType vcselPeriodType;

    int8_t address;
    int16_t io_timeout;
    bool did_timeout;
    int16_t timeout_start_ms;

    int8_t stop_variable; // read by init and used when starting measurement; is StopVariable field of VL53L0X_DevData_t structure in API
    int32_t measurement_timing_budget_us;

    bool getSpadInfo(int8_t * count, bool * type_is_aperture);

    int8_t last_status; // status of last I2C transmission

    VL53L0X(void);

    void setAddress(int8_t new_addr);
    //int8_t getAddress(void) { return address; }

    bool vl53l0x_init(bool io_2v8);
    bool init2();

    void writeReg(int8_t reg, int8_t value);
    void writeReg16Bit(int8_t reg, int16_t value);
    void writeReg32Bit(int8_t reg, int32_t value);
    int8_t readReg(int8_t reg);
    int16_t readReg16Bit(int8_t reg);
    int32_t readReg32Bit(int8_t reg);

    void writeMulti(int8_t reg, int8_t  * src, int8_t count);
    void readMulti(int8_t reg, int8_t * dst, int8_t count);

    bool setSignalRateLimit(float limit_Mcps);
    float getSignalRateLimit(void);

    bool setMeasurementTimingBudget(int32_t budget_us);
    int32_t getMeasurementTimingBudget(void);

    bool setVcselPulsePeriod(vcselPeriodType type, int8_t period_pclks);
    int8_t getVcselPulsePeriod(vcselPeriodType type);

    void startContinuous(int32_t period_ms); // = 0);
    void stopContinuous(void);
    int16_t readRangeContinuousMillimeters(void);
   //int16_t readRangeSingleMillimeters(void);
    void readRangeSingleMillimeters(void);

    //void setTimeout(int16_t timeout) { io_timeout = timeout; }
    //int16_t getTimeout(void) { return io_timeout; }
    bool timeoutOccurred(void);



    bool performSingleRefCalibration(int8_t vhv_init_byte);

    static int16_t decodeTimeout(int16_t value);
    static int16_t encodeTimeout(int16_t timeout_mclks);
    static int32_t timeoutMclksToMicroseconds(int16_t timeout_period_mclks, int8_t vcsel_period_pclks);
    static int32_t timeoutMicrosecondsToMclks(int32_t timeout_period_us, int8_t vcsel_period_pclks);


#endif