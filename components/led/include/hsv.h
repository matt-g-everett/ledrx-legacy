#include "freertos/FreeRTOS.h"

#include "pixels.h"

#ifndef __HSV_H
#define __HSV_H

#define HSV_HUE_SEXTANT	256
#define HSV_HUE_STEPS (6 * HSV_HUE_SEXTANT)

#ifdef __cplusplus
extern "C" {
#endif

void hsv2rgb(HSV_t hsv, RGB_t *rgb);
void fast_hsv2rgb_8bit(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g , uint8_t *b);

#ifdef __cplusplus
}
#endif

#endif
