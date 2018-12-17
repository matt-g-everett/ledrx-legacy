#include "freertos/FreeRTOS.h"

#include "driver/rmt.h"
#include "esp_log.h"

#include "pixels.h"
#include "renderer.h"

#define NUM_PIXELS CONFIG_NUM_PIXELS
#define NUM_SUB_PIXELS (NUM_PIXELS * 3)
#define NUM_BITS (NUM_SUB_PIXELS * 8)

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

uint16_t subpixel_count = NUM_PIXELS * 3;
FrameCalculator calculate_frame = NULL;
void *state = NULL;
rmt_item32_t items[NUM_BITS];

static void send_frame(FRAME_t *frame) {
    //ESP_LOGI(TAG, "Sending frame with length %d.", frame->len);

    uint8_t *subpixels;
    uint8_t subpixel;
    uint32_t bit = 0;
    for (uint16_t i = 0; i < frame->len; i++) {
        //ESP_LOGI(TAG, "R %d G %d B %d.", frame->data[i].r, frame->data[i].g, frame->data[i].b);
        subpixels = frame->data[i].subpixels;
        for (int8_t s = 0; s < 3; s++) {
            subpixel = subpixels[s];
            //ESP_LOGI(TAG, "subpixel %d.", subpixel);

            for (int8_t b = 7; b >= 0; b--) {
                //ESP_LOGI(TAG, "bit %d.", bit);

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

    //ESP_LOGI(TAG, "RMT bits %d.", bit);
    rmt_write_items(RMT_CHANNEL_0, items, bit, true);
}

static void configure_rmt() {
    gpio_config_t gpioConf;
    gpioConf.pin_bit_mask = RMT_PIN_SEL;
    gpioConf.mode = GPIO_MODE_OUTPUT;
    gpioConf.intr_type = GPIO_INTR_DISABLE;
    gpioConf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpioConf.pull_up_en = GPIO_PULLUP_ENABLE;
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

void renderer_set_strategy(FrameCalculator frame_calc, void *pState) {
    calculate_frame = frame_calc;
    state = pState;
}

void renderer_render_frame(FRAME_t *frame) {
    //ESP_LOGI(TAG, "calculate_frame %d.", (uint32_t)calculate_frame);
    if (calculate_frame) {
        calculate_frame(frame, state);
        //ESP_LOGI(TAG, "frame->len %d.", frame->len);
        send_frame(frame);
    }
}

void renderer_initialise() {
    configure_rmt();
}
