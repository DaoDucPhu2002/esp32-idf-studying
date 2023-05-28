#include "Keypad.h"

KeyPad_State get_keypad_state(void)
{
    static KeyPad_State sw_sta;
    uint16_t sw_adc_value = adc1_get_raw(ADC1_CHANNEL_7);
  //  printf("%d\n", sw_adc_value);
    if (sw_adc_value <= 2100)
    {
        sw_sta = MODE_BUTTON_PRESS;
        ESP_LOGI(TAG_BTN, "BUTTON is state: %s", "MODE_BUTTON_PRESS");
    }
    else if (sw_adc_value <= 2600)
    {
        sw_sta = BELL_BUTTON_PRESS;
        ESP_LOGI(TAG_BTN, "BUTTON is state: %s", "BELL_BUTTON_PRESS");
    }
    else if (sw_adc_value <= 3500)
    {
        sw_sta = CHECK_BUTTON_PRESS;
        ESP_LOGI(TAG_BTN, "BUTTON is state: %s", "CHECK_BUTTON_PRESS");
    }
    else
    {
        sw_sta = BUTTON_RELEASE;
    }

    return sw_sta;
}