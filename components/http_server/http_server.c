#include "http_server.h"

#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

#define HTTP_TAG    "HTTP"

void http_server_start(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(HTTP_TAG, "Starting sever");

    if (httpd_start(&server, &config) == ESP_OK) {
}
