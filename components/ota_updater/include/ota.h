#ifndef __OTA_H
#define __OTA_H

#ifdef __cplusplus
extern "C" {
#endif

void ota_initialise(const char *cert, const char *version_url, const char *bin_url, const char *version);
void ota_task(void *pParam);

#ifdef __cplusplus
}
#endif

#endif
