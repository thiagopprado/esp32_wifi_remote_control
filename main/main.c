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
#define IR_CMD_MULTICODE_INTERVAL           pdMS_TO_TICKS(700)
#define IR_CMD_SINGLECODE_REPEAT_NR         3
#define IR_CMD_SINGLECODE_REPEAT_INTERVAL   pdMS_TO_TICKS(60)

typedef struct {
    char* description;
    uint16_t code;
} ir_command_t;

static ir_command_t ir_command[] = {
    {
        .description = "on",
        .code = IR_SKY_ON,
    },
    {
        .description = "off",
        .code = IR_SKY_OFF,
    },
    {
        .description = "guide",
        .code = IR_SKY_GUIDE,
    },
    {
        .description = "list",
        .code = IR_SKY_LIST,
    },
    {
        .description = "info",
        .code = IR_SKY_INFO,
    },
    {
        .description = "esc",
        .code = IR_SKY_ESC,
    },
    {
        .description = "up",
        .code = IR_SKY_UP,
    },
    {
        .description = "left",
        .code = IR_SKY_LEFT,
    },
    {
        .description = "right",
        .code = IR_SKY_RIGHT,
    },
    {
        .description = "down",
        .code = IR_SKY_DOWN,
    },
    {
        .description = "confirm",
        .code = IR_SKY_CONFIRM,
    },
    {
        .description = "menu",
        .code = IR_SKY_MENU,
    },
    {
        .description = "last_ch",
        .code = IR_SKY_LAST_CH,
    },
    {
        .description = "ch_up",
        .code = IR_SKY_CH_UP,
    },
    {
        .description = "ch_down",
        .code = IR_SKY_CH_DOWN,
    },
    {
        .description = "dash",
        .code = IR_SKY_DASH,
    },
    {
        .description = "enter",
        .code = IR_SKY_ENTER,
    },
    {
        .description = "1",
        .code = IR_SKY_1,
    },
    {
        .description = "2",
        .code = IR_SKY_2,
    },
    {
        .description = "3",
        .code = IR_SKY_3,
    },
    {
        .description = "4",
        .code = IR_SKY_4,
    },
    {
        .description = "5",
        .code = IR_SKY_5,
    },
    {
        .description = "6",
        .code = IR_SKY_6,
    },
    {
        .description = "7",
        .code = IR_SKY_7,
    },
    {
        .description = "8",
        .code = IR_SKY_8,
    },
    {
        .description = "9",
        .code = IR_SKY_9,
    },
    {
        .description = "0",
        .code = IR_SKY_0,
    },
};

static QueueHandle_t ir_queue_handler = NULL;

static void http_command_callback(char *content, size_t len);
static void app_send_ir_code(const uint16_t code);
static void app_process_ch_cmd(const char *command);

static void http_command_callback(char *content, size_t len)
{
    char command[IR_CMD_QUEUE_BUFFER_SZ] = "";

    if (len > IR_CMD_QUEUE_BUFFER_SZ) {
        len = IR_CMD_QUEUE_BUFFER_SZ - 1;
    }

    strncpy(command, content, len);
    xQueueSend(ir_queue_handler, command, 0);
}

static void app_send_ir_code(const uint16_t code)
{
    for (uint8_t i = 0; i < IR_CMD_SINGLECODE_REPEAT_NR; i++) {
        ir_emitter_sky(code);
        vTaskDelay(IR_CMD_SINGLECODE_REPEAT_INTERVAL);
    }
}

static void app_process_ch_cmd(const char *command)
{
    const uint16_t ch_cmd_decode[] = {IR_SKY_0, IR_SKY_1, IR_SKY_2, IR_SKY_3, IR_SKY_4,
                                      IR_SKY_5, IR_SKY_6, IR_SKY_7, IR_SKY_8, IR_SKY_9 };

    uint16_t ch_value = 0;
    if (sscanf(command, "ch: %hd", &ch_value) == 0) {
        // Not found
        return;
    }

    int code_size = 0;
    uint16_t ch_ir_code[5];

    while (ch_value > 0) {
        ch_ir_code[code_size++] = ch_cmd_decode[ch_value % 10];
        ch_value /= 10;
    }

    for (int code_idx = code_size - 1; code_idx >= 0; code_idx--) {
        app_send_ir_code(ch_ir_code[code_idx]);
        vTaskDelay(IR_CMD_MULTICODE_INTERVAL);
    }

    ir_emitter_sky(IR_SKY_ENTER);
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
                app_send_ir_code(ir_command[i].code);
            }
        }

        app_process_ch_cmd(received_cmd);
    }
}
