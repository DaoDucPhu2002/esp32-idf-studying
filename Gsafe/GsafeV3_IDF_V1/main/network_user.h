#ifndef __NETWORK_USER_H
#define __NETWORK_USER_H
#include "main.h"
#include "esp_http_server.h"
#include "esp_log.h"
/*Wifi_config*/
/*Wifi Info Access Point Mode*/
#define AP_SSID "DaoDucPhu"
#define AP_PASS "12345678"
#define AP_MAX_CONN 4
#define AP_CHANNEL 1
/*Wifi connect maximum retry*/
#define EXAMPLE_ESP_MAXIMUM_RETRY 5

/*Event Group Handler*/
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const int HTTP_POST_DONE_BIT = BIT2;
static const int WIFI_SCAN_DONE_BIT = BIT3;

/*define function*/
void wifi_manager(void);
/*Webserver*/
typedef void (*http_server_handle)(char *data, int len);

httpd_handle_t start_webserver(void);
esp_err_t stop_webserver(httpd_handle_t server);
void http_get_set_callback(void *cb);
void http_post_set_callback(void *cb);
void http_send_response(char *data, int len);
void http_get_wifi_infor_callback(void *cb);
/*EC200*/
#endif