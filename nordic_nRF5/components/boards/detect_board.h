/**
 * Copyright (c) 2016 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#ifndef PCA10056_H
#define PCA10056_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LEDs definitions for PCA10056
#define LEDS_NUMBER    3

#define LED_1          NRF_GPIO_PIN_MAP(1,06)
#define LED_2          NRF_GPIO_PIN_MAP(1,04)
#define LED_3          NRF_GPIO_PIN_MAP(1,02)

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3

#define LEDS_ACTIVE_STATE 0

#define LEDS_LIST { LED_1, LED_2, LED_3}

#define LEDS_INV_MASK  LEDS_MASK

#define BUTTONS_NUMBER 1

#define BUTTON_1       NRF_GPIO_PIN_MAP(1,07)

#define BSP_BUTTON_0   BUTTON_1

#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1}

#define TWI_SDA                             21
#define TWI_SCL                             19
#define MASTER_TWI_INST                     0
#define TWI_ADDRESS_CONFIG                  LITTLE_ENDIAN

#define AK9750_ADDR 0x64
#define AK9750_INT 22

#define VL53L0X_ADDR 0x29
#define VL53L0X_INT 20


// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#define BATT_VOLTAGE_DIVIDER_R1      1500000
#define BATT_VOLTAGE_DIVIDER_R2      180000

#define APP_TIMER_PRESCALER                 0                       /**< Value of the RTC1 PRESCALER register. */
#define BATTERY                             30                      /** If altered, BATTERY_AIN must be changed as well. */
#define BATTERY_AIN                         NRF_SAADC_INPUT_AIN6    /** Pin->AIN mapping func removed in SDK. Hard-coded AIN definition. */
#define BAT_MON_EN_PIN_USED                 true
#define BAT_MON_EN_PIN_NO                   12
#define BATT_MEAS_LOW_BATT_LIMIT_MV         2700                    // Cutoff voltage [mV].
#define BATT_MEAS_FULL_BATT_LIMIT_MV        3600                    // Full charge definition [mV].
#define BATT_MEAS_VOLTAGE_TO_SOC_ELEMENTS   111                     // Number of elements in the state of charge vector.
#define BATT_MEAS_VOLTAGE_TO_SOC_DELTA_MV   10                      // mV between each element in the SoC vector.

/** Converts voltage to state of charge (SoC) [%]. The first element corresponds to the voltage 
BATT_MEAS_LOW_BATT_LIMIT_MV and each element is BATT_MEAS_VOLTAGE_TO_SOC_DELTA_MV higher than the previous.
Numbers are obtained via model fed with experimental data. */
static const uint8_t BATT_MEAS_VOLTAGE_TO_SOC[] = { 
 0, 0, 0, 0, 0, 0, 0, 0, 0,
 7, 7, 7, 7, 7, 7, 7, 7, 7,
 13, 13, 13, 13, 13, 13, 13, 13, 13,
 28, 28, 28, 28, 28, 28, 28, 28, 28,
 42, 42, 42, 42, 42, 42, 42, 42, 42,
 48, 48, 48, 48, 48, 48, 48, 48, 48,
 56, 56, 56, 56, 56, 56, 56, 56, 56,
 71, 71, 71, 71, 71, 71, 71, 71, 71,
 100, 100, 100, 100, 100, 100, 100, 100, 100,};

// Battery monitoring setup.
#define BATT_MEAS_PARAM_CFG                                             \
{                                                                       \
    .batt_meas_param = {                                                \
        .app_timer_prescaler        = APP_TIMER_PRESCALER,              \
        .adc_pin_no                 = BATTERY,                          \
        .adc_pin_no_ain             = BATTERY_AIN,                      \
        .batt_mon_en_pin_used       = BAT_MON_EN_PIN_USED,              \
        .batt_mon_en_pin_no         = BAT_MON_EN_PIN_NO,                \
        .batt_voltage_limit_low     = BATT_MEAS_LOW_BATT_LIMIT_MV,      \
        .batt_voltage_limit_full    = BATT_MEAS_FULL_BATT_LIMIT_MV,     \
        .state_of_charge =                                              \
        {                                                               \
            .num_elements           = BATT_MEAS_VOLTAGE_TO_SOC_ELEMENTS,\
            .first_element_mv       = BATT_MEAS_LOW_BATT_LIMIT_MV,      \
            .delta_mv               = BATT_MEAS_VOLTAGE_TO_SOC_DELTA_MV,\
            .voltage_to_soc         = BATT_MEAS_VOLTAGE_TO_SOC,         \
        },                                                              \
        .voltage_divider =                                              \
        {                                                               \
            .r_1_ohm                = BATT_VOLTAGE_DIVIDER_R1,          \
            .r_2_ohm                = BATT_VOLTAGE_DIVIDER_R2,          \
        },                                                              \
     },                                                                 \
};

#ifdef __cplusplus
}
#endif

#endif // PCA10056_H
