#ifndef __CONFIG_WIFI_H
#define __CONFIG_WIFI_H

#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_smartconfig.h"

// void init_nvs();
int is_connected_to_wifi();
void wifi_manager(void *pvParameter);
int8_t GetRSSI_Wifi();
#endif