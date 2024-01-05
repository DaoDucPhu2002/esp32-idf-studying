#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_config.h"
#include "driver/gpio.h"

#define SA0 5
#define SA1 18
#define SA2 19
#define SA3 21
#define INSEN 22
#define EOLINE 23
#define RL1 25
#define RL2 33
#define RL3 27
#define RL4 26
#define BEEP 4
#define LED_GREEN 14
#define LED_RED 12
// code ADC switch in để config wifi

void io_init()
{
    gpio_config_t io_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1 << SA0) | (1 << SA1) | (1 << SA2) | (1 << SA3) | (1 << RL1) | (1 << RL2) | (1 << RL3) | (1 << RL4) |
                        (1 << LED_GREEN) | (1 << LED_RED) | (1 << BEEP),
    };
    gpio_config(io_config);
    gpio_config_t input_config = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 
    }
}
void button_handle(void *xParameter)
{
    // config button
}

void app_main(void)
{

    xTaskCreate(wifi_manager, "WIFI", 2048, NULL, 3, NULL);
    xTaskCreate(button_handle, "Button Handle", 2048, NULL, 3, NULL);
}
