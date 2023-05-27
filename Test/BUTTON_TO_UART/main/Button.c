#include "Button.h"
#include "stdio.h"

input_callback_t Press_callback = NULL;
input_callback_t PressShort_Callback = NULL;
input_callback_t PressTimeout_Callback = NULL;
input_callback_t Release_Callback = NULL;

void buton_init(Button_Handel *buttonX, gpio_num_t gpio_pin)
{
    buttonX->gpio_pin = gpio_pin;
}
void button_Handle(Button_Handel *buttonX)
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
            buttonX->state = BUTTON_PRESS_STATE;
        }
        buttonX->btn_last = buttonX->btn_current;
    }
    else
    {
        if (buttonX->is_press_timeout && xTaskGetTickCount() - buttonX->time_start_press <= 1000)
        {
            ESP_LOGI("BUTTON", "Button Press Short");
            buttonX->state = BUTTON_PRESS_SHORT_STATE;
        }
        ESP_LOGI("BUTTON", "Button Release ");
        buttonX->state = BUTTON_RELEASE_STATE;
        buttonX->is_press_timeout = 0;
    }
    if (buttonX->is_press_timeout && xTaskGetTickCount() - buttonX->time_start_press >= 3000)
    {
        ESP_LOGI("BUTTON", "Button Press Timeout");
        buttonX->state = BUTTON_PRESS_TIMEOUT_STATE;
        buttonX->is_press_timeout = 0;
    }
}

int get_state_buttonx(Button_Handel *buttonX)
{
    return buttonX->state;
}