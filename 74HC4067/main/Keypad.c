#include "Keypad.h"

KeyPad_State get_keypad_state(void)
{
    static KeyPad_State sw_sta;
    uint8_t sw_adc_value = adc1_get_raw(ADC1_CHANNEL_7);
    if (sw_adc_value > 1900 && sw_adc_value < 2100)
    {
        sw_sta = MODE_BUTTON_PRESS;
    }
    else if (sw_adc_value > 2650 && sw_adc_value < 2850)
    {
        sw_sta = BELL_BUTTON_PRESS;
    }
    else if (sw_adc_value > 3000 && sw_adc_value < 3200)
    {
        sw_sta = CHECK_BUTTON_PRESS;
    }
    else
    {
        sw_sta = BUTTON_RELEASE;
    }

    ESP_LOGI(TAG_BTN, "BUTTON is state: %d", sw_sta);
    return sw_sta;
}