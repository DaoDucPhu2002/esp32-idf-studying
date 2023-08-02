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
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_smartconfig.h"
#include "wifi_config.h"
#include "mqtt.h"
#include "nvs_app.h"

#define SA0 5
#define SA1 18
#define SA2 19
#define SA3 21
#define EOLINE 23
#define INSEN 22
#define RL1 25
#define RL2 33
#define RL3 27
#define RL4 26

#define LEDR 14
#define LEDG 12
#define BEEP 4
#define VBAT_EN 13

#define ADC_KEY ADC1_CHANNEL_7
#define ADC_RF ADC1_CHANNEL_6
#define ADC_VBAT ADC1_CHANNEL_3
#define ADC_CHARGER ADC_CHANNEL_0

#define BUFF_SIZE 1024 * 2
/*Define user struct */






