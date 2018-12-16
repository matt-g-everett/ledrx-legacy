#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "semver.h"
#include "wifi.h"
#include "ota.h"
#include "coapserver.h"

#define OTA_STACK_SIZE 8192
#define COAP_STACK_SIZE 2048
#define STACK_SIZE 2048

#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASS CONFIG_WIFI_PASSWORD
#define OTA_BIN_URL CONFIG_FIRMWARE_UPG_URL
#define OTA_VERSION_URL CONFIG_FIRMWARE_VERSION_URL

const static char *TAG = "ledrx_main";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");
extern const uint8_t version_start[] asm("_binary_version_txt_start");
extern const uint8_t version_end[] asm("_binary_version_txt_end");

// void ota_task(void *pParam) {
//     while (1) {
//         ESP_LOGI(TAG, "OTAing it on core (%d)...", xPortGetCoreID());
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

// void coap_task(void *pParam) {
//     while (1) {
//         ESP_LOGI(TAG, "Doing coap crap on core (%d)...", xPortGetCoreID());
//         vTaskDelay(2000 / portTICK_PERIOD_MS);
//     }
// }

void led_task(void *pParam) {
    while (1) {
        ESP_LOGI(TAG, "Lit that shit up on core (%d)...", xPortGetCoreID());
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "App version : %s", (const char *)version_start);

    wifi_initialise(WIFI_SSID, WIFI_PASS);
    ota_initialise((const char *)server_cert_pem_start, OTA_VERSION_URL, OTA_BIN_URL, (const char *)version_start);
    xTaskCreatePinnedToCore(ota_task, "ota", OTA_STACK_SIZE, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(coap_task, "coap", COAP_STACK_SIZE, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(led_task, "led", STACK_SIZE, NULL, 5, NULL, 1);
}
