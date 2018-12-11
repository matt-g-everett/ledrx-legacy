#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_partition.h"
#include "esp_flash_partitions.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_ota_ops.h"

#include "ota.h"
#include "semver.h"
#include "wifi.h"

#define HASH_LEN 32 /* SHA-256 digest length */
#define BUFFSIZE 1024

const static char *TAG = "ledrx_ota";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };

const char *ca_cert;
const char *download_url;
semver_t current_version;

void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s: %s", label, hash_print);
}

void log_partition_info(void) {
    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;

    // get sha256 digest for the partition table
    partition.address   = ESP_PARTITION_TABLE_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type      = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for the partition table: ");

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");
}

static void __attribute__((noreturn)) task_fatal_error()
{
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            // ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // if (!esp_http_client_is_chunked_response(evt->client)) {
            //     printf("OTA version: %.*s\n", evt->data_len, (char*)evt->data);

            //     semver_t next;
            //     semver_parse((char*)evt->data, &next);
            //     ESP_LOGI(TAG, "Parsed OTA version - Major: %d, Minor: %d, Patch: %d, pr: %s\n", next.major, next.minor, next.patch, next.prerelease);
                
            //     int result = semver_gt(next, current_version);
            //     if (result) {
            //         ESP_LOGI(TAG, "PERFORM UPGRADE");
            //     }
            //     else {
            //         ESP_LOGI(TAG, "DON'T UPGRADE");
            //     }
                
            //     semver_free(&next);
            //     // printf("OTA version: %.*s\n", evt->data_len, (char*)evt->data);
            // }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

static void https_get(const char *url)
{
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
        .cert_pem = ca_cert,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

// void ota_task(void *pParam) {
// {
//     ESP_LOGI(TAG, "Starting OTA task...");

//     uint8_t wifi_connected;
//     esp_err_t err;
//     /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
//     esp_ota_handle_t update_handle = 0;
//     const esp_partition_t *update_partition = NULL;
//     const esp_partition_t *configured = esp_ota_get_boot_partition();
//     const esp_partition_t *running = esp_ota_get_running_partition();

//     if (configured != running) {
//         ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
//                  configured->address, running->address);
//         ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
//     }
//     ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
//              running->type, running->subtype, running->address);

//     // Wait for initial WiFi connection
//     wifi_connected = wifi_wait(portMAX_DELAY);
//     ESP_LOGI(TAG, "WiFi state after initial wait is %s.", wifi_connected ? "connected" : "not connected");

//     while (1) {
//         wifi_connected = wifi_wait(0);
//     }
    
//     esp_http_client_config_t config = {
//         .url = download_url,
//         .cert_pem = ca_cert,
//     };
//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     if (client == NULL) {
//         ESP_LOGE(TAG, "Failed to initialise HTTP connection");
//         task_fatal_error();
//     }
//     err = esp_http_client_open(client, 0);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
//         esp_http_client_cleanup(client);
//         task_fatal_error();
//     }
//     esp_http_client_fetch_headers(client);

//     update_partition = esp_ota_get_next_update_partition(NULL);
//     ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
//              update_partition->subtype, update_partition->address);
//     assert(update_partition != NULL);

//     err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
//         http_cleanup(client);
//         task_fatal_error();
//     }
//     ESP_LOGI(TAG, "esp_ota_begin succeeded");

//     int binary_file_length = 0;
//     /*deal with all receive packet*/
//     while (1) {
//         int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
//         if (data_read < 0) {
//             ESP_LOGE(TAG, "Error: SSL data read error");
//             http_cleanup(client);
//             task_fatal_error();
//         } else if (data_read > 0) {
//             err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
//             if (err != ESP_OK) {
//                 http_cleanup(client);
//                 task_fatal_error();
//             }
//             binary_file_length += data_read;
//             ESP_LOGD(TAG, "Written image length %d", binary_file_length);
//         } else if (data_read == 0) {
//             ESP_LOGI(TAG, "Connection closed,all data received");
//             break;
//         }
//     }
//     ESP_LOGI(TAG, "Total Write binary data length : %d", binary_file_length);

//     if (esp_ota_end(update_handle) != ESP_OK) {
//         ESP_LOGE(TAG, "esp_ota_end failed!");
//         http_cleanup(client);
//         task_fatal_error();
//     }

//     if (esp_partition_check_identity(esp_ota_get_running_partition(), update_partition) == true) {
//         ESP_LOGI(TAG, "The current running firmware is same as the firmware just downloaded");
//         int i = 0;
//         ESP_LOGI(TAG, "When a new firmware is available on the server, press the reset button to download it");
//         while(1) {
//             ESP_LOGI(TAG, "Waiting for a new firmware ... %d", ++i);
//             vTaskDelay(2000 / portTICK_PERIOD_MS);
//         }
//     }

//     err = esp_ota_set_boot_partition(update_partition);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
//         http_cleanup(client);
//         task_fatal_error();
//     }
//     ESP_LOGI(TAG, "Prepare to restart system!");
//     esp_restart();
//     return;
// }

void ota_task(void *pParam) {
    uint8_t wifi_connected = wifi_wait(portMAX_DELAY);
    ESP_LOGI(TAG, "WiFi state after initial wait is %s.", wifi_connected ? "connected" : "not connected");
    
    while (1) {
        https_get(download_url);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void ota_initialise(const char *cert, const char *url, const char *version) {
    ca_cert = cert;
    download_url = url;

    semver_parse(version, &current_version);
    ESP_LOGI(TAG, "Current version parsed - Major: %d, Minor: %d, Patch: %d, pr: %s\n", current_version.major, current_version.minor, current_version.patch, current_version.prerelease);
}
