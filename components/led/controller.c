#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/rmt.h"

#include "renderer.h"
#include "ledscan.h"

#define FRAME_BUFFER_SIZE CONFIG_FRAME_BUFFER_SIZE

FRAME_t frame_buffer[FRAME_BUFFER_SIZE];

void controller_initialise() {
    renderer_initialise();
    renderer_set_strategy(ledscan_calculate_frame, ledscan_create_state(200));
    // xCreateBinarySemaphoreStatic();
}

void controller_set_mode() {
    
}

void controller_task(void *pParam) {
    while(1) {
        renderer_render_frame(frame_buffer);
        //vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
