#include "Sensor_input.h"

int get_state_sensor(int channel)
{
    // static Sensor_State sta;
    uint8_t sta;
    // int eol_sta, ins_sta;
    //  gpio_set_level(SA0, channel & 0x01 ? 1 : 0);

    // gpio_set_level(SA1, channel & 0x02 ? 1 : 0);

    // gpio_set_level(SA2, channel & 0x04 ? 1 : 0);

    // // gpio_set_level(SA3, channel & 0x08 ? 1 : 0);
    if (channel < 8)
    {
        channel = 7 - channel;
    }

    gpio_set_level(SA0, channel & 0x01);
    gpio_set_level(SA1, (channel >> 1) & 0x01);
    gpio_set_level(SA2, (channel >> 2) & 0x01);
    gpio_set_level(SA3, (channel >> 3) & 0x01);
    vTaskDelay(2 / portTICK_PERIOD_MS);
    // eol_sta = gpio_get_level(EOLINE);
    // ins_sta = gpio_get_level(INSEN);
    sta = gpio_get_level(EOLINE) + gpio_get_level(INSEN) * 2;
    // if (eol_sta == 1 && ins_sta == 1)
    // {
    //     sta = NORMAL_SENSOR_STATE;
    // }
    // if (eol_sta && !ins_sta)
    // {
    //     sta = EOLINE_SENSOR_STATE;
    // }
    // if (!eol_sta && ins_sta)
    // {
    //     sta = INSEN_SENSOR_STATE;
    // }
    // ESP_LOGI(TAG_SENSOR, "channel %d: %d", channel + 1, sta);
    // ESP_LOGI(TAG_SENSOR, "Channel %d : is %s", channel, (sta == 0) ? ("EOLINE_SENSOR_STATE") : ((sta == 1) ? "INSEN_SENSOR_STATE" : "NORMAL_SENSOR_STATE"));
    // ESP_LOGI(TAG_SENSOR, "Channel %d : INSEN %d : EOLINE %d ", channel, ins_sta, eol_sta);
    // printf("Channel %d: %d", channel, sta);
    return sta;
}
