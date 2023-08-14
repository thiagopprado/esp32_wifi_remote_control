#include <stdio.h>
#include <stdbool.h>

// ESP
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_err.h"

#include "ir_emitter.h"

void app_main(void)
{
    ir_emitter_setup();

    while (true) {

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}