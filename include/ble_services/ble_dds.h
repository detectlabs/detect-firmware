#ifndef __BLE_DDS_H__
#define __BLE_DDS_H__

#include "ble.h"
#include <stdint.h>
#include <string.h>

#include "app_util_platform.h"

// EE84xxxx-43B7-4F65-9FB9-D7B92D683E36
#define DDS_BASE_UUID                  {{0x36, 0x3E, 0x68, 0x2D, 0xB9, 0xD7, 0xB9, 0x9F, 0x65, 0x4F, 0xB7, 0x43, 0x00, 0x00, 0x84, 0xEE}}

#define BLE_UUID_DDS_SERVICE            0x0200                      /**< The UUID of the Thingy Environment Service. */
#define BLE_UUID_DDS_PRESENCE_CHAR      0x0201                      /**< The UUID of the temperature Characteristic. */
#define BLE_UUID_DDS_RANGE_CHAR         0x0202                      /**< The UUID of the pressure Characteristic. */
#define BLE_UUID_DDS_CONFIG_CHAR        0x0203                      /**< The UUID of the config Characteristic. */

#define BLE_DDS_MAX_RX_CHAR_LEN        BLE_DDS_MAX_DATA_LEN        /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_DDS_MAX_TX_CHAR_LEN        BLE_DDS_MAX_DATA_LEN        /**< Maximum length of the TX Characteristic (in bytes). */

#define BLE_DDS_MAX_DATA_LEN (BLE_GATT_ATT_MTU_DEFAULT - 3) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Thingy Environment service module. */

#ifdef __GNUC__
    #ifdef PACKED
        #undef PACKED
    #endif

    #define PACKED(TYPE) TYPE __attribute__ ((packed))
#endif

typedef PACKED( struct
{
    uint32_t timestamp;
    uint8_t marker;
    int16_t ir1;
    int16_t ir2;
    int16_t ir3;
    int16_t ir4;
}) ble_dds_presence_t;

typedef PACKED( struct
{
    uint32_t timestamp;
    uint8_t marker;
    uint16_t range;
}) ble_dds_range_t;

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
    uint16_t                   range_interval_ms;
    uint16_t                presence_interval_ms;
    ble_dds_threshold_config_t  threshold_config;
    ble_dds_sample_mode_t            sample_mode;
}) ble_dds_config_t;

#define BLE_DDS_CONFIG_PRESENCE_INT_MIN       20
#define BLE_DDS_CONFIG_PRESENCE_INT_MAX      200
#define BLE_DDS_CONFIG_RANGE_INT_MIN          20
#define BLE_DDS_CONFIG_RANGE_INT_MAX         200
#define BLE_DDS_CONFIG_THRESHOLD_MIN       -2048
#define BLE_DDS_CONFIG_THRESHOLD_MAX        2047

typedef enum
{
    BLE_DDS_EVT_NOTIF_PRESENCE,
    BLE_DDS_EVT_NOTIF_RANGE,
    BLE_DDS_EVT_CONFIG_RECEIVED
}ble_dds_evt_type_t;

/* Forward declaration of the ble_tes_t type. */
typedef struct ble_dds_s ble_dds_t;

/**@brief Detect Detection Service event handler type. */
typedef void (*ble_dds_evt_handler_t) (ble_dds_t        * p_tes,
                                       ble_dds_evt_type_t evt_type,
                                       uint8_t         const * p_data,
                                       uint16_t        const    length);

/**@brief Detect Detection Service initialization structure.
 *
 * @details This structure contains the initialization information for the service. The application
 * must fill this structure and pass it to the service using the @ref ble_tes_init function.
 */
typedef struct
{
    ble_dds_presence_t * p_init_presence;
    ble_dds_range_t    * p_init_range;
    ble_dds_config_t      * p_init_config;
    ble_dds_evt_handler_t     evt_handler; /**< Event handler to be called for handling received data. */
} ble_dds_init_t;

/**@brief Detect Detection Service structure.
 *
 * @details This structure contains status information related to the service.
 */
struct ble_dds_s
{
    uint8_t                  uuid_type;                    /**< UUID type for Detect Detection Service Base UUID. */
    uint16_t                 service_handle;               /**< Handle of Detect Detection Service (as provided by the S110 SoftDevice). */
    ble_gatts_char_handles_t presence_handles;          /**< Handles related to the presence characteristic (as provided by the S132 SoftDevice). */
    ble_gatts_char_handles_t range_handles;             /**< Handles related to the range characteristic (as provided by the S132 SoftDevice). */
    ble_gatts_char_handles_t config_handles;               /**< Handles related to the config characteristic (as provided by the S132 SoftDevice). */
    uint16_t                 conn_handle;                  /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    bool                     is_presence_notif_enabled; /**< Variable to indicate if the peer has enabled notification of the characteristic.*/
    bool                     is_range_notif_enabled;    /**< Variable to indicate if the peer has enabled notification of the characteristic.*/
    ble_dds_evt_handler_t    evt_handler;                  /**< Event handler to be called for handling received data. */
};

void ble_dds_on_ble_evt(ble_dds_t * p_dds, ble_evt_t const * p_ble_evt);

uint32_t ble_dds_presence_set(ble_dds_t * p_tes, ble_dds_presence_t * p_data);

uint32_t ble_dds_range_set(ble_dds_t * p_tes, ble_dds_range_t * p_data);

uint32_t ble_dds_init(ble_dds_t * p_dds, const ble_dds_init_t * p_dds_init);

#endif