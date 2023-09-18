#include "main.h"
#include <stdio.h>
#include "driver/gpio.h"

void alam_config_gpio()
{
    gpio_config_t io_config;
    io_config.pin_bit_mask = (1 << S0) | (1 << S1) | (1 << S2) | (1 << S3) | (1 << EOLINE) | (1 << INSEN) | (1 << CLK_PIN) | (1 << LAT_PIN) | (1 << DATA_PIN) |
                             (1 << LED_GREEN) | (1 << LED_RED) | (1 << LED_WIFI);
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_config);

    gpio_config_t io_input;
    io_input.pin_bit_mask = (1 << EOLINE) | (1 << INSEN) | (1 << BTN_WF_CONF);
    io_input.mode = GPIO_MODE_INPUT;
    io_input.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_input.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_input);
}
void app_main(void)
{
}
