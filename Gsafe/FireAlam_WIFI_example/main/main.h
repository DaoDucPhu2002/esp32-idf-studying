#include <stdio.h>
#include "math.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"

#include "esp_mac.h"

#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_SSID "DaoDucPhu"
#define WIFI_PASS "11111111"
#define WIFI_CHANNEL 0
#define MAX_STA_CONN 4
#define MAXINUM_AP 20
#define TAG "Scan "