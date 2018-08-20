//Detect Configuration Service
#include "sdk_macros.h"
#include "ble_dcs.h"
#include "nrf_log.h"

/**@brief Function for adding device name characteristic.
 *
 * @param[in] p_tcs       Thingy Configuration Service structure.
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
    NRF_LOG_INFO("name char");

    // // Add the advertising parameters Characteristic.
    // err_code = adv_params_char_add(p_dcs, p_dcs_init);
    // VERIFY_SUCCESS(err_code);

    // // Add the connection parameters Characteristic.
    // err_code = conn_params_char_add(p_dcs, p_dcs_init);
    // VERIFY_SUCCESS(err_code);

    // // Add the FW version Characteristic.
    // err_code = fw_version_char_add(p_dcs, p_dcs_init);
    // VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}
