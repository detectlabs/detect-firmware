#include <stdint.h>
#include <string.h>
#include "m_detection_flash.h"
#include "app_scheduler.h"

#include "fds.h"
#include "nrf_log.h"

#define DS_FLASH_CONFIG_VALID   0x42UL
#define DET_FILE_ID             0x1001
#define DET_REC_KEY             0x1002

/**@brief Data structure of configuration data stored to flash.
 */
typedef struct
{
    uint32_t         valid;
    ble_dds_config_t config;
} m_det_flash_config_data_t;

/**@brief Configuration data with size.
 */
typedef union
{
    m_det_flash_config_data_t data;
    uint32_t               padding[CEIL_DIV(sizeof(m_det_flash_config_data_t), 4)];
} m_det_flash_config_t;

static fds_record_desc_t        m_record_config_desc;
static m_det_flash_config_t     m_config;
static bool                     m_fds_config_write_success = false;
static bool                     m_fds_config_initialized = false;

/**@brief Function for handling flash data storage events.
 */
static void det_fds_evt_handler( fds_evt_t const * const p_fds_evt )
{
    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                m_fds_config_initialized = true;
            }
            else
            {
                // Initialization failed.
                NRF_LOG_ERROR("FDS init failed!\r\n");
                APP_ERROR_CHECK_BOOL(false);
            }
            break;
        case FDS_EVT_WRITE:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                if (p_fds_evt->write.file_id == DET_FILE_ID)
                {
                    NRF_LOG_INFO("FDS config write success! %d FileId: 0x%x RecKey:0x%x\r\n",   p_fds_evt->write.is_record_updated,
                                                                                                p_fds_evt->write.file_id,
                                                                                                p_fds_evt->write.record_key);
                    m_fds_config_write_success = true;
                }
            }
            else
            {
                // Initialization failed.
                NRF_LOG_ERROR("FDS write failed!\r\n");
                APP_ERROR_CHECK_BOOL(false);
            }
            break;
        default:
            NRF_LOG_INFO("FDS handler - %d - %d\r\n", p_fds_evt->id, p_fds_evt->result);
            APP_ERROR_CHECK(p_fds_evt->result);
            break;
    }
}

uint32_t m_det_flash_config_store(const ble_dds_config_t * p_config)
{
    fds_record_t        record;
    ret_code_t rc;

    NRF_LOG_INFO("Storing configuration\r\n");

    VERIFY_PARAM_NOT_NULL(p_config);

    memcpy(&m_config.data.config, p_config, sizeof(ble_dds_config_t));
    m_config.data.valid = DS_FLASH_CONFIG_VALID;

    // Set up data.
    record.data.p_data         = &m_config;
    record.data.length_words   = sizeof(m_det_flash_config_t)/4;

    // Set up record.
    record.file_id              = DET_FILE_ID;
    record.key                  = DET_REC_KEY;

    rc = fds_record_update(&m_record_config_desc, &record);
    APP_ERROR_CHECK(rc);

    return NRF_SUCCESS;
}

uint32_t m_det_flash_config_load(ble_dds_config_t ** p_config)
{
    ret_code_t rc;
    fds_flash_record_t  flash_record;
    fds_find_token_t    ftok;

    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    NRF_LOG_INFO("Loading Detection configuration\r\n");

    rc = fds_record_find(DET_FILE_ID, DET_REC_KEY, &m_record_config_desc, &ftok);
    if (rc == FDS_ERR_NOT_FOUND)
    {
        NRF_LOG_INFO("Loading Detection configuration Found in Flash\r\n");
        return rc;
    }

    rc = fds_record_open(&m_record_config_desc, &flash_record);
    APP_ERROR_CHECK(rc);

    memcpy(&m_config, flash_record.p_data, sizeof(m_det_flash_config_t));

    rc = fds_record_close(&m_record_config_desc);
    APP_ERROR_CHECK(rc);

    *p_config = &m_config.data.config;

    return NRF_SUCCESS;
}

uint32_t m_det_flash_init(const ble_dds_config_t * p_default_config,
                          ble_dds_config_t      ** p_config)
{
    ret_code_t rc;

    // NRF_LOG_INFO("\rDetection Default Config\r\n");
    // NRF_LOG_RAW_INFO("presence_intervale_ms: %d  \n", p_default_config->presence_interval_ms);
    // NRF_LOG_RAW_INFO("range_intervale_ms: %d  \n", p_default_config->range_interval_ms);
    // NRF_LOG_RAW_INFO("threshold_config.eth13h: %d  \n", p_default_config->threshold_config.eth13h);
    // NRF_LOG_RAW_INFO("threshold_config.eth13l: %d  \n", p_default_config->threshold_config.eth13l);
    // NRF_LOG_RAW_INFO("threshold_config.eth24h: %d  \n", p_default_config->threshold_config.eth24h);
    // NRF_LOG_RAW_INFO("threshold_config.eth24l: %d  \n", p_default_config->threshold_config.eth24l);
    // NRF_LOG_RAW_INFO("sample_mode: %d  \n", p_default_config->sample_mode);


    // NRF_LOG_INFO("Detection Flash Initialization\r\n");
    
    VERIFY_PARAM_NOT_NULL(p_default_config);

    /* Register first to receive an event when initialization is complete. */
    (void) fds_register(det_fds_evt_handler);
    
    rc = fds_init();
    APP_ERROR_CHECK(rc);

    while (m_fds_config_initialized == false)
    {
        app_sched_execute();
    }

    rc = m_det_flash_config_load(p_config);
    
    if (rc == FDS_ERR_NOT_FOUND)
    {
        NRF_LOG_INFO("Writing default detection config\r\n");

        fds_record_t        record;

        memcpy(&m_config.data.config, p_default_config, sizeof(ble_dds_config_t));
        m_config.data.valid = DS_FLASH_CONFIG_VALID;

        // Set up data.
        record.data.p_data         = &m_config;
        record.data.length_words   = sizeof(m_det_flash_config_t)/4;

        // Set up record.
        record.file_id              = DET_FILE_ID;
        record.key                  = DET_REC_KEY;

        m_fds_config_write_success = false;
        rc = fds_record_write(&m_record_config_desc, &record);
        APP_ERROR_CHECK(rc);

        *p_config = &m_config.data.config;

        while(m_fds_config_write_success != true)
        {
            app_sched_execute();
        }
    }
    else
    {
        // NRF_LOG_INFO("\rDetection Loaded Config\r\n");
        // NRF_LOG_RAW_INFO("presence_intervale_ms: %d  \n", (*p_config)->presence_interval_ms);
        // NRF_LOG_RAW_INFO("range_intervale_ms: %d  \n", (*p_config)->range_interval_ms);
        // NRF_LOG_RAW_INFO("threshold_config.eth13h: %d  \n", (*p_config)->threshold_config.eth13h);
        // NRF_LOG_RAW_INFO("threshold_config.eth13l: %d  \n", (*p_config)->threshold_config.eth13l);
        // NRF_LOG_RAW_INFO("threshold_config.eth24h: %d  \n", (*p_config)->threshold_config.eth24h);
        // NRF_LOG_RAW_INFO("threshold_config.eth24l: %d  \n", (*p_config)->threshold_config.eth24l);
        // NRF_LOG_RAW_INFO("sample_mode: %d  \n", (*p_config)->sample_mode);

        APP_ERROR_CHECK(rc);
    }

    return NRF_SUCCESS;
}