#include "freertos/FreeRTOS.h"

#include "pixels.h"

#ifndef __LEDCONTROL_H
#define __LEDCONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct frame_t {
    uint16_t len;
    RGB_t *data;
} FRAME_t;

typedef FRAME_t* (*FrameCalculator)(void *state);

void led_control_set_strategy(FrameCalculator frame_calc, void *state);
void led_control_task(void *pParam);
void led_control_initialise(uint16_t num_pixels);

#ifdef __cplusplus
}
#endif

#endif
