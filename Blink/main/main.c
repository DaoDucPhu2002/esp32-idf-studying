#include <stdio.h>
#include "sdkconfig.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <driver/gpio.h>
#include "input_dev.h"

#define BLINK_GPIO GPIO_NUM_2
void button_Handle( int pin)
{
    static int x =0;
    gpio_set_level(BLINK_GPIO,x);
    x=1-x;
}

void app_main(void)
{
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO,GPIO_MODE_OUTPUT);
    input_init(GPIO_NUM_0,HI_TO_LO);

    input_set_callback(button_Handle);
}
