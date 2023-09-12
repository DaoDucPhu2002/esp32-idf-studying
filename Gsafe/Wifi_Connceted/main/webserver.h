#ifndef __APP_HTTP_SERVER_H
#define __APP_HTTP_SERVER_H

#include "esp_http_server.h"

typedef void (*http_server_handle)(char *data, int len);

httpd_handle_t start_webserver(void);
esp_err_t stop_webserver(httpd_handle_t server);
void http_get_set_callback(void *cb);
void http_post_set_callback(void *cb);
void http_send_response(char *data, int len);

#endif
