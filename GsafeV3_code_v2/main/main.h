#include <stdio.h>
#include "sdkconfig.h"
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <math.h>

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
#define LOGI_EN 1

#define UART_SIM UART_NUM_2
#define BUFF_SIZE 1024 * 2
