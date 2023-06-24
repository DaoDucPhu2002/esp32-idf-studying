#include "input_sensor.h"

static void sensor_eoline_callback(void)
{
}
static void sensor_insen_callback(void)
{
}
static void sensor_normally_callback(void)
{
}
uint8_t get_state_sensor(int channel)
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

void sensor_handle(void)
{
    for (uint8_t i = 0; i < 16; i++)
    {
        if (get_state_sensor(i) == 1)
        {
            // call back
            sensor_eoline_callback();
        }
        if (get_state_sensor(i) == 2)
        {
            // callback
            sensor_insen_callback();
        }
        if (get_state_sensor(i) == 3)
        {
            // callback
            sensor_normally_callback();
        }
    }
}
