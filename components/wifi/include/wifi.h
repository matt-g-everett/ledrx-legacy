#include "freertos/FreeRTOS.h"

#ifndef __WIFI_H
#define __WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

void wifi_initialise(const char *ssid, const char *password);
uint8_t wifi_wait(TickType_t xTicksToWait);

#ifdef __cplusplus
}
#endif

#endif
