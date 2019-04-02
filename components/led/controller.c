#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/rmt.h"

#include "renderer.h"
#include "ledscan.h"
#include "ws2811.h"
#include "controller.h"

#define FRAME_BUFFER_SIZE CONFIG_FRAME_BUFFER_SIZE

const static char *TAG = "controller";

FRAME_t frame_buffer[FRAME_BUFFER_SIZE];

uint8_t running = 0;

void controller_initialise() {
    running = 1;
    renderer_initialise();
    renderer_set_strategy(ledscan_calculate_frame, ledscan_create_state(220));
}

void controller_configure(Ledapi__Config *config) {
    switch (config->mode) {
        case LEDAPI__CONFIG__MODE__OFF:
            ESP_LOGI(TAG, "MODE=Off");
            controller_stop();
            break;
        
        case LEDAPI__CONFIG__MODE__FIXED_FRAME:
            ESP_LOGI(TAG, "MODE=Fixed Frame");
            ESP_LOGI(TAG, "Frame LED Count %d.", config->frame->data.len / 3);
            break;
        
        case LEDAPI__CONFIG__MODE__SCAN:
            ESP_LOGI(TAG, "MODE=Scan");
            break;
        
        case LEDAPI__CONFIG__MODE__CLASSIC:
            ESP_LOGI(TAG, "MODE=Classic");
            break;

        case LEDAPI__CONFIG__MODE__PRESET:
            ESP_LOGI(TAG, "MODE=Preset");
            break;
        
        default:
            ESP_LOGI(TAG, "Mode %d not supported.", config->mode);
            break;
    }
}

void controller_stop() {
    running = 0;
}

void controller_task(void *pParam) {
    while(1) {
        if (running) {
            renderer_render_frame(frame_buffer);
            vTaskDelay(2 / portTICK_PERIOD_MS);
        }
        else {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}
