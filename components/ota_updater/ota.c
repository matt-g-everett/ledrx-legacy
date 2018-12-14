#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_partition.h"
#include "esp_flash_partitions.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "string.h"

#include "ota.h"
#include "semver.h"
#include "wifi.h"

#define HASH_LEN 32 /* SHA-256 digest length */
#define BUFFSIZE 1024

const static char *TAG = "ledrx_ota";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };

const char *ca_cert;
const char *ota_version_url;
const char *ota_bin_url;
semver_t current_version;
uint8_t new_version_available = 0;

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
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                char version[16];
                strncpy(version, (const char *)evt->data, (size_t)evt->data_len);
                version[evt->data_len] = '\0';
                ESP_LOGI(TAG, "OTA Version: %s", version);
            
                semver_t next;
                semver_parse(version, &next);
                ESP_LOGI(TAG, "Parsed OTA version - Major: %d, Minor: %d, Patch: %d, pr: %s\n", next.major, next.minor, next.patch, next.prerelease);
                
                int result = semver_gt(next, current_version);
                if (result) {
                    ESP_LOGI(TAG, "New version available.");
                    new_version_available = 1;
                }
                
                semver_free(&next);
            }

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

uint8_t ota_download() {
    // Prevent triggering another download
    new_version_available = 0;

    ESP_LOGI(TAG, "Starting OTA download...");

    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);
    
    esp_http_client_config_t config = {
        .url = ota_bin_url,
        .cert_pem = ca_cert,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        return 0;
    }
    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return 0;
    }
    esp_http_client_fetch_headers(client);

    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        http_cleanup(client);
        return 0;
    }
    ESP_LOGI(TAG, "esp_ota_begin succeeded");

    int binary_file_length = 0;
    /*deal with all receive packet*/
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            http_cleanup(client);
            return 0;
        } else if (data_read > 0) {
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                http_cleanup(client);
                return 0;
            }
            binary_file_length += data_read;
            ESP_LOGD(TAG, "Written image length %d", binary_file_length);
        } else if (data_read == 0) {
            ESP_LOGI(TAG, "Connection closed,all data received");
            break;
        }
    }
    ESP_LOGI(TAG, "Total Write binary data length : %d", binary_file_length);

    if (esp_ota_end(update_handle) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed!");
        http_cleanup(client);
        return 0;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);
        return 0;
    }

    ESP_LOGI(TAG, "Prepare to restart system!");
    esp_restart();
    
    return 1;
}

void ota_task(void *pParam) {
    uint8_t wifi_connected = wifi_wait(portMAX_DELAY);
    ESP_LOGI(TAG, "WiFi state after initial wait is %s.", wifi_connected ? "connected" : "not connected");
    
    while (1) {
        // Check that wifi is still connected
        wifi_connected = wifi_wait(0);
        if (wifi_connected) {
            ESP_LOGI(TAG, "WiFi connected, checking for OTA update...");
            https_get(ota_version_url);
            if (new_version_available) {
                ota_download();
            }
        }
        else {
            ESP_LOGI(TAG, "WiFi not connected, skipping check for OTA update...");
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void ota_initialise(const char *cert, const char *version_url, const char *bin_url, const char *version) {
    ca_cert = cert;
    ota_version_url = version_url;
    ota_bin_url = bin_url;

    semver_parse(version, &current_version);
    ESP_LOGI(TAG, "Current version parsed - Major: %d, Minor: %d, Patch: %d, pr: %s\n", current_version.major, current_version.minor, current_version.patch, current_version.prerelease);
}
