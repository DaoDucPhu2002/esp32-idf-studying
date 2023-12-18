#ifndef _MQTT_APP_H
#define _MQTT_APP_H
#include "main.h"
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

#include "esp_log.h"
#include "mqtt_client.h"
void mqtt_app_start(void);
uint8_t mqtt_app_public_callback();
void readMAC();
void send_data_to_server(uint8_t *data);
#endif