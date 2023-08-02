#include "Sensor_input.h"

int get_state_sensor(int channel)
{

    uint8_t sta;

    if (channel < 8)
    {
        channel = 7 - channel;
    }

    gpio_set_level(SA0, channel & 0x01);
    gpio_set_level(SA1, (channel >> 1) & 0x01);
    gpio_set_level(SA2, (channel >> 2) & 0x01);
    gpio_set_level(SA3, (channel >> 3) & 0x01);
    vTaskDelay(2 / portTICK_PERIOD_MS);

    sta = gpio_get_level(EOLINE) + gpio_get_level(INSEN) * 2;

    return sta;
}
