#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stddef.h>

typedef void (*http_command_callback_t)(char* content, size_t len);

void http_server_start(http_command_callback_t callback);

#endif /** HTTP_SERVER_H */