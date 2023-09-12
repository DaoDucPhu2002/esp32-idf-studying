

#include "Button.h"
#include "stdio.h"

void button_init(Button_Handel_t *buttonX, gpio_num_t gpio_pin)
{
    buttonX->gpio_pin = gpio_pin;
}
void button_Handle(Button_Handel_t *buttonX)
{
    // Handing of noise
    uint8_t sta = gpio_get_level(buttonX->gpio_pin);
    if (sta != buttonX->btn_filter)
    {
        ESP_LOGI("BUTTON", "Handing of noise");
        buttonX->btn_filter = sta;
        buttonX->is_debouncing = 1;
        buttonX->time_debounce = xTaskGetTickCount();
    }
    if (buttonX->is_debouncing && (xTaskGetTickCount() - buttonX->time_debounce) >= 15)
    {
        ESP_LOGI("BUTTON", "Sign the signal");
        buttonX->btn_current = buttonX->btn_filter;
        buttonX->is_debouncing = 0;
    }

    if (buttonX->btn_current != buttonX->btn_last)
    {
        if (buttonX->btn_current == 0)
        {
            ESP_LOGI("BUTTON", "Button Pressing");
            buttonX->is_press_timeout = 1;
            buttonX->time_start_press = xTaskGetTickCount();
            if (buttonX->Press_callback != NULL)
            {
                buttonX->Press_callback();
            }
        }
        buttonX->btn_last = buttonX->btn_current;
    }
    else
    {
        if (buttonX->is_press_timeout && xTaskGetTickCount() - buttonX->time_start_press <= 1000)
        {
            ESP_LOGI("BUTTON", "Button Press Short");
            if (buttonX->PressShort_Callback != NULL)
            {
                buttonX->PressShort_Callback();
            }

            ;
        }
        // ESP_LOGI("BUTTON", "Button Release ");
        if (buttonX->Release_Callback != NULL)
        {
            buttonX->Release_Callback();
        }
        buttonX->is_press_timeout = 0;
    }
    if (buttonX->is_press_timeout && xTaskGetTickCount() - buttonX->time_start_press >= 3000)
    {
        ESP_LOGI("BUTTON", "Button Press Timeout");
        if (buttonX->PressTimeout_Callback != NULL)
        {
            buttonX->PressTimeout_Callback();
        }
        buttonX->is_press_timeout = 0;
    }
}
