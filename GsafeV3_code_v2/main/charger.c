#include "Charger.h"
#include "main.h"
int temp = 0;
uint16_t vbat_adc;
uint16_t vbat_volt;
uint16_t get_value_vbat(void)
{
    temp += 1;
    vbat_adc += adc1_get_raw(ADC_VBAT);
    if (temp > 10)
    {
        vbat_adc /= 10;
        vbat_volt = vbat_adc / 3 - (ADC_VBAT - 1800) / 50;
        if (LOGI_EN)
            ESP_LOGI("VBAT", "%d", vbat_volt);
    }
    return vbat_volt;
}

uint16_t get_state_charger(void)
{
    uint16_t value_charg = adc1_get_raw(ADC_CHARGER);
    ESP_LOGI(TAG_CHARGER, "value charg %d", value_charg);

    return value_charg;
}