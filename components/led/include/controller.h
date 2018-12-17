#include "freertos/FreeRTOS.h"

#include "pixels.h"

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

void controller_initialise();
void controller_set_mode();
void controller_task(void *pParam);

#ifdef __cplusplus
}
#endif

#endif
