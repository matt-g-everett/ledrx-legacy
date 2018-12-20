#include "freertos/FreeRTOS.h"

#include "esp_log.h"

#include <stdlib.h>

#include "controller.h"
#include "pixels.h"
#include "ledscan.h"
#include "hsv.h"

#define SATURATION 255
#define VALUE 40

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct {
    uint16_t trail_length;
    uint8_t val;
    int16_t current;
    uint8_t forward;
} SCAN_STATE_t;

SCAN_STATE_t scan_state = {};

void * ledscan_create_state(uint16_t trail_length) {
    scan_state.trail_length = trail_length;
    scan_state.val = 60;
    scan_state.current = 0;
    scan_state.forward = 1;

    return (void *)&scan_state;
}

void ledscan_free_state(void * state) {
    // Nothing to free
}

static void rainbow_trail(uint16_t index, uint16_t lit, uint16_t trail_length, uint8_t v, uint8_t s,
        uint8_t *r, uint8_t *g, uint8_t *b) {

    int16_t diff = (int16_t)index - (int16_t)lit;
    uint16_t half_trail = trail_length / 2;

    if (abs(diff) > half_trail) {
        v = 0;
    }
    
    int16_t clip = min(max(diff, -half_trail), half_trail);
    int16_t hue = (clip + half_trail) * (HSV_HUE_STEPS / trail_length);

    fast_hsv2rgb_8bit(hue, s, v, r, g, b);
}

// static void rainbow_wrap(uint16_t index, uint16_t lit, uint16_t trail_length, uint8_t v, uint8_t s,
//         uint8_t *r, uint8_t *g, uint8_t *b) {

//     int16_t diff = (int16_t)index - (int16_t)lit;
//     uint16_t half_trail = trail_length / 2;

    
//     int16_t clip = min(max(diff, -half_trail), half_trail);
//     int16_t hue = (clip + half_trail) * (HSV_HUE_STEPS / trail_length);

//     fast_hsv2rgb_8bit(hue, s, v, r, g, b);
// }

// static void advance_index_wrap(int16_t *index, int16_t trail_length, int16_t num_pixels) {
//     *index++;

//     int16_t trail_multiple = (int16_t)(num_pixels / trail_length) + 1;
//     int16_t virtual_length = trail_multiple * trail_length;
//     if (index > virtual_length) {
//         index = 0;
//     }
// }

static void advance_index(int16_t *index, uint8_t *forward, int16_t min, int16_t max) {
    if (*forward) {
        if (*index < max) {
            *index += 1;
        }
        else {
            *forward = 0;
            *index -= 1;
        }
    }
    else {
        if (*index > min) {
            *index -= 1;
        }
        else {
            *forward = 1;
            *index += 1;
        }
    }
}

void ledscan_calculate_frame(FRAME_t *frame, void *state) {
    SCAN_STATE_t *sstate = (SCAN_STATE_t *)state;

    frame->len = CONFIG_NUM_PIXELS;
    for (uint16_t i = 0; i < frame->len; i++) {
        RGB_t *pixel = frame->data + i;

        rainbow_trail(i, sstate->current, sstate->trail_length, VALUE, SATURATION,
            &(pixel->r), &(pixel->g) ,&(pixel->b));
    }

    advance_index(&(sstate->current), &(sstate->forward), 0, CONFIG_NUM_PIXELS);
}
