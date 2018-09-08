#ifndef __BLE_DDS_H__
#define __BLE_DDS_H__

#include "ble.h"
#include <stdint.h>
#include <string.h>

#include "app_util_platform.h"

#ifdef __GNUC__
    #ifdef PACKED
        #undef PACKED
    #endif

    #define PACKED(TYPE) TYPE __attribute__ ((packed))
#endif

typedef enum
{
    SAMPLE_MODE_CONTINUOUS,
    SAMPLE_MODE_MOTION,
} ble_dds_sample_mode_t;

typedef PACKED( struct
{
    int16_t  eth13h;
    int16_t  eth13l;
    int16_t  eth24h;
    int16_t  eth24l;
}) ble_dds_threshold_config_t;

typedef PACKED( struct
{
    uint16_t                presence_interval_ms;
    uint16_t                   range_interval_ms;
    ble_dds_sample_mode_t            sample_mode;
    ble_dds_threshold_config_t  threshold_config;
}) ble_dds_config_t;

#define BLE_DDS_CONFIG_PRESENCE_INT_MIN      100
#define BLE_DDS_CONFIG_PRESENCE_INT_MAX    60000
#define BLE_DDS_CONFIG_RANGE_INT_MIN          50
#define BLE_DDS_CONFIG_RANGE_INT_MAX       60000
#define BLE_DDS_CONFIG_THRESHOLD_MIN       -2048
#define BLE_DDS_CONFIG_THRESHOLD_MAX        2047

#endif