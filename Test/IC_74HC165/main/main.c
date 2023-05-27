#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "sdkconfig.h"
#include "esp_log.h"

#define HC_DS 13    // QH // chân 9 //in
#define HC_CLK 15   // CP // chân 2 //out
#define HC_Latch 15 // CP // chân 1 //out
#define IC_TAG "74HC165"
int i;
int value;

void config_GPIO(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1 << HC_Latch) | (1 << HC_CLK);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1 << HC_DS);
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);
}

void IC_74HC165_Handle(void)
{
    i = 0;
    value = 0;

    gpio_set_level(HC_Latch, 0);
    vTaskDelay(2 / portTICK_PERIOD_MS);
    gpio_set_level(HC_Latch, 1);
    vTaskDelay(2 / portTICK_PERIOD_MS);
    printf("Switch Status: \n");
    for (i = 0; i < 16; i++)
    {
        int sta = gpio_get_level(HC_DS);
        printf("Pin %d : %d \n", i, sta);
        gpio_set_level(HC_CLK, 1);
        vTaskDelay(2 / portTICK_PERIOD_MS);
        gpio_set_level(HC_CLK, 0);
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
    printf("Value in decimal %d\n", value);
}

void app_main(void)
{
    config_GPIO();
    gpio_set_level(HC_Latch, 1);
    gpio_set_level(HC_CLK, 0);
    while (1)
    {
        IC_74HC165_Handle();
    }
}
