#ifndef __BLE_DCS_H__
#define __BLE_DCS_H__

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

// EE84xxxx-43B7-4F65-9FB9-D7B92D683E36
#define DCS_BASE_UUID                  {{0x36, 0x3E, 0x68, 0x2D, 0xB9, 0xD7, 0xB9, 0x9F, 0x65, 0x4F, 0xB7, 0x43, 0x00, 0x00, 0x84, 0xEE}}

#define BLE_UUID_DCS_DEVICE_NAME_CHAR   0x0101                      /**< The UUID of the device name Characteristic. */
#define BLE_UUID_DCS_ADV_PARAMS_CHAR    0x0102                      /**< The UUID of the advertising parameters Characteristic. */
#define BLE_UUID_DCS_CONN_PARAM_CHAR    0x0103                      /**< The UUID of the connection parameters Characteristic. */
#define BLE_UUID_DCS_FW_VERSION_CHAR    0x0104                      /**< The UUID of the FW version Characteristic. */

#define BLE_TCS_DEVICE_NAME_LEN_MAX 10

#define BLE_UUID_DCS_SERVICE 0x0100                      /**< The UUID of the Detect Configuration Service. */

/* File ID and Key used for the configuration record. */

#define CONFIG_FILE     (0xF010)
#define CONFIG_REC_KEY  (0x7010)

/* Colors used to print on the console. */

#define COLOR_GREEN     "\033[1;32m"
#define COLOR_YELLOW    "\033[1;33m"
#define COLOR_CYAN      "\033[1;36m"

/* Macros to print on the console using colors. */

#define NRF_LOG_CYAN(...)   NRF_LOG_INFO(COLOR_CYAN   __VA_ARGS__)
#define NRF_LOG_YELLOW(...) NRF_LOG_INFO(COLOR_YELLOW __VA_ARGS__)
#define NRF_LOG_GREEN(...)  NRF_LOG_INFO(COLOR_GREEN  __VA_ARGS__)

#define DEVICE_NAME                     "A0A0"                                      /**< Name of device. Will be included in the advertising data. */
#define APP_ADV_INTERVAL_MS             380                                         /**< The advertising interval in ms. */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                         /**< The advertising timeout in s. */

#define DCS_ADV_PARAMS_INTERVAL_MIN 32UL
#define DCS_ADV_PARAMS_INTERVAL_MAX 8000UL
#define DCS_ADV_PARAMS_TIMEOUT_MIN  0UL
#define DCS_ADV_PARAMS_TIMEOUT_MAX  180UL

#define MIN_CONN_INTERVAL_MS            7.5                                         /**< Minimum acceptable connection interval in ms. */
#define MAX_CONN_INTERVAL_MS            30                                          /**< Maximum acceptable connection interval in ms. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT_MS             10000                                        /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */

/**@brief Detect FW version.
* 0xFF indicates a custom build from source. 
Version numbers are changed for releases. */
#define DETECT_FW_VERSION_MAJOR     (0xFF)
#define DETECT_FW_VERSION_MINOR     (0xFF)
#define DETECT_FW_VERSION_PATCH     (0xFF)

/**@brief Thingy default configuration. */
#define DETECT_CONFIG_DEFAULT                         \
{                                                     \
    .dev_name =                                       \
    {                                                 \
        .name = DEVICE_NAME,                          \
        .len = 4                                      \
    },                                                \
    .adv_params =                                     \
    {                                                 \
        .interval = MSEC_TO_UNITS(APP_ADV_INTERVAL_MS, UNIT_0_625_MS),                  \
        .timeout = APP_ADV_TIMEOUT_IN_SECONDS         \
    },                                                \
    .conn_params =                                    \
    {                                                 \
        .min_conn_int  = (uint16_t)MSEC_TO_UNITS(MIN_CONN_INTERVAL_MS, UNIT_1_25_MS),   \
        .max_conn_int  = MSEC_TO_UNITS(MAX_CONN_INTERVAL_MS, UNIT_1_25_MS),             \
        .slave_latency = SLAVE_LATENCY,                                                 \
        .sup_timeout   = MSEC_TO_UNITS(CONN_SUP_TIMEOUT_MS, UNIT_10_MS)                 \
    },                                                \
    .fw_version =                                     \
    {                                                 \
        .major = DETECT_FW_VERSION_MAJOR,             \
        .minor = DETECT_FW_VERSION_MINOR,             \
        .patch = DETECT_FW_VERSION_PATCH              \
    }                                                 \
}

typedef PACKED( struct
{
    uint8_t name[BLE_TCS_DEVICE_NAME_LEN_MAX+1];
    uint8_t len;
}) ble_dcs_dev_name_t;

typedef PACKED( struct
{
    uint16_t interval;
    uint8_t  timeout;
}) ble_dcs_adv_params_t;

typedef PACKED( struct
{
    uint16_t min_conn_int;
    uint16_t max_conn_int;
    uint16_t slave_latency;
    uint16_t sup_timeout;
}) ble_dcs_conn_params_t;

typedef PACKED( struct
{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
}) ble_dcs_fw_version_t;

typedef enum
{
    BLE_DCS_EVT_DEV_NAME,
    BLE_DCS_EVT_ADV_PARAM,
    BLE_DCS_EVT_CONN_PARAM
}ble_dcs_evt_type_t;

/* Forward declaration of the ble_tcs_t type. */
typedef struct ble_dcs_s ble_dcs_t;

typedef struct
{
    ble_dcs_dev_name_t      dev_name;
    ble_dcs_adv_params_t    adv_params;
    ble_dcs_conn_params_t   conn_params;
    ble_dcs_fw_version_t    fw_version;
}ble_dcs_params_t;

/**@brief Detect Configuration Service event handler type. */
typedef void (*ble_dcs_evt_handler_t) (ble_dcs_t          * p_dcs,
                                       ble_dcs_evt_type_t evt_type,
                                       uint8_t  const     * p_data,
                                       uint16_t           length);

/**@brief Detect Configuration Service structure.
 *
 * @details This structure contains status information related to the service.
 */
struct ble_dcs_s
{
    uint8_t                  uuid_type;                    /**< UUID type for Detect Configuration Service Base UUID. */
    uint16_t                 service_handle;               /**< Handle of Thingy Configuration Service (as provided by the S110 SoftDevice). */
    ble_gatts_char_handles_t dev_name_handles;             /**< Handles related to the temperature characteristic (as provided by the S132 SoftDevice). */
    ble_gatts_char_handles_t adv_param_handles;            /**< Handles related to the pressure characteristic (as provided by the S132 SoftDevice). */
    ble_gatts_char_handles_t conn_param_handles;           /**< Handles related to the config characteristic (as provided by the S132 SoftDevice). */
    ble_gatts_char_handles_t fwv_handles;
    uint16_t                 conn_handle;                  /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_dcs_evt_handler_t    evt_handler;                  /**< Event handler to be called for handling received data. */
};

/**@brief Detect Configuration Service initialization structure.
 *
 * @details This structure contains the initialization information for the service. The application
 * must fill this structure and pass it to the service using the @ref ble_tcs_init function.
 */
typedef struct
{
    ble_dcs_params_t      * p_init_vals;
    ble_dcs_evt_handler_t   evt_handler; /**< Event handler to be called for handling received data. */
} ble_dcs_init_t;

uint32_t ble_dcs_init(ble_dcs_t * p_dcs, const ble_dcs_init_t * p_dcs_init);

/**@brief Function for handling the Thingy Configuration Service's BLE events.
 *
 * @details The Thingy Configuration Service expects the application to call this function each time an
 * event is received from the S110 SoftDevice. This function processes the event if it
 * is relevant and calls the Thingy Configuration Service event handler of the
 * application if necessary.
 *
 * @param[in] p_tcs       Thingy Configuration Service structure.
 * @param[in] p_ble_evt   Event received from the S110 SoftDevice.
 */
void ble_dcs_on_ble_evt(ble_dcs_t * p_dcs, ble_evt_t const * p_ble_evt);

#endif