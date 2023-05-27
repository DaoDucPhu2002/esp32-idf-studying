#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>

typedef struct
{
    // GPIO
    gpio_num_t gpio_pin;
    // Handle
    uint8_t btn_current;
    uint8_t btn_last;
    uint8_t btn_filter;
    uint8_t is_debouncing;
    uint32_t time_debounce;
    uint32_t time_start_press;
    uint8_t is_press_timeout;
} Button_Handel;
