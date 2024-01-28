#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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

#include "ir_emitter.h"
#include "wifi_ctl.h"

#include "http_server.h"

#define IR_CMD_QUEUE_BUFFER_SZ              20
#define IR_CMD_QUEUE_SZ                     10
#define IR_CMD_NR                           (sizeof(ir_command) / sizeof (ir_command_t))
#define IR_CMD_MULTICODE_NR                 5
#define IR_CMD_MULTICODE_INTERVAL           pdMS_TO_TICKS(1000)
#define IR_CMD_SINGLECODE_REPEAT_NR         3
#define IR_CMD_SINGLECODE_REPEAT_INTERVAL   pdMS_TO_TICKS(60)

typedef struct {
    char* description;
    uint16_t code[IR_CMD_MULTICODE_NR];
    uint8_t code_nr;
} ir_command_t;

static ir_command_t ir_command[] = {
    {
        .description = "on",
        .code = { IR_SKY_ON },
        .code_nr = 1,
    },
    {
        .description = "off",
        .code = { IR_SKY_OFF },
        .code_nr = 1,
    },
    {
        .description = "guide",
        .code = { IR_SKY_GUIDE },
        .code_nr = 1,
    },
    {
        .description = "list",
        .code = { IR_SKY_LIST },
        .code_nr = 1,
    },
    {
        .description = "info",
        .code = { IR_SKY_INFO },
        .code_nr = 1,
    },
    {
        .description = "esc",
        .code = { IR_SKY_ESC },
        .code_nr = 1,
    },
    {
        .description = "up",
        .code = { IR_SKY_UP },
        .code_nr = 1,
    },
    {
        .description = "left",
        .code = { IR_SKY_LEFT },
        .code_nr = 1,
    },
    {
        .description = "right",
        .code = { IR_SKY_RIGHT },
        .code_nr = 1,
    },
    {
        .description = "down",
        .code = { IR_SKY_DOWN },
        .code_nr = 1,
    },
    {
        .description = "confirm",
        .code = { IR_SKY_CONFIRM },
        .code_nr = 1,
    },
    {
        .description = "menu",
        .code = { IR_SKY_MENU },
        .code_nr = 1,
    },
    {
        .description = "last_ch",
        .code = { IR_SKY_LAST_CH },
        .code_nr = 1,
    },
    {
        .description = "ch_up",
        .code = { IR_SKY_CH_UP },
        .code_nr = 1,
    },
    {
        .description = "ch_down",
        .code = { IR_SKY_CH_DOWN },
        .code_nr = 1,
    },
    {
        .description = "dash",
        .code = { IR_SKY_DASH },
        .code_nr = 1,
    },
    {
        .description = "enter",
        .code = { IR_SKY_ENTER },
        .code_nr = 1,
    },
    {
        .description = "1",
        .code = { IR_SKY_1 },
        .code_nr = 1,
    },
    {
        .description = "2",
        .code = { IR_SKY_2 },
        .code_nr = 1,
    },
    {
        .description = "3",
        .code = { IR_SKY_3 },
        .code_nr = 1,
    },
    {
        .description = "4",
        .code = { IR_SKY_4 },
        .code_nr = 1,
    },
    {
        .description = "5",
        .code = { IR_SKY_5 },
        .code_nr = 1,
    },
    {
        .description = "6",
        .code = { IR_SKY_6 },
        .code_nr = 1,
    },
    {
        .description = "7",
        .code = { IR_SKY_7 },
        .code_nr = 1,
    },
    {
        .description = "8",
        .code = { IR_SKY_8 },
        .code_nr = 1,
    },
    {
        .description = "9",
        .code = { IR_SKY_9 },
        .code_nr = 1,
    },
    {
        .description = "0",
        .code = { IR_SKY_0 },
        .code_nr = 1,
    },
    {
        .description = "globo",
        .code = { IR_SKY_4, IR_SKY_0, IR_SKY_5, IR_SKY_ENTER },
        .code_nr = 4,
    },
    {
        .description = "sbt",
        .code = { IR_SKY_4, IR_SKY_0, IR_SKY_9, IR_SKY_ENTER },
        .code_nr = 4,
    },
    {
        .description = "espn",
        .code = { IR_SKY_5, IR_SKY_9, IR_SKY_8, IR_SKY_ENTER },
        .code_nr = 4,
    },
    {
        .description = "premiere",
        .code = { IR_SKY_6, IR_SKY_3, IR_SKY_3, IR_SKY_ENTER },
        .code_nr = 4,
    },
    {
        .description = "sportv",
        .code = { IR_SKY_4, IR_SKY_3, IR_SKY_9, IR_SKY_ENTER },
        .code_nr = 4,
    },
    {
        .description = "history",
        .code = { IR_SKY_4, IR_SKY_7, IR_SKY_6, IR_SKY_ENTER },
        .code_nr = 4,
    },
    {
        .description = "tnt",
        .code = { IR_SKY_5, IR_SKY_0, IR_SKY_8, IR_SKY_ENTER },
        .code_nr = 4,
    },
    {
        .description = "globo news",
        .code = { IR_SKY_4, IR_SKY_4, IR_SKY_0, IR_SKY_ENTER },
        .code_nr = 4,
    },
    {
        .description = "turbo",
        .code = { IR_SKY_4, IR_SKY_7, IR_SKY_2, IR_SKY_ENTER },
        .code_nr = 4,
    },
    {
        .description = "cnn",
        .code = { IR_SKY_5, IR_SKY_7, IR_SKY_7, IR_SKY_ENTER },
        .code_nr = 4,
    },
};

static QueueHandle_t ir_queue_handler = NULL;

static void http_command_callback(char *content, size_t len);
static void app_send_command(const ir_command_t *command);

static void http_command_callback(char *content, size_t len)
{
    char command[IR_CMD_QUEUE_BUFFER_SZ] = "";

    if (len > IR_CMD_QUEUE_BUFFER_SZ) {
        len = IR_CMD_QUEUE_BUFFER_SZ - 1;
    }

    strncpy(command, content, len);
    xQueueSend(ir_queue_handler, command, 0);
}

static void app_send_command(const ir_command_t *command)
{
    for (uint8_t code_idx = 0; code_idx < command->code_nr; code_idx++) {
        for (uint8_t i = 0; i < IR_CMD_SINGLECODE_REPEAT_NR; i++) {
            ir_emitter_sky(code);
            vTaskDelay(IR_CMD_SINGLECODE_REPEAT_INTERVAL);
        }

        vTaskDelay(IR_CMD_MULTICODE_INTERVAL);
    }
}

void app_main(void)
{
    nvs_flash_init();
    esp_event_loop_create_default();

    ir_emitter_setup();
    wifi_ctl_setup();
    http_server_start(http_command_callback);

    ir_queue_handler = xQueueCreate(IR_CMD_QUEUE_SZ, IR_CMD_QUEUE_BUFFER_SZ * sizeof(char));

    while (true) {
        char received_cmd[IR_CMD_QUEUE_BUFFER_SZ] = "";
        xQueueReceive(ir_queue_handler, received_cmd, portMAX_DELAY);

        for (size_t i = 0; i < IR_CMD_NR; i++) {
            if (strcmp(received_cmd, ir_command[i].description) == 0) {
                app_send_command(&ir_command[i]);
            }
        }
    }
}
