#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "semver.h"

#define STACK_SIZE 2048

const static char *TAG = "ledrx_main";

void wifi_task(void *pParam) {
    while (1) {
        ESP_LOGI(TAG, "Doing wifi shit on core (%d)...", xPortGetCoreID());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void coap_task(void *pParam) {
    while (1) {
        ESP_LOGI(TAG, "Doing coap crap on core (%d)...", xPortGetCoreID());
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void led_task(void *pParam) {
    while (1) {
        ESP_LOGI(TAG, "Lit that shit up on core (%d)...", xPortGetCoreID());
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "App started");
    xTaskCreatePinnedToCore(wifi_task, "wifi", STACK_SIZE, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(coap_task, "coap", STACK_SIZE, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(led_task, "led", STACK_SIZE, NULL, 5, NULL, 1);
}
