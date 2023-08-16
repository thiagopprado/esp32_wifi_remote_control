#include <stdio.h>
#include <stdbool.h>

// FreeRTOS
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

// ESP
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "driver/gpio.h"

#include "ir_emitter.h"
#include "wifi_ctl.h"

void app_main(void)
{
    nvs_flash_init();
    esp_event_loop_create_default();

    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_pullup_en(GPIO_NUM_0);

    ir_emitter_setup();

    wifi_ctl_setup();

    while (true) {
        if (gpio_get_level(GPIO_NUM_0) == false) {
            ir_emitter_sky(IR_SKY_ON);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
