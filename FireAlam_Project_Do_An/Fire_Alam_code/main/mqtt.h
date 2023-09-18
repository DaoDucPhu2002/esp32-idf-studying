#ifndef _MQTT_APP_H
#define _MQTT_APP_H
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

// #include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

typedef struct
{
    char buf[256];
    size_t offset;
} json_gen_test_result_t;

typedef struct
{
    char *data;
    int len;
} mqtt_data_handle_t;

typedef struct
{
    int sta_button1;
    int sta_button2;
    int64_t timer_button1;
    int64_t timer_button2;
} Button_status_t;
typedef void (*mqtt_data_handle_callback_t)(char *data, uint16_t len);

void mqtt_app_start(void);
uint8_t mqtt_app_public_callback();
// void readMAC();
void send_data_to_server(uint8_t *data);
void get_mqtt_data(char *data, int len);
void json_gen_perform_test(json_gen_test_result_t *result, int sta_button1, int sta_button2, uint64_t time_button1, uint64_t time_button2);
void mqtt_set_callback(void *cb);
#endif