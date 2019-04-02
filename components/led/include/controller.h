#include "freertos/FreeRTOS.h"

#include "coapapi.pb-c.h"
#include "pixels.h"

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

void controller_initialise();
void controller_configure(Ledapi__Config *);
void controller_task(void *pParam);
void controller_stop();

#ifdef __cplusplus
}
#endif

#endif
