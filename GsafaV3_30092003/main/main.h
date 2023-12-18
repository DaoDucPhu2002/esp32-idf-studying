#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define DEBUG_EN 1
#define LOG_DEBUG(TAG, format, ...)               \
    do                                            \
    {                                             \
        if (DEBUG_EN)                             \
        {                                         \
            ESP_LOGI(TAG, format, ##__VA_ARGS__); \
        }                                         \
    } while (0)

extern const char *mqtt_hostname;
extern const uint32_t mqtt_port;
extern const char *mqtt_user;
extern const char *mqtt_pass;
extern const char *topic_server;
extern char txt_subscribe[];
extern char ESP_ID[13];
extern char IMEI[15];