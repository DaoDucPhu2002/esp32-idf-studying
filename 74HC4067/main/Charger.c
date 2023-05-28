#include "Charger.h"
#include "main.h"
uint16_t get_value_adc_vbat(void)
{

    uint16_t value_vbat = adc1_get_raw(ADC_VBAT);

    // ESP_LOGI(TAG_CHARGER, "value vbat %d", value_vbat);

    return value_vbat;
}
uint16_t get_value_Adc_charger(void)
{
    uint16_t value_charg = adc1_get_raw(ADC_CHARGER);
    ESP_LOGI(TAG_CHARGER, "value charg %d", value_charg);
    return value_charg;
}
