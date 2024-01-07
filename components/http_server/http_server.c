#include "http_server.h"

#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

#include "ir_emitter.h"

#define HTTP_TAG    "HTTP"

static esp_err_t get_page_handler(httpd_req_t *req);
static esp_err_t favicon_handler(httpd_req_t *req);
static esp_err_t process_command_handler(httpd_req_t *req);

static http_command_callback_t command_callback = NULL;

static const httpd_uri_t page_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = get_page_handler,
    .user_ctx  = NULL,
};

static const httpd_uri_t favicon_uri = {
    .uri       = "/favicon.ico",
    .method    = HTTP_GET,
    .handler   = favicon_handler,
    .user_ctx  = NULL,
};

static const httpd_uri_t command_uri = {
    .uri       = "/command",
    .method    = HTTP_POST,
    .handler   = process_command_handler,
    .user_ctx  = NULL,
};

static esp_err_t get_page_handler(httpd_req_t *req)
{
    extern const unsigned char webpage_start[] asm("_binary_index_html_start");
    extern const unsigned char webpage_end[] asm("_binary_index_html_end");
    const size_t webpage_size = (webpage_end - webpage_start);

    ESP_LOGI(HTTP_TAG, "GET page");

    httpd_resp_send(req, (const char *)webpage_start, webpage_size);

    return ESP_OK;
}

static esp_err_t favicon_handler(httpd_req_t *req)
{
    extern const unsigned char favicon_start[] asm("_binary_favicon_ico_start");
    extern const unsigned char favicon_end[] asm("_binary_favicon_ico_end");
    const size_t favicon_size = (favicon_end - favicon_start);

    ESP_LOGI(HTTP_TAG, "GET favicon");

    httpd_resp_send(req, (const char *)favicon_start, favicon_size);

    return ESP_OK;
}

static esp_err_t process_command_handler(httpd_req_t *req)
{
    char content[req->content_len + 1];

    httpd_req_recv(req, content, req->content_len);
    content[req->content_len] = '\0';

    ESP_LOGI(HTTP_TAG, "POST content: %s", content);

    if (command_callback != NULL) {
        command_callback(content, req->content_len + 1);
    }

    // Empty response
    httpd_resp_send(req, content, 0);

    return ESP_OK;
}

void http_server_start(http_command_callback_t callback)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    command_callback = callback;

    ESP_LOGI(HTTP_TAG, "Starting sever");

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &page_uri);
        httpd_register_uri_handler(server, &favicon_uri);
        httpd_register_uri_handler(server, &command_uri);
    }
}
