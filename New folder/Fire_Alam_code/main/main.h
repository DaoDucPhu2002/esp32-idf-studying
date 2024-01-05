#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

/*IC 74HC4067*/
#define S0 25
#define S1 32
#define S2 33
// #define S3 23

/*PIN Status*/
#define EOLINE 26
#define INSEN 27

#define NOTUSE 0
#define ALARM_STATUS 1
#define FAIL_LINE 2
#define NORMAL_STATUS 3
/*IC 74HC595*/
// #define DATA_PIN 26 // sua
// #define CLK_PIN 27
// #define LAT_PIN 26

/*LED Status*/
#define LED_RED 22
#define LED_WIFI 23
#define LED_GREEN 21
/*Button Config Wifi*/
/*
    Press 5s Access Point
    Press 10s Smart Config
*/
#define BTN_WF_CONF 19

/*Output*/

#define BEEP 5
#define LAMP1 17
#define OUT1 16
#define OUT2 4
#define BUZZER 18