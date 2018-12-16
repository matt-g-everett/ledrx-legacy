#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/rmt.h"

#include "ledcontrol.h"
#include "pixels.h"

#define RMT_PIN_SEL GPIO_SEL_27
#define RMT_PIN GPIO_NUM_27

#define ONE_HIGH_TICKS 12
#define ONE_LOW_TICKS 13
#define ZERO_HIGH_TICKS 5
#define ZERO_LOW_TICKS 20

const static char *TAG = "ledrx_ledcontrol";

rmt_item32_t high = {
    .duration0 = ONE_HIGH_TICKS, .level0 = 1,
    .duration1 = ONE_LOW_TICKS, .level1 = 0
};
rmt_item32_t low = {
    .duration0 = ZERO_HIGH_TICKS, .level0 = 1,
    .duration1 = ZERO_LOW_TICKS, .level1 = 0
};

uint16_t pixel_count;
uint16_t subpixel_count;
FrameCalculator calculate_frame = NULL;
void *state = NULL;
rmt_item32_t *items;

static void render_frame(FRAME_t *frame) {
    uint8_t *subpixels;
    uint8_t subpixel;
    uint32_t bit = 0;
    for (uint16_t i = 0; i < frame->len; i++) {
        subpixels = frame->data[i].subpixels;
        for (int8_t s = 0; s < 3; s++) {
            subpixel = subpixels[s];
            for (int8_t b = 7; b >= 0; b++) {
                if ((1 << b) & subpixel) {
                    items[bit] = high;
                }
                else {
                    items[bit] = low;
                }

                bit++;
            }
        }
    }

    rmt_write_items(RMT_CHANNEL_0, items, subpixel_count, true);
}

static void configure_rmt() {
    gpio_config_t gpioConf;
    gpioConf.pin_bit_mask = RMT_PIN_SEL;
    gpioConf.mode = GPIO_MODE_OUTPUT;
    gpioConf.intr_type = GPIO_INTR_DISABLE;
    gpioConf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpioConf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&gpioConf);

    rmt_tx_config_t rmtTXConf;
    rmtTXConf.carrier_en = false;
    rmtTXConf.loop_en = false;
    rmtTXConf.idle_output_en = false;
    rmtTXConf.idle_level = RMT_IDLE_LEVEL_LOW;

    rmt_config_t rmtConfig;
    rmtConfig.channel = RMT_CHANNEL_0;
    rmtConfig.clk_div = 8;
    rmtConfig.gpio_num = RMT_PIN;
    rmtConfig.mem_block_num = 1;
    rmtConfig.rmt_mode = RMT_MODE_TX;
    rmtConfig.tx_config = rmtTXConf;
    rmt_config(&rmtConfig);

    rmt_driver_install(RMT_CHANNEL_0, 0, 0);
}

void led_control_set_strategy(FrameCalculator frame_calc, void *pState) {
    calculate_frame = frame_calc;
    state = pState;
}

void led_control_task(void *pParam) {
    FRAME_t *frame;
    
    while(1) {
        if (calculate_frame) {
            frame = calculate_frame(state);
            render_frame(frame);
        }
    }
}

void led_control_initialise(uint16_t num_pixels) {
    configure_rmt();
    pixel_count = num_pixels;
    ESP_LOGI(TAG, "*****pixel_count: %d", pixel_count);
    subpixel_count = pixel_count * 3;
    ESP_LOGI(TAG, "*****subpixel_count: %d", subpixel_count);
    items = (rmt_item32_t*)malloc(subpixel_count * sizeof(rmt_item32_t));
    // xCreateBinarySemaphoreStatic();
}
