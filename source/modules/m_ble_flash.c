#include <stdint.h>
#include <string.h>
#include "m_ble_flash.h"

#include "fds.h"
#include "nrf_log.h"
#include "nrf_pwr_mgmt.h"

#define DC_FLASH_CONFIG_VALID 0x42UL
#define DC_FILE_ID 0x2000
#define DC_REC_KEY 0x2001

/**@brief Data structure of configuration data stored to flash.
 */
typedef struct
{
    uint32_t         valid;
    ble_dcs_params_t config;
} dc_flash_config_data_t;

/**@brief Configuration data with size.
 */
typedef union
{
    dc_flash_config_data_t data;
    uint32_t               padding[CEIL_DIV(sizeof(dc_flash_config_data_t), 4)];
} dc_flash_config_t;

static fds_record_desc_t    m_record_desc;
static dc_flash_config_t    m_config;
static bool                 m_fds_write_success = false;

/* A dummy structure to save in flash. */
typedef struct
{
    uint32_t boot_count;
    char     device_name[16];
    bool     config1_on;
    bool     config2_on;
} configuration_t;

/* Array to map FDS return values to strings. */
char const * fds_err_str[] =
{
    "FDS_SUCCESS",
    "FDS_ERR_OPERATION_TIMEOUT",
    "FDS_ERR_NOT_INITIALIZED",
    "FDS_ERR_UNALIGNED_ADDR",
    "FDS_ERR_INVALID_ARG",
    "FDS_ERR_NULL_ARG",
    "FDS_ERR_NO_OPEN_RECORDS",
    "FDS_ERR_NO_SPACE_IN_FLASH",
    "FDS_ERR_NO_SPACE_IN_QUEUES",
    "FDS_ERR_RECORD_TOO_LARGE",
    "FDS_ERR_NOT_FOUND",
    "FDS_ERR_NO_PAGES",
    "FDS_ERR_USER_LIMIT_REACHED",
    "FDS_ERR_CRC_CHECK_FAILED",
    "FDS_ERR_BUSY",
    "FDS_ERR_INTERNAL",
};

/* Array to map FDS events to strings. */
static char const * fds_evt_str[] =
{
    "FDS_EVT_INIT",
    "FDS_EVT_WRITE",
    "FDS_EVT_UPDATE",
    "FDS_EVT_DEL_RECORD",
    "FDS_EVT_DEL_FILE",
    "FDS_EVT_GC",
};

// /* Dummy configuration data. */
// static configuration_t m_dummy_cfg =
// {
//     .config1_on  = false,
//     .config2_on  = true,
//     .boot_count  = 0x0,
//     .device_name = "dummy",
// };

// /* A record containing dummy configuration data. */
// static fds_record_t const m_dummy_record =
// {
//     .file_id           = CONFIG_FILE,
//     .key               = CONFIG_REC_KEY,
//     .data.p_data       = &m_dummy_cfg,
//     /* The length of a record is always expressed in 4-byte units (words). */
//     .data.length_words = (sizeof(m_dummy_cfg) + 3) / sizeof(uint32_t),
// };

/* Keep track of the progress of a delete_all operation. */
static struct
{
    bool delete_next;   //!< Delete next record.
    bool pending;       //!< Waiting for an fds FDS_EVT_DEL_RECORD event, to delete the next record.
} m_delete_all;

/* Flag to check fds initialization. */
static bool volatile m_fds_initialized;
static void fds_evt_handler(fds_evt_t const * p_evt)
{
    NRF_LOG_GREEN("Event: %s received (%s)",
                  fds_evt_str[p_evt->id],
                  fds_err_str[p_evt->result]);

    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_evt->result == FDS_SUCCESS)
            {
                m_fds_initialized = true;
            }
            break;

        case FDS_EVT_WRITE:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->write.record_id);
                NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->write.file_id);
                NRF_LOG_INFO("Record key:\t0x%04x", p_evt->write.record_key);
            }
        } break;

        case FDS_EVT_DEL_RECORD:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->del.record_id);
                NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->del.file_id);
                NRF_LOG_INFO("Record key:\t0x%04x", p_evt->del.record_key);
            }
            m_delete_all.pending = false;
        } break;

        default:
            break;
    }
}

/**@brief   Wait for fds to initialize. */
static void wait_for_fds_ready(void)
{
    while (!m_fds_initialized)
    {
       nrf_pwr_mgmt_run();
    }
}

uint32_t m_ble_flash_config_store(const ble_dcs_params_t * p_config)
{
    ret_code_t rc;
    //uint32_t            err_code;
    fds_record_t        record;
    //fds_record_chunk_t  record_chunk;

    NRF_LOG_INFO("Storing configuration \r\n");

    //NULL_PARAM_CHECK(p_config);

    memcpy(&m_config.data.config, p_config, sizeof(ble_dcs_params_t));
    m_config.data.valid = DC_FLASH_CONFIG_VALID;

    // Set up data.
    //record_chunk.p_data         = &m_config;
    //record_chunk.length_words   = sizeof(dc_flash_config_t)/4;

    record.data.p_data          = &m_config;
    record.data.length_words    = sizeof(dc_flash_config_t)/4;

    // Set up record.
    record.file_id              = DC_FILE_ID;
    record.key                  = DC_REC_KEY;
    //record.data.p_chunks        = &record_chunk;
    //record.data.num_chunks      = 1;

    rc = fds_record_update(&m_record_desc, &record);
    APP_ERROR_CHECK(rc);

    return NRF_SUCCESS;
}

uint32_t m_ble_flash_config_load(ble_dcs_params_t ** p_config)
{
    ret_code_t rc;
    fds_flash_record_t  flash_record;
    fds_find_token_t    ftok;

    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    NRF_LOG_INFO("Loading configuration\r\n");

    rc = fds_record_find(DC_FILE_ID, DC_REC_KEY, &m_record_desc, &ftok);
    if (rc == FDS_ERR_NOT_FOUND)
        return rc;
    //APP_ERROR_CHECK(rc);

    rc = fds_record_open(&m_record_desc, &flash_record);
    APP_ERROR_CHECK(rc);

    memcpy(&m_config, flash_record.p_data, sizeof(dc_flash_config_t));

    rc = fds_record_close(&m_record_desc);
    APP_ERROR_CHECK(rc);

    *p_config = &m_config.data.config;

    return NRF_SUCCESS;
}


uint32_t m_ble_flash_init(const ble_dcs_params_t    * p_default_config,
                          ble_dcs_params_t         ** p_config)
{
    ret_code_t rc;

    NRF_LOG_INFO("FDS example started.")

    /* Register first to receive an event when initialization is complete. */
    (void) fds_register(fds_evt_handler);

    NRF_LOG_INFO("Initializing fds...");

    rc = fds_init();
    APP_ERROR_CHECK(rc);

    /* Wait for fds to initialize. */
    wait_for_fds_ready();

    // NRF_LOG_INFO("Reading flash usage statistics...");

    rc = m_ble_flash_config_load(p_config);

    if (rc == FDS_ERR_NOT_FOUND)
    {
        fds_record_t        record;
        //fds_record_chunk_t  record_chunk;

        NRF_LOG_INFO("Writing default config\r\n");

        memcpy(&m_config.data.config, p_default_config, sizeof(ble_dcs_params_t));
        m_config.data.valid = DC_FLASH_CONFIG_VALID;

        // Set up data.
        // record_chunk.p_data         = &m_config;
        // record_chunk.length_words   = sizeof(dc_flash_config_t)/4;

        record.data.p_data          = &m_config;
        record.data.length_words    = sizeof(dc_flash_config_t)/4;

        // Set up record.
        record.file_id              = DC_FILE_ID;
        record.key                  = DC_REC_KEY;
        //record.data.p_chunks        = &record_chunk;
        //record.data.num_chunks      = 1;

        m_fds_write_success = false;
        rc = fds_record_write(&m_record_desc, &record);
        APP_ERROR_CHECK(rc);

        *p_config = &m_config.data.config;

        // while(m_fds_write_success != true)
        // {
        //     app_sched_execute();
        // }
    }
    else
    {
        APP_ERROR_CHECK(rc);
    }

    // fds_stat_t stat = {0};

    // rc = fds_stat(&stat);
    // APP_ERROR_CHECK(rc);

    // NRF_LOG_INFO("Found %d valid records.", stat.valid_records);
    // NRF_LOG_INFO("Found %d dirty records (ready to be garbage collected).", stat.dirty_records);

    // fds_record_desc_t desc = {0};
    // fds_find_token_t  tok  = {0};

    // rc = fds_record_find(CONFIG_FILE, CONFIG_REC_KEY, &desc, &tok);

    // if (rc == FDS_SUCCESS)
    // {
    //     /* A config file is in flash. Let's update it. */
    //     fds_flash_record_t config = {0};

    //     /* Open the record and read its contents. */
    //     rc = fds_record_open(&desc, &config);
    //     APP_ERROR_CHECK(rc);

    //     /* Copy the configuration from flash into m_dummy_cfg. */
    //     memcpy(&m_dummy_cfg, config.p_data, sizeof(configuration_t));

    //     NRF_LOG_INFO("Config file found, updating boot count to %d.", m_dummy_cfg.boot_count);

    //     /* Update boot count. */
    //     m_dummy_cfg.boot_count++;

    //     /* Close the record when done reading. */
    //     rc = fds_record_close(&desc);
    //     APP_ERROR_CHECK(rc);

    //     /* Write the updated record to flash. */
    //     rc = fds_record_update(&desc, &m_dummy_record);
    //     APP_ERROR_CHECK(rc);
    // }
    // else
    // {
    //     /* System config not found; write a new one. */
    //     NRF_LOG_INFO("Writing config file...");

    //     rc = fds_record_write(&desc, &m_dummy_record);
    //     APP_ERROR_CHECK(rc);
    // }

    return NRF_SUCCESS;
}