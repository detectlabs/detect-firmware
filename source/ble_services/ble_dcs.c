#include "sdk_macros.h"
#include "ble_dcs.h"
#include "nrf_log.h"

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the S140 SoftDevice.
 *
 * @param[in] p_tcs     Detect Configuration Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_dcs_t * p_tcs, ble_evt_t const * p_ble_evt)
{
    p_tcs->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S140 SoftDevice.
 *
 * @param[in] p_tcs     Detect Configuration Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_dcs_t * p_tcs, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_tcs->conn_handle = BLE_CONN_HANDLE_INVALID;
}


static void on_authorize_req(ble_dcs_t * p_dcs, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_rw_authorize_request_t const * p_evt_rw_authorize_request = &p_ble_evt->evt.gatts_evt.params.authorize_request;
    uint32_t err_code;

    if (p_evt_rw_authorize_request->type  == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
    {
        ble_gatts_rw_authorize_reply_params_t rw_authorize_reply;
        bool                                  valid_data = true;
        bool                                  reply = true;
        ble_dcs_evt_type_t                    evt_type = BLE_DCS_EVT_DEV_NAME;


        if (p_evt_rw_authorize_request->request.write.handle == p_dcs->dev_name_handles.value_handle)
        {
            evt_type = BLE_DCS_EVT_DEV_NAME;

            // Check for valid data
            if(p_evt_rw_authorize_request->request.write.len > BLE_TCS_DEVICE_NAME_LEN_MAX)
            {
                valid_data = false;
            }
        }
        else if (p_evt_rw_authorize_request->request.write.handle == p_dcs->adv_param_handles.value_handle)
        {
            // Check for valid data
            if(p_evt_rw_authorize_request->request.write.len != sizeof(ble_dcs_adv_params_t))
            {
                valid_data = false;
            }
            else
            {
                ble_dcs_adv_params_t * p_data = (ble_dcs_adv_params_t *)p_evt_rw_authorize_request->request.write.data;

                evt_type = BLE_DCS_EVT_ADV_PARAM;

                if ( (p_data->interval  < DCS_ADV_PARAMS_INTERVAL_MIN)    ||
                     (p_data->interval  > DCS_ADV_PARAMS_INTERVAL_MAX)    ||
                  /* (p_data->timeout  < TCS_ADV_PARAMS_TIMEOUT_MIN)      || */
                     (p_data->timeout  > DCS_ADV_PARAMS_TIMEOUT_MAX))
                {
                    valid_data = false;
                }
            }
        }
        else if (p_evt_rw_authorize_request->request.write.handle == p_dcs->conn_param_handles.value_handle)
        {
            // Check for valid data
            if(p_evt_rw_authorize_request->request.write.len != sizeof(ble_dcs_conn_params_t))
            {
                valid_data = false;
            }
            else
            {
                ble_dcs_conn_params_t * p_data = (ble_dcs_conn_params_t *)p_evt_rw_authorize_request->request.write.data;

                evt_type = BLE_DCS_EVT_CONN_PARAM;

                if ( (p_data->min_conn_int  < BLE_GAP_CP_MIN_CONN_INTVL_MIN)    ||
                     (p_data->min_conn_int  > BLE_GAP_CP_MIN_CONN_INTVL_MAX)    ||
                     (p_data->max_conn_int  < BLE_GAP_CP_MAX_CONN_INTVL_MIN)    ||
                     (p_data->max_conn_int  > BLE_GAP_CP_MAX_CONN_INTVL_MAX)    ||
                     (p_data->slave_latency > BLE_GAP_CP_SLAVE_LATENCY_MAX)     ||
                     (p_data->sup_timeout   < BLE_GAP_CP_CONN_SUP_TIMEOUT_MIN)  ||
                     (p_data->sup_timeout   > BLE_GAP_CP_CONN_SUP_TIMEOUT_MAX))
                {
                    valid_data = false;
                }

              /* If both conn_sup_timeout and max_conn_interval are specified, then the following constraint applies:
                 conn_sup_timeout * 4 > (1 + slave_latency) * max_conn_interval
                 that corresponds to the following Bluetooth Spec requirement:
                 The Supervision_Timeout in milliseconds shall be larger than
                 (1 + Conn_Latency) * Conn_Interval_Max * 2, where Conn_Interval_Max is given in milliseconds.   */

                if ( (p_data->sup_timeout * 4) <= ((1 + p_data->slave_latency) * p_data->max_conn_int) )
                {
                    valid_data = false;
                }
            }
        }
        else
        {
            valid_data = false;
            reply = false;
        }

        if (reply)
        {
            // Reply depending on valid data or not
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
        }

        // Call event handler
        if ( valid_data && (p_dcs->evt_handler != NULL))
        {
            p_dcs->evt_handler(p_dcs,
                               evt_type,
                               p_evt_rw_authorize_request->request.write.data,
                               p_evt_rw_authorize_request->request.write.len);
        }
    }
}


void ble_dcs_on_ble_evt(ble_dcs_t * p_dcs, ble_evt_t const * p_ble_evt)
{
    if ((p_dcs == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_dcs, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_dcs, p_ble_evt);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            on_authorize_req(p_dcs, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for adding device name characteristic.
 *
 * @param[in] p_tcs       Detect Configuration Service structure.
 * @param[in] p_tcs_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t device_name_char_add(ble_dcs_t * p_dcs, const ble_dcs_init_t * p_dcs_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.char_props.read          = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_dcs->uuid_type;
    ble_uuid.uuid = BLE_UUID_DCS_DEVICE_NAME_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 1;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = p_dcs_init->p_init_vals->dev_name.len;
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = p_dcs_init->p_init_vals->dev_name.name;
    attr_char_value.max_len   = BLE_TCS_DEVICE_NAME_LEN_MAX;

    return sd_ble_gatts_characteristic_add(p_dcs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_dcs->dev_name_handles);
}


/**@brief Function for adding advertising parameters characteristic.
 *
 * @param[in] p_tcs       Detect Configuration Service structure.
 * @param[in] p_tcs_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t adv_params_char_add(ble_dcs_t * p_tcs, const ble_dcs_init_t * p_dcs_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.char_props.read          = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_tcs->uuid_type;
    ble_uuid.uuid = BLE_UUID_DCS_ADV_PARAMS_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 1;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(ble_dcs_adv_params_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = (uint8_t *)&p_dcs_init->p_init_vals->adv_params;
    attr_char_value.max_len   = sizeof(ble_dcs_adv_params_t);

    return sd_ble_gatts_characteristic_add(p_tcs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_tcs->adv_param_handles);
}


/**@brief Function for adding connection parameters characteristic.
 *
 * @param[in] p_tcs       Detect Configuration Service structure.
 * @param[in] p_tcs_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t conn_params_char_add(ble_dcs_t * p_dcs, const ble_dcs_init_t * p_dcs_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.char_props.read          = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_dcs->uuid_type;
    ble_uuid.uuid = BLE_UUID_DCS_CONN_PARAM_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 1;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(ble_dcs_conn_params_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = (uint8_t *)&p_dcs_init->p_init_vals->conn_params;
    attr_char_value.max_len   = sizeof(ble_dcs_conn_params_t);

    return sd_ble_gatts_characteristic_add(p_dcs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_dcs->conn_param_handles);
}


/**@brief Function for adding FW version characteristic.
 *
 * @param[in] p_tcs       Detect Configuration Service structure.
 * @param[in] p_tcs_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t fw_version_char_add(ble_dcs_t * p_dcs, const ble_dcs_init_t * p_dcs_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_dcs->uuid_type;
    ble_uuid.uuid = BLE_UUID_DCS_FW_VERSION_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(ble_dcs_fw_version_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = (uint8_t *)&p_dcs_init->p_init_vals->fw_version;
    attr_char_value.max_len   = sizeof(ble_dcs_fw_version_t);

    return sd_ble_gatts_characteristic_add(p_dcs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_dcs->fwv_handles);
}



uint32_t ble_dcs_init(ble_dcs_t * p_dcs, const ble_dcs_init_t * p_dcs_init)
{
    uint32_t        err_code;
    ble_uuid_t      service_uuid;
    ble_uuid128_t   dcs_base_uuid = DCS_BASE_UUID;

    NRF_LOG_INFO("Start of DCS Service Init");

    VERIFY_PARAM_NOT_NULL(p_dcs);
    VERIFY_PARAM_NOT_NULL(p_dcs_init);

    NRF_LOG_INFO("Passed null check");

    // Initialize the service structure.
    p_dcs->conn_handle                  = BLE_CONN_HANDLE_INVALID;
    p_dcs->evt_handler                  = p_dcs_init->evt_handler;

    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&dcs_base_uuid, &p_dcs->uuid_type);
    NRF_LOG_INFO("sd_uuid_vs_add error:  %d.",err_code);

    VERIFY_SUCCESS(err_code);
    NRF_LOG_INFO("Added dcs base UUID");

    service_uuid.type = p_dcs->uuid_type;
    service_uuid.uuid = BLE_UUID_DCS_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &p_dcs->service_handle);
    VERIFY_SUCCESS(err_code);
    NRF_LOG_INFO("Added dcs service");

    // Add the device name Characteristic.
    err_code = device_name_char_add(p_dcs, p_dcs_init);
    VERIFY_SUCCESS(err_code);
    NRF_LOG_INFO("device_name_char_add:  %d.",err_code);

    // Add the advertising parameters Characteristic.
    err_code = adv_params_char_add(p_dcs, p_dcs_init);
    VERIFY_SUCCESS(err_code);

    // Add the connection parameters Characteristic.
    err_code = conn_params_char_add(p_dcs, p_dcs_init);
    VERIFY_SUCCESS(err_code);

    // Add the FW version Characteristic.
    err_code = fw_version_char_add(p_dcs, p_dcs_init);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}
