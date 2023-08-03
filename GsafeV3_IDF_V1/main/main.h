
#include <stdio.h>
#include "sdkconfig.h"
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <math.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <stdint.h>
#include <stddef.h>
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "mqtt_client.h"
#define DEBUG_EN 1
#define BUZZER_EN 1
/*Define GPIO*/
#define SA0 5
#define SA1 18
#define SA2 19
#define SA3 21
#define EOLINE 23
#define INSEN 22
#define LAMP 25
#define BELL 33
#define RL1 27
#define RL2 26
/*Wifi indicator*/
#define LED_WF 2
#define LEDR 14
#define LEDG 12
#define BEEP 4
#define VBAT_EN 13
#define V_BOOST 15

/*Define ADC*/
#define ADC_KEY ADC1_CHANNEL_7
#define ADC_RF ADC1_CHANNEL_6
#define ADC_VBAT ADC1_CHANNEL_3
#define ADC_CHARGER ADC_CHANNEL_0

#define BUFF_SIZE 1024 * 2
#define numLine 16

/*Macro to ESP_LOGI*/
#define LOG_DEBUG(TAG, format, ...)               \
    do                                            \
    {                                             \
        if (DEBUG_EN)                             \
        {                                         \
            ESP_LOGI(TAG, format, ##__VA_ARGS__); \
        }                                         \
    } while (0)
/*Operations with bit*/
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))
