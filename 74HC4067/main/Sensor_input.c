#include "Sensor_input.h"

Sensor_State get_state_sensor(int channel)
{
    static Sensor_State sta;
    int eol_sta, ins_sta;
    gpio_set_level(SA0, channel & 0x01 ? 1 : 0);
    gpio_set_level(SA1, channel & 0x02 ? 1 : 0);
    gpio_set_level(SA2, channel & 0x04 ? 1 : 0);
    gpio_set_level(SA3, channel & 0x08 ? 1 : 0);

    eol_sta = gpio_get_level(EOLINE);
    ins_sta = gpio_get_level(INSEN);

    if (eol_sta == 1 && ins_sta == 1)
    {
        sta = NORMAL_SENSOR_STATE;
    }
    if (eol_sta && !ins_sta)
    {
        sta = EOLINE_SENSOR_STATE;
    }
    if (!eol_sta && ins_sta)
    {
        sta = INSEN_SENSOR_STATE;
    }
    ESP_LOGI(TAG_SENSOR, "Channel %d : is %s", (sta == 0) ? ("EOLINE_SENSOR_STATE") : ((sta == 1) ? "INSEN_SENSOR_STATE" : "NORMAL_SENSOR_STATE"));
    return sta;
}
