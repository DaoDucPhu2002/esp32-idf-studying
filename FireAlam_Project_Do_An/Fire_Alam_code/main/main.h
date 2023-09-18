#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

/*IC 74HC4067*/
#define S0 19
#define S1 21
#define S2 22
#define S3 23

/*PIN Status*/
#define EOLINE 26
#define INSEN 27

/*IC 74HC595*/
#define DATA_PIN 32
#define CLK_PIN 33
#define LAT_PIN 34

/*LED Status*/
#define LED_RED 13
#define LED_WIFI 14
#define LED_GREEN 15
/*Button Config Wifi*/
/*
    Press 5s Access Point
    Press 10s Smart Config
*/
#define BTN_WF_CONF 4