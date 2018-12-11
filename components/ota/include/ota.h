#ifndef __OTA_H
#define __OTA_H

#ifdef __cplusplus
extern "C" {
#endif

void log_partition_info(void);
void ota_task(void *pParam);

#ifdef __cplusplus
}
#endif

#endif
