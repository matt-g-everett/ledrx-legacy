/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
//#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "soc/rtc.h"
#include "driver/rmt.h"

extern "C" void app_main()
{
    printf("Starting...\n");

    while(true) {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        printf("Doing stuff...\n");
    }
}
