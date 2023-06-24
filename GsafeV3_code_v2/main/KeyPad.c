#include "Keypad.h"

void KeyPad_Mode_Callback(void)
{
}
void KeyPad_Bell_Callback(void)
{
}
void KeyPad_Check_Callback(void)
{
}

KeyPad_State get_keypad_state(void)
{
    int x;
    static KeyPad_State sw_sta;
    uint16_t sw_adc_value = adc1_get_raw(ADC1_CHANNEL_7);
    //  printf("%d\n", sw_adc_value);
    if (sw_adc_value <= 2100)
    {
        sw_sta = MODE_BUTTON_PRESS;
        if (LOGI_EN)
            ESP_LOGI(TAG_KEYPAD, "BUTTON is state:MODE_BUTTON_PRESS");
    }
    else if (sw_adc_value <= 2600)
    {
        sw_sta = BELL_BUTTON_PRESS;
        if (LOGI_EN)
            ESP_LOGI(TAG_KEYPAD, "BUTTON is state: BELL_BUTTON_PRESS");
    }
    else if (sw_adc_value <= 3500)
    {
        sw_sta = CHECK_BUTTON_PRESS;
        if (LOGI_EN)
            ESP_LOGI(TAG_KEYPAD, "BUTTON is state: CHECK_BUTTON_PRESS");
    }
    else
    {
        sw_sta = BUTTON_RELEASE;
    }

    return sw_sta;
}