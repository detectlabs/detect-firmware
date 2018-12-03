#include "ble_dds.h"
#include "ble_srv_common.h"
#include "sdk_common.h"
#include "nrf_log.h"

#define BLE_UUID_DDS_PRESENCE_CHAR      0x0201                      /**< The UUID of the temperature Characteristic. */
#define BLE_UUID_DDS_RANGE_CHAR         0x0202                      /**< The UUID of the pressure Characteristic. */
#define BLE_UUID_DDS_CONFIG_CHAR        0x0203                      /**< The UUID of the config Characteristic. */


#define BLE_DDS_MAX_RX_CHAR_LEN        BLE_DDS_MAX_DATA_LEN        /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_DDS_MAX_TX_CHAR_LEN        BLE_DDS_MAX_DATA_LEN        /**< Maximum length of the TX Characteristic (in bytes). */

// EE84xxxx-43B7-4F65-9FB9-D7B92D683E36
#define DDS_BASE_UUID                  {{0x36, 0x3E, 0x68, 0x2D, 0xB9, 0xD7, 0xB9, 0x9F, 0x65, 0x4F, 0xB7, 0x43, 0x00, 0x00, 0x84, 0xEE}}

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the S132 SoftDevice.
 *
 * @param[in] p_tes     Thingy Environment Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_dds_t * p_dds, ble_evt_t const * p_ble_evt)
{
    p_dds->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S132 SoftDevice.
 *
 * @param[in] p_tes     Thingy Environment Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_dds_t * p_dds, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_dds->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S132 SoftDevice.
 *
 * @param[in] p_tes     Thingy Environment Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_dds_t * p_dds, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if ( (p_evt_write->handle == p_dds->presence_handles.cccd_handle) &&
         (p_evt_write->len == 2) )
    {
        bool notif_enabled;

        notif_enabled = ble_srv_is_notification_enabled(p_evt_write->data);

        if (p_dds->is_presence_notif_enabled != notif_enabled)
        {

            //NRF_LOG_INFO("******** NOTIF PRESENCE **********\r\n");
            p_dds->is_presence_notif_enabled = notif_enabled;

            if (p_dds->evt_handler != NULL)
            {
                p_dds->evt_handler(p_dds, BLE_DDS_EVT_NOTIF_PRESENCE, p_evt_write->data, p_evt_write->len);
            }
        }
    }
    else if ( (p_evt_write->handle == p_dds->range_handles.cccd_handle) &&
         (p_evt_write->len == 2) )
    {
        bool notif_enabled;

        notif_enabled = ble_srv_is_notification_enabled(p_evt_write->data);

        if (p_dds->is_range_notif_enabled != notif_enabled)
        {
            //NRF_LOG_INFO("******** NOTIF RANGE **********\r\n");

            p_dds->is_range_notif_enabled = notif_enabled;

            if (p_dds->evt_handler != NULL)
            {
                p_dds->evt_handler(p_dds, BLE_DDS_EVT_NOTIF_RANGE, p_evt_write->data, p_evt_write->len);
            }
        }
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}

static void on_authorize_req(ble_dds_t * p_dds, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_rw_authorize_request_t const * p_evt_rw_authorize_request = &p_ble_evt->evt.gatts_evt.params.authorize_request;
    uint32_t err_code;

    if (p_evt_rw_authorize_request->type  == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
    {
        if (p_evt_rw_authorize_request->request.write.handle == p_dds->config_handles.value_handle)
        {
            ble_gatts_rw_authorize_reply_params_t rw_authorize_reply;
            bool                                  valid_data = true;

            // Check for valid data
            if(p_evt_rw_authorize_request->request.write.len != sizeof(ble_dds_config_t))
            {
                valid_data = false;
            }
            else
            {
                ble_dds_config_t * p_config = (ble_dds_config_t *)p_evt_rw_authorize_request->request.write.data;

                if ( (p_config->presence_interval_ms < BLE_DDS_CONFIG_PRESENCE_INT_MIN)           ||
                    (p_config->presence_interval_ms > BLE_DDS_CONFIG_PRESENCE_INT_MAX)            ||
                    (p_config->range_interval_ms < BLE_DDS_CONFIG_RANGE_INT_MIN)                  ||
                    (p_config->range_interval_ms > BLE_DDS_CONFIG_RANGE_INT_MAX)                  ||
                    (p_config->threshold_config.eth13h < BLE_DDS_CONFIG_THRESHOLD_MIN)            ||
                    (p_config->threshold_config.eth13l > BLE_DDS_CONFIG_THRESHOLD_MAX)            ||
                    (p_config->threshold_config.eth24h < BLE_DDS_CONFIG_THRESHOLD_MIN)            ||
                    ((int)p_config->threshold_config.eth24l > (int)BLE_DDS_CONFIG_THRESHOLD_MAX))
                {
                    valid_data = false;
                }
            }

            rw_authorize_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;

            if (valid_data)
            {
                rw_authorize_reply.params.write.update      = 1;
                rw_authorize_reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;
                rw_authorize_reply.params.write.p_data      = p_evt_rw_authorize_request->request.write.data;
                rw_authorize_reply.params.write.len         = p_evt_rw_authorize_request->request.write.len;
                rw_authorize_reply.params.write.offset      = p_evt_rw_authorize_request->request.write.offset;
            }
            else
            {
                rw_authorize_reply.params.write.update      = 0;
                rw_authorize_reply.params.write.gatt_status = BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED;
            }

            err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                       &rw_authorize_reply);
            APP_ERROR_CHECK(err_code);

            if ( valid_data && (p_dds->evt_handler != NULL))
            {
                p_dds->evt_handler(p_dds,
                                   BLE_DDS_EVT_CONFIG_RECEIVED,
                                   p_evt_rw_authorize_request->request.write.data,
                                   p_evt_rw_authorize_request->request.write.len);
            }
        }
    }
}

void ble_dds_on_ble_evt(ble_dds_t * p_dds, ble_evt_t const * p_ble_evt)
{
    if ((p_dds == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_dds, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_dds, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_dds, p_ble_evt);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            on_authorize_req(p_dds, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_dds_presence_set(ble_dds_t * p_tes, ble_dds_presence_t * p_data)
{
    ble_gatts_hvx_params_t hvx_params;
    uint16_t               length = sizeof(ble_dds_presence_t);

    VERIFY_PARAM_NOT_NULL(p_tes);

    if ((p_tes->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_tes->is_presence_notif_enabled))
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > BLE_DDS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_tes->presence_handles.value_handle;
    hvx_params.p_data = (uint8_t *)p_data;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_tes->conn_handle, &hvx_params);
}

uint32_t ble_dds_range_set(ble_dds_t * p_tes, ble_dds_range_t * p_data)
{
    ble_gatts_hvx_params_t hvx_params;
    uint16_t               length = sizeof(ble_dds_range_t);

    VERIFY_PARAM_NOT_NULL(p_tes);

    if ((p_tes->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_tes->is_range_notif_enabled))
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > BLE_DDS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_tes->range_handles.value_handle;
    hvx_params.p_data = (uint8_t *)p_data;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_tes->conn_handle, &hvx_params);
}

/**@brief Function for adding pressure characteristic.
 *
 * @param[in] p_tes       Thingy Environment Service structure.
 * @param[in] p_tes_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t presence_char_add(ble_dds_t * p_dds, const ble_dds_init_t * p_dds_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_dds->uuid_type;
    ble_uuid.uuid = BLE_UUID_DDS_PRESENCE_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(ble_dds_presence_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = (uint8_t *)p_dds_init->p_init_presence;
    attr_char_value.max_len   = sizeof(ble_dds_presence_t);

    return sd_ble_gatts_characteristic_add(p_dds->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_dds->presence_handles);
}

/**@brief Function for adding pressure characteristic.
 *
 * @param[in] p_tes       Thingy Environment Service structure.
 * @param[in] p_tes_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t range_char_add(ble_dds_t * p_dds, const ble_dds_init_t * p_dds_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_dds->uuid_type;
    ble_uuid.uuid = BLE_UUID_DDS_RANGE_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(ble_dds_range_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = (uint8_t *)p_dds_init->p_init_range;
    attr_char_value.max_len   = sizeof(ble_dds_range_t);

    return sd_ble_gatts_characteristic_add(p_dds->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_dds->range_handles);
}

/**@brief Function for adding configuration characteristic.
 *
 * @param[in] p_tes       Thingy Environment Service structure.
 * @param[in] p_tes_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t config_char_add(ble_dds_t * p_tes, const ble_dds_init_t * p_dds_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_tes->uuid_type;
    ble_uuid.uuid = BLE_UUID_DDS_CONFIG_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 1;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(ble_dds_config_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = (uint8_t *)p_dds_init->p_init_config;
    attr_char_value.max_len   = sizeof(ble_dds_config_t);

    return sd_ble_gatts_characteristic_add(p_tes->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_tes->config_handles);
}

uint32_t ble_dds_init(ble_dds_t * p_dds, const ble_dds_init_t * p_dds_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t dds_base_uuid = DDS_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_dds);
    VERIFY_PARAM_NOT_NULL(p_dds_init);

    // Initialize the service structure.
    p_dds->conn_handle                  = BLE_CONN_HANDLE_INVALID;
    p_dds->evt_handler                  = p_dds_init->evt_handler;
    p_dds->is_presence_notif_enabled = false;
    p_dds->is_range_notif_enabled    = false;

    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&dds_base_uuid, &p_dds->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_dds->uuid_type;
    ble_uuid.uuid = BLE_UUID_DDS_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_dds->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add the temperature Characteristic.
    err_code = presence_char_add(p_dds, p_dds_init);
    VERIFY_SUCCESS(err_code);

    // Add the pressure Characteristic.
    err_code = range_char_add(p_dds, p_dds_init);
    VERIFY_SUCCESS(err_code);

    // Add the config Characteristic.
    err_code = config_char_add(p_dds, p_dds_init);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}
