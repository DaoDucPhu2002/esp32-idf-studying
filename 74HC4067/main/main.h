#include <stdio.h>
#include "sdkconfig.h"
#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#define EOLINE 23
#define INSEN 22
#define SA0 5
#define SA1 18
#define SA2 19
#define SA3 21

#define RL1 25
#define RL2 33
#define RL3 27
#define RL4 26

void GPIO_config(void)
{
    gpio_config_t io_config = {};
    io_config.pin_bit_mask = (1 << SA0) | (1 << SA1) | (1 << SA2) | (1 << SA3) | (1 << RL1) | (1 << RL2) | (1 << RL3) | (1 << RL4);
    io_config.mode = GPIO_MODE_DEF_OUTPUT;
    gpio_config(&io_config);

    io_config.pin_bit_mask = (1 << EOLINE) | (1 << INSEN);
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&io_config);
}
void ADC_config()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11); // DB 11 dai dien ap toi da la 3,3v
    ESP_LOGI("ADC", "ADC config is done");
}
