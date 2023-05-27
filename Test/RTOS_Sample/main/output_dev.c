#include "output_dev.h"
#include "driver/gpio.h"

void output_create(int pin)
{
    gpio_config_t GPIO_config = {};
    GPIO_config.pin_bit_mask = (1 << pin);
    GPIO_config.mode = GPIO_MODE_OUTPUT;
    GPIO_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    GPIO_config.pull_up_en = GPIO_PULLUP_DISABLE;
    GPIO_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&GPIO_config);
}

void output_set(int pin)
{
    gpio_set_level(pin, 1);
}

void output_clear(int pin)
{
    gpio_set_level(pin, 0);
}

void output_toggle(int pin)
{
    int state = gpio_get_level(pin);
    if (state)
    {
        gpio_set_level(pin, 0);
    }
    else
    {
        gpio_set_level(pin, 1);
    }
}