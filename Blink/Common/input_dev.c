#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include "input_dev.h"

typedef enum 
{
    GPIO_RISING,
    GPIO_FALLING,
    GPIO_ENY_EDLE,
}interrupt_type_edle_t ;


esp_err_t input_init(gpio_num_t gpio_num,interrupt_type_edle_t type)
{
   gpio_pad_select_gpio(gpio_num);
   gpio_set_direction(gpio_num,GPIO_MODE_INPUT);
   gpio_set_pull_mode(gpio_num,)

}

void input_io_get_level(gpio_num_t gpio_num)
{
    return gpio_get_level(gpio_num);
}

void input_set_callback(void *callback)
{

}