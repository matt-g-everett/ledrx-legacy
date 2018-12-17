#include "freertos/FreeRTOS.h"

#include "pixels.h"

#ifndef __RENDERER_H
#define __RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct frame_t {
    uint16_t len;
    RGB_t data[CONFIG_NUM_PIXELS];
} FRAME_t;

typedef void (*FrameCalculator)(FRAME_t* frame, void *state);

void renderer_initialise();
void renderer_task(void *pParam);
void renderer_set_strategy(FrameCalculator frame_calc, void *state);
void renderer_render_frame();

#ifdef __cplusplus
}
#endif

#endif
