#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/rmt.h"

#include "renderer.h"
#include "ledscan.h"
#include "ws2811.h"

#define FRAME_BUFFER_SIZE CONFIG_FRAME_BUFFER_SIZE

FRAME_t frame_buffer[FRAME_BUFFER_SIZE];

uint8_t running = 0;

void controller_initialise() {
    running = 1;
    renderer_initialise();
    renderer_set_strategy(ledscan_calculate_frame, ledscan_create_state(220));
    // xCreateBinarySemaphoreStatic();
}

void controller_set_mode() {
    
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
