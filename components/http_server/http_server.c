#include "http_server.h"

#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

#define HTTP_TAG    "HTTP"

static esp_err_t get_page_handler(httpd_req_t *req);

static const httpd_uri_t page_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = get_page_handler,
    .user_ctx  = NULL,
};

static esp_err_t get_page_handler(httpd_req_t *req)
{
    extern const unsigned char webpage_start[] asm("_binary_index_html_start");
    extern const unsigned char webpage_end[]   asm("_binary_index_html_end");
    const size_t webpage_size = (webpage_end - webpage_start);

    ESP_LOGI(HTTP_TAG, "GET page");

    httpd_resp_send(req, (const char *)webpage_start, webpage_size);

    return ESP_OK;
}

void http_server_start(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(HTTP_TAG, "Starting sever");

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &page_uri);
    }
}
