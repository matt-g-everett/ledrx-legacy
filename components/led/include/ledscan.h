#include "freertos/FreeRTOS.h"

#include "renderer.h"

#ifndef __LEDSCAN_H
#define __LEDSCAN_H

#ifdef __cplusplus
extern "C" {
#endif

void * ledscan_create_state(uint16_t trail_length);
void ledscan_free_state(void * state);
void ledscan_calculate_frame(FRAME_t *frame, void *state);

#ifdef __cplusplus
}
#endif

#endif
